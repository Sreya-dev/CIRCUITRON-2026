#define BLYNK_TEMPLATE_ID "TMPL3qbnnkctp"
#define BLYNK_TEMPLATE_NAME "Smart Door Lock"
#define BLYNK_AUTH_TOKEN "0JRDFU6A2MQWxUKCYhffgoJT7hcqCbNV"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// ── Wi-Fi credentials ──────────────────────────────────────────────────────
const char *ssid = "Wokwi-GUEST"; // Wokwi's built-in Wi-Fi
const char *password = "";        // No password for Wokwi-GUEST

// ── Hardware ───────────────────────────────────────────────────────────────
#define SERVO_PIN 18 // GPIO18 → Servo signal wire

Servo doorServo;

// ── Blynk Virtual Pins ─────────────────────────────────────────────────────
// V0 → Button Widget  (Switch mode, 0/1)
// V1 → Label Widget   (shows lock status)

// ── Door state ─────────────────────────────────────────────────────────────
bool isDoorUnlocked = false;

// ──────────────────────────────────────────────────────────────────────────
// Helper: move servo + push status to Blynk dashboard
// ──────────────────────────────────────────────────────────────────────────
void setDoorState(bool unlock)
{
    isDoorUnlocked = unlock;

    if (unlock)
    {
        doorServo.write(90); // Unlock position
        Blynk.virtualWrite(V1, "Door Unlocked");
        Serial.println("[DOOR] Unlocked → Servo @ 90°");
    }
    else
    {
        doorServo.write(0); // Lock position
        Blynk.virtualWrite(V1, "Door Locked");
        Serial.println("[DOOR] Locked   → Servo @ 0°");
    }
}

// ──────────────────────────────────────────────────────────────────────────
// Blynk: V0 Button handler
//   value == 1  → button pressed  (unlock)
//   value == 0  → button released (lock)
// ──────────────────────────────────────────────────────────────────────────
BLYNK_WRITE(V0)
{
    int buttonState = param.asInt();
    setDoorState(buttonState == 1);
}

// ──────────────────────────────────────────────────────────────────────────
// Blynk: sync state when app connects / reconnects
// ──────────────────────────────────────────────────────────────────────────
BLYNK_CONNECTED()
{
    Serial.println("[Blynk] Connected – syncing V0");
    Blynk.syncVirtual(V0);
}

// ──────────────────────────────────────────────────────────────────────────
// setup
// ──────────────────────────────────────────────────────────────────────────
void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Smart Door Lock System ===");

    // Attach servo
    doorServo.attach(SERVO_PIN, 500, 2400); // min/max pulse µs
    doorServo.write(0);                     // Start locked
    Serial.println("[SERVO] Initialised at 0° (Locked)");

    // Connect to Blynk (handles Wi-Fi internally)
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
    Serial.println("[WiFi] Connecting via Blynk.begin ...");
}

// ──────────────────────────────────────────────────────────────────────────
// loop
// ──────────────────────────────────────────────────────────────────────────
void loop()
{
    Blynk.run();
}