// Smart Home Automation System
// ESP32 + Blynk Cloud + DHT22 + LDR + LEDs + Buzzer

#define BLYNK_TEMPLATE_ID    "TMPL3BHAi4kY2"
#define BLYNK_TEMPLATE_NAME  "Smart Home Automation System"
#define BLYNK_AUTH_TOKEN     "yHYiEHyfc4QHt_cLa-RnBdEynfoiiQ1F"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// ---- Pin Definitions ----
#define DHT_PIN            4
#define DHT_TYPE           DHT22
#define LDR_PIN            34
#define LIGHT_LED_PIN      26
#define FAN_LED_PIN        27
#define BUZZER_PIN         25

// ---- Thresholds ----
#define TEMP_THRESHOLD     30.0   // Fan turns ON above this
#define LDR_DARK_THRESHOLD 2500   // Light turns ON below this (dark room)

// ---- WiFi Credentials ----
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

DHT dht(DHT_PIN, DHT_TYPE);
BlynkTimer timer;

bool lightState = false;
bool fanState   = false;

void readSensorsAndControl() {
    // --- Read DHT22 ---
    float temperature = dht.readTemperature();
    float humidity    = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("DHT22 read failed!");
        return;
    }

    // --- Read LDR ---
    int ldrValue = analogRead(LDR_PIN);  // 0-4095 (12-bit ADC)

    // --- Serial Output ---
    Serial.printf("Temp: %.1f C | Hum: %.1f%% | Light: %s | Fan: %s | LDR: %d ",
      temperature, humidity,lightState ? "ON" : "OFF",fanState   ? "ON" : "OFF",ldrValue);
    Serial.println("\n");

    // ---- Light Control (LDR) ----
    if (ldrValue > LDR_DARK_THRESHOLD) {
        // Dark room → turn light ON
        if (!lightState) {
            lightState = true;
            digitalWrite(LIGHT_LED_PIN, HIGH);
            Serial.println("Room is DARK  → Light ON");
            digitalWrite(BUZZER_PIN, HIGH);
            delay(100);
            digitalWrite(BUZZER_PIN, LOW);
        }
    } else {
        // Bright room → turn light OFF
        if (lightState) {
            lightState = false;
            digitalWrite(LIGHT_LED_PIN, LOW);
            Serial.println("Room is BRIGHT → Light OFF");
        }
    }

    // ---- Fan Control (Temperature) ----
    if (temperature > TEMP_THRESHOLD) {
        if (!fanState) {
            fanState = true;
            digitalWrite(FAN_LED_PIN, HIGH);
            Serial.println("Temp HIGH → Fan ON");
            for (int i = 0; i < 2; i++) {
                digitalWrite(BUZZER_PIN, HIGH);
                delay(100);
                digitalWrite(BUZZER_PIN, LOW);
                delay(100);
            }
        }
    } else {
        if (fanState) {
            fanState = false;
            digitalWrite(FAN_LED_PIN, LOW);
            Serial.println("Temp LOW  → Fan OFF");
        }
    }

    // ---- Send to Blynk Dashboard ----
    Blynk.virtualWrite(V0, temperature);
    Blynk.virtualWrite(V1, humidity);
    Blynk.virtualWrite(V2, lightState ? "ON" : "OFF");
    Blynk.virtualWrite(V3, fanState   ? "ON" : "OFF");
    Blynk.virtualWrite(V4, ldrValue);
}

void setup() {
    Serial.begin(115200);

    pinMode(LIGHT_LED_PIN, OUTPUT);
    pinMode(FAN_LED_PIN,   OUTPUT);
    pinMode(BUZZER_PIN,    OUTPUT);

    digitalWrite(LIGHT_LED_PIN, LOW);
    digitalWrite(FAN_LED_PIN,   LOW);
    digitalWrite(BUZZER_PIN,    LOW);

    dht.begin();

    Serial.println("Connecting to Blynk...");
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    timer.setInterval(2000L, readSensorsAndControl);
}

void loop() {
    Blynk.run();
    timer.run();
}