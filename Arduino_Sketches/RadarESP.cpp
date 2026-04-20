// https://dronebotworkshop.com/esp-now/
#include <esp_now.h>
#include <WiFi.h>
#include <MyLD2410.h>

// ===== MAC ADDRESSES =====
// Replace your own ESP32 MAC address leave the 0x
uint8_t servoAndBuzzerAddress[]  = {0x28, 0x05, 0xA5, 0x31, 0xDF, 0x2C};
uint8_t lcdAddress[] = {0x28, 0x05, 0xA5, 0x31, 0xD8, 0x30};

// ===== PINS =====
#define LED_PIN 2
#define RADAR_RX 4  
#define RADAR_TX 14 

MyLD2410 radar(Serial2);

// ===== PWM =====
const int freq = 5000;
const int resolution = 8;

// ===== STATES =====
enum State { IDLE, SCANNING, TRACKING, LOST };
State currentState = IDLE;

bool radarEnabled = true;

// ===== TIMING =====
unsigned long scanStart = 0;
unsigned long lastDetectTime = 0;
const unsigned long scanTime = 500;
const unsigned long holdTime = 3000;

// ===== LED =====
int brightness = 0;
int targetBrightness = 0;

// ===== MESSAGE STRUCT =====
struct Message {
  bool active;
  uint8_t state;
  uint8_t motion;
  uint8_t brightness;
  uint32_t heartbeat;
};

uint32_t heartbeatCounter = 0;
bool lastPresence = false;

void setup() {
  Serial.begin(115200);
  Serial2.begin(256000, SERIAL_8N1, RADAR_RX, RADAR_TX);

  WiFi.mode(WIFI_STA);
  esp_now_init();

  esp_now_peer_info_t peer1 = {};
  memcpy(peer1.peer_addr, servoAndBuzzerAddress, 6);
  esp_now_add_peer(&peer1);

  esp_now_peer_info_t peer2 = {};
  memcpy(peer2.peer_addr, lcdAddress, 6);
  esp_now_add_peer(&peer2);

  radar.begin();
  ledcAttach(LED_PIN, freq, resolution);
  ledcWrite(LED_PIN, 0);
}

void updateLED() {
  if (brightness < targetBrightness) brightness++;
  else if (brightness > targetBrightness) brightness--;
  ledcWrite(LED_PIN, brightness);
}

void loop() {

    // ===== SERIAL OVERRIDE LAYER =====
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "RADAR_OFF") {
      radarEnabled = false;
      Serial.println("Radar DISABLED (Python override)");
    }

    if (cmd == "RADAR_ON") {
      radarEnabled = true;
      Serial.println("Radar ENABLED (Python override)");
    }
  }

  // ===== HARD STOP RADAR PROCESSING =====
  if (!radarEnabled) {
    targetBrightness = 0;
    updateLED();
    delay(5);
    return;
  }

  if (radar.check() == MyLD2410::DATA) {

    bool presence = radar.presenceDetected();
    bool moving   = radar.movingTargetDetected();
    bool still    = radar.stationaryTargetDetected();

    // ===== STATE MACHINE =====
    switch (currentState) {
      case IDLE:
        targetBrightness = 0;
        if (presence) {
          currentState = SCANNING;
          scanStart = millis();
        }
        break;

      case SCANNING:
        targetBrightness = 30;
        if (presence && millis() - scanStart > scanTime) {
          currentState = TRACKING;
          lastDetectTime = millis();
        } else if (!presence) {
          currentState = IDLE;
        }
        break;

      case TRACKING:
        targetBrightness = moving ? 255 : (still ? 100 : 0);
        if (presence) {
          lastDetectTime = millis();
        } else if (millis() - lastDetectTime > holdTime) {
          currentState = LOST;
        }
        break;

      case LOST:
        targetBrightness = 0;
        if (presence) {
          currentState = TRACKING;
          lastDetectTime = millis();
        } else if (millis() - lastDetectTime > 2000) {
          currentState = IDLE;
        }
        break;
    }

    // ===== BUILD MESSAGE =====
    Message msg;
    msg.active = (currentState == TRACKING);
    msg.state = currentState;

    if (moving) msg.motion = 1;
    else if (still) msg.motion = 2;
    else msg.motion = 0;

    msg.brightness = targetBrightness;

    // Increment heartbeat
    msg.heartbeat = heartbeatCounter++;

    // ===== SEND (20Hz) =====
    static unsigned long lastSend = 0;
    if (millis() - lastSend > 50) {
      lastSend = millis();

      esp_now_send(servoAndBuzzerAddress,  (uint8_t*)&msg, sizeof(msg));
      esp_now_send(lcdAddress, (uint8_t*)&msg, sizeof(msg));

      Serial.print("State: "); Serial.print(msg.state);
      Serial.print(" Motion: "); Serial.print(msg.motion);
      Serial.print(" Active: "); Serial.println(msg.active);
    }

    if (presence != lastPresence) {
      if (presence) Serial.println("CAMERA_ON");
      else Serial.println("CAMERA_OFF");

      lastPresence = presence;
    }

  }

  updateLED();
  delay(5);
}