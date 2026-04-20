#include <esp_now.h>
#include <WiFi.h>
#include "LiquidCrystal.h"

// ===== LCD =====
LiquidCrystal lcd(19, 23, 18, 17, 16, 15);

// ===== MESSAGE STRUCT =====
struct Message {
  bool active;
  uint8_t state;
  uint8_t motion;
  uint8_t brightness;
  uint32_t heartbeat;
};

Message msg;

// ===== TIMING =====
unsigned long lastReceiveTime = 0;
const unsigned long timeout = 500;

unsigned long lastBlink = 0;
bool blinkState = false;

// ===== CALLBACK =====
void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&msg, incomingData, sizeof(msg));
  lastReceiveTime = millis();
}

// ===== HELPERS =====
String getMotion(uint8_t m) {
  if (m == 1) return "MOVING";
  if (m == 2) return "STILL ";
  return "CLEAR ";
}

// ===== SETUP =====
void setup() {
  lcd.begin(16, 2);
  lcd.print("SECURITY PANEL");

  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));
}

// ===== LOOP =====
void loop() {

  bool signalLost = (millis() - lastReceiveTime > timeout);

  // ===== BLINK TIMER =====
  if (millis() - lastBlink > 400) {
    lastBlink = millis();
    blinkState = !blinkState;
  }

  lcd.setCursor(0, 0);
  lcd.setCursor(0, 1);

  // ===== SIGNAL LOST =====
  if (signalLost) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!! SYSTEM ERROR !!");
    lcd.setCursor(0, 1);
    lcd.print("NO SIGNAL");
    return;
  }

  // ===== ALERT MODE =====
  if (msg.active) {

    lcd.clear();

    // Blinking alert header
    lcd.setCursor(0, 0);
    if (blinkState) {
      lcd.print(">>> INTRUDER <<<");
    } else {
      lcd.print("                ");
    }

    // Motion info
    lcd.setCursor(0, 1);
    lcd.print("Target: ");
    lcd.print(getMotion(msg.motion));
  }

  // ===== NORMAL MODE =====
  else {
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("[SYSTEM NORMAL]");

    lcd.setCursor(0, 1);
    lcd.print("Area: CLEAR");
  }

  delay(100);
}