#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

#define SERVO_PIN 26
#define BUZZER_PIN 32

Servo servoMotor;

// ================= MESSAGE STRUCT =================
struct Message {
  bool active;
  uint8_t state;       // 0 IDLE, 1 SCANNING, 2 TRACKING, 3 LOST
  uint8_t motion;      // 0 none, 1 moving, 2 still
  uint8_t brightness;
  uint32_t heartbeat;
};

Message msg;

// ================= HEARTBEAT =================
unsigned long lastReceiveTime = 0;
const unsigned long timeout = 500;

// ================= SERVO MOTION (SMOOTH CONTROL) =================
float servoPos = 90;
float velocity = 0;

float maxSpeed = 2.5;
float acceleration = 0.12;

int direction = 1;

unsigned long lastServoUpdate = 0;
int servoInterval = 20;

// ================= ESP-NOW CALLBACK =================
void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&msg, incomingData, sizeof(msg));
  lastReceiveTime = millis();

  // Adjust responsiveness based on motion
  if (msg.motion == 1) {
    maxSpeed = 6.0;
    acceleration = 0.25;
    servoInterval = 8;
  } else {
    maxSpeed = 2.5;
    acceleration = 0.10;
    servoInterval = 20;
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  servoMotor.attach(SERVO_PIN);
  pinMode(BUZZER_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));
}

// ================= SMOOTH SERVO UPDATE =================
void updateServo() {
  unsigned long now = millis();
  if (now - lastServoUpdate < servoInterval) return;
  lastServoUpdate = now;

  // Reverse direction at limits
  if (servoPos >= 180) direction = -1;
  if (servoPos <= 0) direction = 1;

  // Smooth acceleration
  float targetSpeed = maxSpeed;

  if (velocity < targetSpeed) velocity += acceleration;
  if (velocity > targetSpeed) velocity -= acceleration;

  // Apply movement
  servoPos += velocity * direction;

  servoMotor.write((int)servoPos);
}

// ================= BUZZER LOGIC =================
void updateBuzzer() {
  switch (msg.state) {

    case 0: // IDLE
      noTone(BUZZER_PIN);
      break;

    case 1: // SCANNING
      tone(BUZZER_PIN, 800);
      break;

    case 2: // TRACKING
      if (msg.motion == 1) tone(BUZZER_PIN, 2000);
      else tone(BUZZER_PIN, 1200);
      break;

    case 3: // LOST
      tone(BUZZER_PIN, 400);
      break;
  }
}

// ================= LOOP =================
void loop() {

  bool signalLost = (millis() - lastReceiveTime > timeout);

  // ===== FAIL SAFE =====
  if (signalLost) {
    servoMotor.write(90);
    noTone(BUZZER_PIN);
    return;
  }

  // ===== ACTIVE CONTROL =====
  if (msg.active) {
    updateServo();
  } else {
    servoMotor.write(90);
  }

  updateBuzzer();
}
