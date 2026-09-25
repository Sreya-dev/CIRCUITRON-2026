# 🏠 Smart Home Automation System

**Platform:** Wokwi Simulation (with local VS Code option) · **Microcontroller:** ESP32 DevKit V1 · **Cloud:** Blynk IoT

🔗 **Wokwi Simulation:** [https://wokwi.com/projects/466598199922365441](https://wokwi.com/projects/466598199922365441)

> If the Wokwi website simulation isn't working, download the shared "Smart Home Automation System" folder and open it in VS Code.

---

## 📌 Project Aim

To design and implement a Smart Home Automation System using an ESP32 microcontroller that automatically controls home appliances (room light and fan) based on real-time environmental conditions, while enabling remote monitoring and control through the Blynk Cloud mobile dashboard.

---

## 📖 Project Overview

This system monitors ambient light intensity using an LDR sensor and room temperature and humidity using a DHT22 sensor. Based on the sensor readings, the system automatically toggles a room light LED and a fan LED. A buzzer provides audible alerts whenever an appliance state changes. All sensor data and appliance statuses are transmitted in real time to the Blynk Cloud platform, making them accessible from the Blynk mobile app from anywhere.

The project is simulated on the Wokwi online simulator, which emulates the ESP32 hardware, sensors, and output components without requiring physical hardware.

---

## 🧩 Components Used

| Component           | Quantity | Purpose                           |
| ------------------- | -------- | --------------------------------- |
| ESP32 DevKit V1     | 1        | Main microcontroller              |
| DHT22 Sensor        | 1        | Measures temperature and humidity |
| LDR (Photoresistor) | 1        | Detects ambient light level       |
| LED (Yellow)        | 1        | Represents room light             |
| LED (Blue)          | 1        | Represents fan                    |
| Buzzer              | 1        | Audible alert on state change     |
| Resistor 220Ω       | 2        | Current limiting for LEDs         |
| Resistor 10kΩ       | 1        | Pull-down for LDR voltage divider |
| Blynk Cloud         | —        | IoT dashboard for monitoring      |
| Wokwi Simulator     | —        | Virtual simulation environment    |

---

## 🔌 Construction and Wiring Details

### DHT22 Sensor

| DHT22 Pin | ESP32 Pin |
| --------- | --------- |
| VCC       | 3.3V      |
| GND       | GND       |
| DATA      | GPIO 4    |

### LDR Sensor (Voltage Divider)

| Connection     | Detail              |
| -------------- | ------------------- |
| LDR terminal 1 | 3.3V                |
| LDR terminal 2 | GPIO 34             |
| Midpoint       | GPIO 34 (ADC input) |

Dark room → high resistance → high ADC value (near 4095). Bright room → low resistance → low ADC value (near 0).

### Room Light LED

| LED Pin     | Connection                |
| ----------- | ------------------------- |
| Anode (+)   | GPIO 26 via 220Ω resistor |
| Cathode (−) | GND                       |

### Fan LED

| LED Pin     | Connection                |
| ----------- | ------------------------- |
| Anode (+)   | GPIO 27 via 220Ω resistor |
| Cathode (−) | GND                       |

### Buzzer

| Buzzer Pin   | ESP32 Pin |
| ------------ | --------- |
| Positive (+) | GPIO 25   |
| Negative (−) | GND       |

---

## 🔄 Code Structure / Program Flow

### Setup Phase

1. Serial monitor initialized at 115200 baud
2. Output pins configured for LEDs and buzzer
3. DHT22 sensor initialized
4. ESP32 connects to WiFi (Wokwi-GUEST) and authenticates with Blynk Cloud
5. Timer set to call `readSensorsAndControl()` every 2000 ms

### Main Loop

`loop()` continuously calls `Blynk.run()` (maintains cloud connection) and `timer.run()` (triggers the sensor function on schedule).

### `readSensorsAndControl()` — Core Logic

1. Read temperature and humidity from DHT22
2. Validate readings (skip cycle if NaN)
3. Read LDR raw ADC value (0–4095)
4. Print formatted values to serial monitor
5. **Light control:** if LDR > 2500 (dark) → Light LED ON + short beep; else Light LED OFF
6. **Fan control:** if temperature > 30°C → Fan LED ON + double beep; else Fan LED OFF
7. Send V0 (temperature), V1 (humidity), V2 (light status), V3 (fan status), V4 (LDR value) to Blynk

### Blynk Virtual Pin Map

| Pin | Data                  | Widget |
| --- | --------------------- | ------ |
| V0  | Temperature (°C)      | Gauge  |
| V1  | Humidity (%)          | Gauge  |
| V2  | Light Status (ON/OFF) | Label  |
| V3  | Fan Status (ON/OFF)   | Label  |
| V4  | LDR Raw Value         | Label  |

---

## 💻 Source Code

```cpp
// Smart Home Automation System
// ESP32 + Blynk Cloud + DHT22 + LDR + LEDs + Buzzer

#define BLYNK_TEMPLATE_ID   "TMPL3BHAi4kY2"
#define BLYNK_TEMPLATE_NAME "Smart Home Automation System"
#define BLYNK_AUTH_TOKEN    "yHYiEHyfc4QHt_cLa-RnBdEynfoiiQ1F"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// ---- Pin Definitions ----
#define DHT_PIN       4
#define DHT_TYPE      DHT22
#define LDR_PIN       34
#define LIGHT_LED_PIN 26
#define FAN_LED_PIN   27
#define BUZZER_PIN    25

// ---- Thresholds ----
#define TEMP_THRESHOLD     30.0  // Fan turns ON above this
#define LDR_DARK_THRESHOLD 2500  // Light turns ON below this (dark room)

// ---- WiFi Credentials ----
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

DHT dht(DHT_PIN, DHT_TYPE);
BlynkTimer timer;

bool lightState = false;
bool fanState = false;

void readSensorsAndControl() {
  // --- Read DHT22 ---
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("DHT22 read failed!");
    return;
  }

  // --- Read LDR ---
  int ldrValue = analogRead(LDR_PIN); // 0-4095 (12-bit ADC)

  // --- Serial Output ---
  Serial.printf("Temp: %.1f C | Hum: %.1f%% | Light: %s | Fan: %s | LDR: %d ",
    temperature, humidity, lightState ? "ON" : "OFF", fanState ? "ON" : "OFF", ldrValue);
  Serial.println("\n");

  // ---- Light Control (LDR) ----
  if (ldrValue > LDR_DARK_THRESHOLD) {
    // Dark room → turn light ON
    if (!lightState) {
      lightState = true;
      digitalWrite(LIGHT_LED_PIN, HIGH);
      Serial.println("Room is DARK   -> Light ON");
      digitalWrite(BUZZER_PIN, HIGH);
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);
    }
  } else {
    // Bright room → turn light OFF
    if (lightState) {
      lightState = false;
      digitalWrite(LIGHT_LED_PIN, LOW);
      Serial.println("Room is BRIGHT -> Light OFF");
    }
  }

  // ---- Fan Control (Temperature) ----
  if (temperature > TEMP_THRESHOLD) {
    if (!fanState) {
      fanState = true;
      digitalWrite(FAN_LED_PIN, HIGH);
      Serial.println("Temp HIGH -> Fan ON");
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
      Serial.println("Temp LOW  -> Fan OFF");
    }
  }

  // ---- Send to Blynk Dashboard ----
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, lightState ? "ON" : "OFF");
  Blynk.virtualWrite(V3, fanState ? "ON" : "OFF");
  Blynk.virtualWrite(V4, ldrValue);
}

void setup() {
  Serial.begin(115200);

  pinMode(LIGHT_LED_PIN, OUTPUT);
  pinMode(FAN_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LIGHT_LED_PIN, LOW);
  digitalWrite(FAN_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  dht.begin();

  Serial.println("Connecting to Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, readSensorsAndControl);
}

void loop() {
  Blynk.run();
  timer.run();
}
```

---

## 🎯 Automation Logic Summary

| Condition                | Action               | Buzzer        |
| ------------------------ | -------------------- | ------------- |
| LDR > 2500 (dark room)   | Room Light LED → ON  | 1 short beep  |
| LDR ≤ 2500 (bright room) | Room Light LED → OFF | Silent        |
| Temperature > 30°C       | Fan LED → ON         | 2 short beeps |
| Temperature ≤ 30°C       | Fan LED → OFF        | Silent        |

---

## ✨ Additional Features

- **Buzzer alerts** — audible feedback on every appliance state change, distinguishing light events (single beep) from fan events (double beep)
- **Serial monitor dashboard** — formatted real-time output with separator lines for clean debugging
- **NaN guard** — sensor validation prevents erroneous actuations from faulty reads
- **State-change logic** — `lightState` and `fanState` flags ensure the buzzer and serial logs fire only on transitions, not on every 2-second tick
- **Blynk External API** — all 5 virtual pins accessible via REST URLs for third-party integrations

---

## 🌐 Blynk Cloud API Links

| Pin | Purpose       | URL                                                                                  |
| --- | ------------- | ------------------------------------------------------------------------------------ |
| V0  | Temperature   | `https://blynk.cloud/external/api/get?token=yHYiEHyfc4QHt_cLa-RnBdEynfoiiQ1F&pin=V0` |
| V1  | Humidity      | `https://blynk.cloud/external/api/get?token=yHYiEHyfc4QHt_cLa-RnBdEynfoiiQ1F&pin=V1` |
| V2  | Light Status  | `https://blynk.cloud/external/api/get?token=yHYiEHyfc4QHt_cLa-RnBdEynfoiiQ1F&pin=V2` |
| V3  | Fan Status    | `https://blynk.cloud/external/api/get?token=yHYiEHyfc4QHt_cLa-RnBdEynfoiiQ1F&pin=V3` |
| V4  | LDR Raw Value | `https://blynk.cloud/external/api/get?token=yHYiEHyfc4QHt_cLa-RnBdEynfoiiQ1F&pin=V4` |

**Get All Pin Values:** `https://blynk.cloud/external/api/getAll?token=yHYiEHyfc4QHt_cLa-RnBdEynfoiiQ1F`

---

**Author:** Sreya Vinod
