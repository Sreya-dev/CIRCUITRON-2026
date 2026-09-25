# 🌡️ Smart Temperature Monitoring and Fan Control System

**Platform:** Wokwi Simulation (with local VS Code option) · **Microcontroller:** ESP32 DevKit · **Cloud:** Blynk IoT

🔗 **Wokwi Simulation:** [https://wokwi.com/projects/466418140611780609](https://wokwi.com/projects/466418140611780609)

> If the Wokwi website simulation isn't working, download the shared "Smart Temperature Monitoring and Fan Control System" folder and open it in VS Code.

**Live Data:** Temperature | Humidity | Fan Status

---

## 📌 Project Aim

To design and implement an IoT-based Smart Temperature Monitoring and Automated Fan Control System using an ESP32 microcontroller, a DHT22 temperature and humidity sensor, and the Blynk Cloud IoT platform. The system continuously monitors environmental conditions in real time and automatically activates or deactivates a fan (simulated by an LED) based on a configurable temperature threshold, with live data visualised on a cloud dashboard accessible from any device.

---

## 📖 Project Overview

This project demonstrates a complete IoT monitoring pipeline — from physical sensing through cloud connectivity to automated control. The ESP32 microcontroller reads temperature and humidity values from a DHT22 sensor every 5 seconds and transmits them to the Blynk Cloud over Wi-Fi. Based on the temperature reading, it automatically controls a fan indicator LED connected to GPIO 13.

**Key behaviour:**

- If temperature > 30°C → Fan LED turns ON
- If temperature ≤ 30°C → Fan LED turns OFF
- Live values are pushed to Blynk Virtual Pins: **V0** (Temperature), **V1** (Humidity), **V2** (Fan Status)
- Simulated and tested entirely within the Wokwi online embedded-systems simulator

The project is a practical demonstration of IoT principles including sensor interfacing, cloud connectivity, real-time data streaming, and threshold-based actuation — all core concepts in modern smart home and industrial monitoring systems.

---

## 🧩 Components Used

| Component       | Specification / Role                                  | Quantity  |
| --------------- | ----------------------------------------------------- | --------- |
| ESP32 DevKit    | Main microcontroller — Wi-Fi, GPIO, processing        | 1         |
| DHT22 Sensor    | Temperature (−40 to 80°C) & humidity (0–100%) sensing | 1         |
| LED (Green)     | Fan indicator — simulates fan motor output on GPIO 13 | 1         |
| Resistor        | 220Ω current-limiting resistor for LED                | 1         |
| Jumper Wires    | Electrical connections between components             | As needed |
| Blynk Cloud     | IoT cloud platform — dashboard, virtual pins, Wi-Fi   | 1 account |
| Wokwi Simulator | Online ESP32/Arduino simulator — virtual hardware     | Online    |

---

## 🔌 Construction and Wiring Details

| Component Pin     | ESP32 Pin                 | Notes                   |
| ----------------- | ------------------------- | ----------------------- |
| DHT22 — VCC       | 3.3V                      | Power supply for sensor |
| DHT22 — GND       | GND                       | Common ground           |
| DHT22 — DATA      | GPIO 15                   | Single-wire data line   |
| LED — Anode (+)   | GPIO 13 via 220Ω resistor | Fan indicator output    |
| LED — Cathode (−) | GND                       | Common ground           |

### Wiring Principles Followed

- All GND connections share a common ground with the ESP32 GND pin.
- The DHT22 data line connects directly to GPIO 15 — the DHTesp library handles the single-wire protocol internally.
- A 220Ω resistor in series with the LED limits current to a safe level (~15mA at 3.3V), preventing damage to the GPIO pin.
- No external pull-up resistor is required for the DHT22 data line in Wokwi — the internal library configuration handles this.

---

## 🔄 Code Structure / Program Flow

The firmware is structured into three logical sections:

| Section                  | Function / Purpose                                                                                                               |
| ------------------------ | -------------------------------------------------------------------------------------------------------------------------------- |
| Pin & Object Definitions | `DHT_PIN=15`, `FAN_PIN=13`; `DHTesp` and `BlynkTimer` objects declared globally                                                  |
| `setup()`                | Serial init at 115200 baud, GPIO pin mode, DHT22 init, `Blynk.begin()` for Wi-Fi/cloud, timer interval set to 5000ms             |
| `loop()`                 | Calls `Blynk.run()` and `timer.run()` continuously — Blynk handles comms, timer triggers `sendSensorData()`                      |
| `sendSensorData()`       | Reads DHT22, validates data, writes V0/V1 to Blynk, evaluates temperature threshold, sets GPIO 13 HIGH/LOW, writes V2 fan status |

### Program Flow Summary

- System initialises hardware and connects to Blynk Cloud via Wokwi-GUEST Wi-Fi.
- Every 5 seconds (configured as 2 seconds in code), `sendSensorData()` is called by the `BlynkTimer`.
- DHT22 is read; if reading is invalid (NaN), the function exits early and retries next cycle.
- Valid temperature and humidity are written to Virtual Pins V0 and V1 on the Blynk dashboard.
- If temperature > 30°C: GPIO 13 goes HIGH (LED ON), V2 = `"ON"`. Otherwise GPIO 13 goes LOW (LED OFF), V2 = `"OFF"`.
- `loop()` keeps the Blynk connection alive and timer running — the system loops indefinitely.

---

## 💻 Source Code

```cpp
#define BLYNK_TEMPLATE_ID   "TMPL3k3svVwwW"
#define BLYNK_TEMPLATE_NAME "Smart Temp Monitor"
#define BLYNK_AUTH_TOKEN    "jsa9ToD2x8PAu5nGOGFNc-p_lfa6AQ7W"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHTesp.h>

// Wi-Fi credentials (Wokwi virtual network)
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

DHTesp dht;
BlynkTimer timer;

// Pin definitions
const int DHT_PIN = 15; // DHT22 data pin
const int FAN_PIN = 13; // LED fan indicator pin

// === Read sensor & control fan ===
void sendSensorData() {
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  // Validate sensor reading
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Push temperature and humidity to Blynk dashboard
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);

  // Debug output to serial monitor
  Serial.print("Temp: ");     Serial.print(temperature);   Serial.print("C | ");
  Serial.print("Humidity: "); Serial.print(humidity);      Serial.println("%");

  // Fan control logic based on temperature threshold
  if (temperature > 30.0) {
    digitalWrite(FAN_PIN, HIGH);   // Turn fan ON
    Blynk.virtualWrite(V2, "ON");  // Update Blynk fan status
    Serial.println("Fan Status: ON");
  } else {
    digitalWrite(FAN_PIN, LOW);    // Turn fan OFF
    Blynk.virtualWrite(V2, "OFF"); // Update Blynk fan status
    Serial.println("Fan Status: OFF");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(FAN_PIN, OUTPUT);

  // Initialise DHT22 sensor
  dht.setup(DHT_PIN, DHTesp::DHT22);

  // Connect to Blynk cloud via Wi-Fi
  Blynk.begin(auth, ssid, pass);

  // Schedule sensor reads every 2 seconds
  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();  // Handle Blynk communication
  timer.run();  // Execute scheduled timer tasks
}
```

---

## ☁️ Blynk Cloud Dashboard Configuration

| Virtual Pin | Widget Type           | Label       | Data Type | Range / Notes               |
| ----------- | --------------------- | ----------- | --------- | --------------------------- |
| V0          | Gauge / Value Display | Temperature | Double    | Min: −10, Max: 80, Unit: °C |
| V1          | Gauge / Value Display | Humidity    | Double    | Min: 0, Max: 100, Unit: %   |
| V2          | Value Display / Label | Fan Status  | String    | Displays "ON" or "OFF"      |

- Template created in Blynk Cloud with Hardware = ESP32, Connection = WiFi.
- Three datastreams added matching the virtual pins above.
- Device created from the template; Auth Token copied into the `#define` at the top of the code.

---

## ✅ Testing and Verification

Verification procedure used during simulation:

- Serial Monitor (115200 baud) confirmed temperature, humidity, and fan status printed every 5 seconds.
- DHT22 temperature adjusted in Wokwi component panel to values above and below 30°C.
- LED in simulator toggled correctly — ON above 30°C, OFF at or below 30°C.
- Blynk dashboard gauges updated in real time; V2 label changed between ON and OFF.
- Disconnection test: `Blynk.begin()` retries connection automatically on Wi-Fi loss.

---

## ✨ Additional Features / Enhancements

- Real-time cloud dashboard accessible from mobile and web — not just serial output.
- NaN validation guard prevents crashes on sensor read failure.
- 2-second update interval.
- Fan status transmitted as a readable string (`"ON"`/`"OFF"`) rather than numeric 0/1 for clear human-readable display.
- Code uses `BlynkTimer` (non-blocking) instead of `delay()` — keeps the Blynk connection alive during waits.

---

## 📸 Project Photograph(s)

The Wokwi simulator circuit shows the ESP32 DevKit with the DHT22 sensor connected to GPIO 15, and the green LED with current-limiting resistor connected to GPIO 13. Screenshots of the Blynk dashboard showing live temperature, humidity, and fan status should be attached here.

---

**Author:** Sreya Vinod
