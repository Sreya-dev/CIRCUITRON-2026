# Smart City Emergency Response & Safety Management System

**CIRCUITRON WEEK 2: DAY 05**
Author: **Sreya Vinod**

A Smart City Emergency Response and Safety Management System built on **ESP32**, combining ultrasonic parking detection, gas/pollution monitoring, fire-risk temperature sensing, and citizen emergency reporting — all coordinated through a real-time OLED dashboard.

🔗 **Wokwi Simulation:** [https://wokwi.com/projects/466690577837257729](https://wokwi.com/projects/466690577837257729)

---

## 1. Project Aim

To design and implement a Smart City Emergency Response and Safety Management System using **ESP32, HC-SR04 Ultrasonic Sensor, MQ2 Gas Sensor, DHT22 Temperature Sensor, SSD1306 OLED Display, Push Button, LEDs, Buzzer, Servo Motor, and Relay Module** that automatically monitors parking availability, air quality, temperature, and emergency situations — while controlling alarms, barriers, and emergency devices with real-time status updates on the OLED display.

## 2. Project Overview

This system monitors four environmental and situational parameters:

- **Parking occupancy** — via HC-SR04
- **Air quality** — via MQ2
- **Temperature** — via DHT22
- **Citizen emergency reports** — via push button

Based on these readings, the system automatically controls a red LED, green LED, buzzer, servo motor barrier, and relay module, with all statuses displayed in real time on a 128x64 SSD1306 OLED dashboard.

### Architecture (3-tier smart city model)

| Layer                | Role                                                          |
| -------------------- | ------------------------------------------------------------- |
| **Edge Device**      | ESP32 + Sensors + Actuators + OLED Display                    |
| **Processing Layer** | On-device logic for threshold evaluation and actuator control |
| **User Interface**   | SSD1306 OLED City Dashboard with live status updates          |

---

## 3. Components Used

| Component                    | Model                  | Purpose                                  |
| ---------------------------- | ---------------------- | ---------------------------------------- |
| Microcontroller              | ESP32 DevKit V1        | Main controller, all GPIO control        |
| Distance Sensor              | HC-SR04 Ultrasonic     | Detects parking slot occupancy           |
| Gas Sensor                   | MQ2                    | Monitors air quality / pollution level   |
| Temperature Sensor           | DHT22                  | Reads ambient temperature for fire risk  |
| Display                      | SSD1306 OLED 128x64    | Real-time city dashboard                 |
| Push Button                  | Tactile Button         | Citizen emergency reporting              |
| Parking Available LED        | Green LED              | Indicates parking slot is free           |
| Parking Full / Emergency LED | Red LED                | Indicates slot occupied or emergency     |
| Alert Buzzer                 | Active Buzzer          | Audio alert for all emergency conditions |
| Road Barrier                 | SG90 Servo Motor       | Opens/closes emergency route             |
| Emergency System             | Relay Module           | Activates external emergency devices     |
| Current Limiting Resistors   | 220Ω × 2               | Protects LEDs from excess current        |
| Pull-up Resistor             | 10kΩ                   | DHT22 data line stability                |
| Simulation Platform          | Wokwi Online Simulator | Virtual hardware simulation              |

---

## 4. Wiring Details

### HC-SR04 Ultrasonic Sensor

| HC-SR04 Pin | ESP32 Pin                                |
| ----------- | ---------------------------------------- |
| VCC         | 5V                                       |
| TRIG        | GPIO 5                                   |
| ECHO        | GPIO 18 (via voltage divider: 1kΩ + 2kΩ) |
| GND         | GND                                      |

### MQ2 Gas Sensor

| MQ2 Pin | ESP32 Pin |
| ------- | --------- |
| VCC     | 3.3V      |
| AOUT    | GPIO 34   |
| GND     | GND       |

### DHT22 Temperature Sensor

| DHT22 Pin | ESP32 Pin             |
| --------- | --------------------- |
| VCC       | 3.3V                  |
| DATA      | GPIO 4 + 10kΩ to 3.3V |
| GND       | GND                   |

### SSD1306 OLED Display (I2C)

| OLED Pin | ESP32 Pin |
| -------- | --------- |
| VCC      | 3.3V      |
| GND      | GND       |
| SDA      | GPIO 21   |
| SCL      | GPIO 22   |

### Push Button

| Button Pin | Connection |
| ---------- | ---------- |
| One leg    | GPIO 15    |
| Other leg  | 3.3V       |

### Green LED (Parking Available)

| LED Pin     | Connection     |
| ----------- | -------------- |
| Anode (+)   | 220Ω → GPIO 26 |
| Cathode (−) | GND            |

### Red LED (Parking Full / Emergency)

| LED Pin     | Connection     |
| ----------- | -------------- |
| Anode (+)   | 220Ω → GPIO 27 |
| Cathode (−) | GND            |

### Buzzer

| Buzzer Pin | ESP32 Pin |
| ---------- | --------- |
| + Positive | GPIO 25   |
| − Negative | GND       |

### Servo Motor (Road Barrier)

| Servo Wire             | ESP32 Pin |
| ---------------------- | --------- |
| Red (VCC)              | 5V        |
| Brown/Black (GND)      | GND       |
| Orange/Yellow (Signal) | GPIO 13   |

### Relay Module

| Relay Pin | ESP32 Pin |
| --------- | --------- |
| VCC       | 3.3V      |
| GND       | GND       |
| IN        | GPIO 12   |

---

## 5. Program Flow

1. ESP32 boots and initializes Serial at 115200 baud
2. ADC configured with `analogSetAttenuation(ADC_11db)` and 12-bit resolution
3. All GPIO pins set as OUTPUT/INPUT and initialized to safe LOW states
4. DHT22 initialized on GPIO 4, Servo attached to GPIO 13 at 0°
5. SSD1306 OLED initialized at I2C address `0x3C` — splash screen displayed
6. Main loop begins at ~10 Hz (100 ms delay)
7. Every loop cycle: HC-SR04 reads distance → `parkingOccupied` evaluated
8. DHT22 reads temperature → `fireAlert` evaluated (temp > 35°C)
9. MQ2 reads 20 analog samples → averaged → mapped from raw 843–4095 to 0–1023 → `pollutionAlert` evaluated (mapped value > 600)
10. Push button state read → if HIGH (pressed), `emergencyPressed` latches true
11. `cityEmergency` evaluated — true only if `pollutionAlert` AND `fireAlert` both active
12. LEDs updated: Red ON if `cityEmergency` or `parkingOccupied`, else Green ON
13. Relay updated: ON if `fireAlert` or `cityEmergency`
14. Buzzer updated: ON if `pollutionAlert`, `fireAlert`, `btnPressed`, or `cityEmergency`
15. Servo updated: 90° if `cityEmergency` or `emergencyPressed`, else 0°
16. Every 500 ms: OLED updates based on priority — City Emergency → Fire Alert → Pollution Alert → Emergency Reported → Normal Dashboard

---

## 6. Source Code

```cpp
/*
============================================================
Smart City Emergency Response & Safety Management System
Platform : Wokwi (ESP32)
Author   : Sreya
============================================================
PIN MAP
-------
HC-SR04    TRIG   → GPIO 5    ECHO → GPIO 18
MQ2        AOUT   → GPIO 34
DHT22      DATA   → GPIO 4
SSD1306    SDA    → GPIO 21   SCL  → GPIO 22
Push Btn          → GPIO 15 (one leg → GPIO15, other leg → 3.3V)
Green LED         → GPIO 26 (via 220Ω)
Red LED           → GPIO 27 (via 220Ω)
Buzzer            → GPIO 25
Servo             → GPIO 13
Relay             → GPIO 12 (active HIGH)
============================================================
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <ESP32Servo.h>

// ── OLED ──────────────────────────────────────────────────
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ── DHT22 ─────────────────────────────────────────────────
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ── Servo ─────────────────────────────────────────────────
Servo barrierServo;
#define SERVO_PIN 13

// ── Pin Definitions ───────────────────────────────────────
#define TRIG_PIN 5
#define ECHO_PIN 18
#define MQ2_PIN 34
#define BTN_PIN 15
#define GREEN_LED 26
#define RED_LED 27
#define BUZZER_PIN 25
#define RELAY_PIN 12

// ── Thresholds ────────────────────────────────────────────
#define PARKING_THRESHOLD_CM 10
#define GAS_THRESHOLD 600      // applied after map() remapping
#define TEMP_THRESHOLD 35.0

// ── State Variables ───────────────────────────────────────
bool parkingOccupied = false;
bool pollutionAlert = false;
bool fireAlert = false;
bool cityEmergency = false;

float currentTemp = 0.0;
int currentGas = 0;
long currentDist = 0;
int servoAngle = 0;

unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_INTERVAL = 500;

// ══════════════════════════════════════════════════════════
// Read HC-SR04 ultrasonic distance in cm
// ══════════════════════════════════════════════════════════
long readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  return duration * 0.034 / 2;
}

// ══════════════════════════════════════════════════════════
// Set servo angle and update servoAngle tracker
// ══════════════════════════════════════════════════════════
void setServo(int angle) {
  servoAngle = angle;
  barrierServo.write(angle);
}

// ══════════════════════════════════════════════════════════
// Buzzer ON/OFF
// ══════════════════════════════════════════════════════════
void setBuzzer(bool on) {
  if (on) { tone(BUZZER_PIN, 300); }
  else { noTone(BUZZER_PIN); }
}

// ══════════════════════════════════════════════════════════
// OLED: Normal 5-line city dashboard
// Shown during NORMAL and WARNING states
// ══════════════════════════════════════════════════════════
void showDashboard(const char* parkStatus,
                    const char* gasStatus,
                    const char* barrierStatus,
                    const char* cityStatus) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("PKG : "); display.println(parkStatus);

  display.setCursor(0, 12);
  display.print("AIR : "); display.print(gasStatus);
  display.print(" ("); display.print(currentGas); display.println(")");

  display.setCursor(0, 24);
  display.print("TMP : "); display.print(currentTemp, 1); display.println(" C");

  display.setCursor(0, 36);
  display.print("ROAD: "); display.println(barrierStatus);

  display.drawFastHLine(0, 47, 128, SSD1306_WHITE);

  // Status line with inverted highlight
  display.setCursor(0, 51);
  display.print("STS:");
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  display.print(" "); display.print(cityStatus); display.print(" ");
  display.setTextColor(SSD1306_WHITE);

  display.display();
}

// ══════════════════════════════════════════════════════════
// OLED: Full-screen alert
// Shown only during EMERGENCY conditions
// ══════════════════════════════════════════════════════════
void showAlert(const char* headerLine,
               const char* bigLine1,
               const char* bigLine2) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0); display.println(headerLine);

  display.setTextSize(2);
  display.setCursor(0, 16); display.println(bigLine1);
  display.setCursor(0, 40); display.println(bigLine2);

  display.display();
}

// ══════════════════════════════════════════════════════════
// SETUP
// ══════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);

  // Configure ESP32 ADC for stable MQ2 readings
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12); // 12-bit: raw range 0–4095

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  // Ensure nothing activates at boot
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(RELAY_PIN, LOW);
  noTone(BUZZER_PIN);

  dht.begin();
  barrierServo.attach(SERVO_PIN);
  setServo(0); // Road closed by default (0° = closed, 90° = open)

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 failed — check wiring!");
    while (true);
  }

  // Splash screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(8, 10);  display.println("SMART CITY SYSTEM");
  display.setCursor(8, 26);  display.println("Initialising...");
  display.setCursor(8, 42);  display.println("Author: Sreya");
  display.display();
  delay(2500);
}

// ══════════════════════════════════════════════════════════
// MAIN LOOP
// ══════════════════════════════════════════════════════════
void loop() {
  // ── 1. READ ALL SENSORS ──────────────────────────────────
  currentDist = readUltrasonic();

  currentTemp = dht.readTemperature();
  if (isnan(currentTemp)) currentTemp = 0.0; // guard NaN on read failure

  // MQ2: 20-sample average to reduce ADC noise
  // Raw Wokwi MQ2 range is 843 (clean air) to 4095 (max gas)
  // map() remaps this to 0–1023 so GAS_THRESHOLD 600 works correctly
  long gasSum = 0;
  for (int i = 0; i < 20; i++) {
    gasSum += analogRead(MQ2_PIN);
    delay(2);
  }
  int rawGas = (int)(gasSum / 20);
  currentGas = (int)map(rawGas, 843, 4095, 0, 1023);
  currentGas = constrain(currentGas, 0, 1023);

  // Button: HIGH = pressed (one leg → GPIO15, other leg → 3.3V)
  bool btnPressed = (digitalRead(BTN_PIN) == HIGH);

  // ── 2. EVALUATE CONDITIONS ───────────────────────────────
  parkingOccupied = (currentDist > 0 && currentDist < PARKING_THRESHOLD_CM);
  pollutionAlert  = (currentGas > GAS_THRESHOLD);           // Condition 2
  fireAlert       = (currentTemp > TEMP_THRESHOLD);         // Condition 3
  cityEmergency   = (pollutionAlert && fireAlert);          // Condition 6

  // ── 3. ACTUATOR CONTROL ──────────────────────────────────
  // LEDs — Red if parking full or city emergency, else Green (Conditions 1 & 6)
  if (cityEmergency || parkingOccupied) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
  }

  // Relay — ON during fire alert or city emergency (Conditions 3 & 6)
  digitalWrite(RELAY_PIN, (fireAlert || cityEmergency) ? HIGH : LOW);

  // Buzzer — ON for any active alert or while button is held (Conditions 2,3,4,6)
  // Button buzzer follows physical press only: ON while held, OFF when released
  bool buzzerOn = pollutionAlert ||
                  fireAlert ||
                  btnPressed ||
                  cityEmergency;
  setBuzzer(buzzerOn);

  // Servo barrier — 90° while button held OR during city emergency (Conditions 4,5,6)
  // Returns to 0° immediately when button released (if no city emergency)
  if (cityEmergency || btnPressed) {
    setServo(90); // Emergency route OPEN
  } else {
    setServo(0);  // Road CLOSED
  }

  // ── 4. SERIAL DEBUG ──────────────────────────────────────
  Serial.printf(
    "Dist:%ld cm | Gas:%d | Temp:%.1f C | Btn:%d | "
    "Park:%s | Poll:%s | Fire:%s | CityEmg:%s | Servo:%d\n",
    currentDist, currentGas, currentTemp, btnPressed,
    parkingOccupied ? "YES" : "NO",
    pollutionAlert  ? "YES" : "NO",
    fireAlert       ? "YES" : "NO",
    cityEmergency   ? "YES" : "NO",
    servoAngle
  );

  // ── 5. OLED UPDATE every 500ms ───────────────────────────
  if (millis() - lastDisplayUpdate >= DISPLAY_INTERVAL) {
    lastDisplayUpdate = millis();

    const char* parkStr    = parkingOccupied ? "FULL" : "AVAIL";
    const char* gasStr     = pollutionAlert   ? "POLL" : "SAFE";
    const char* barrierStr = (servoAngle == 90) ? "OPEN" : "CLOSED";

    // Specific status labels — WARNING shows exact cause
    const char* cityStatus;
    if (cityEmergency)                      cityStatus = "!! EMERGENCY !!";
    else if (btnPressed)                    cityStatus = "!! EMERGENCY !!";
    else if (pollutionAlert && fireAlert)   cityStatus = "WARN:FIRE+GAS";
    else if (fireAlert)                     cityStatus = "WARN: FIRE";
    else if (pollutionAlert)                cityStatus = "WARN: GAS";
    else                                    cityStatus = "NORMAL";

    // OLED priority:
    // EMERGENCY conditions → full-screen alert
    // WARNING / NORMAL → dashboard (status line shows current state)
    if (cityEmergency) {
      showAlert("!! CITY EMERGENCY !!", "CITY EMERG", "EVACUATE!");
    }
    else if (btnPressed) {
      // Full screen only while button is held — clears on release
      showAlert("!! CITIZEN EMERGENCY !!", "EMERGENCY", "REPORTED!!");
    }
    else {
      // Dashboard shown for NORMAL and WARNING states
      showDashboard(parkStr, gasStr, barrierStr, cityStatus);
    }
  }

  delay(100); // 10 Hz main loop
}
```

---

## 7. Project Implementation and Technical Quality

### 7.1 Working Functionality

The Smart City Emergency Response System operates on a multi-sensor continuous monitoring mechanism running at 10 Hz.

- The **HC-SR04** ultrasonic sensor detects parking occupancy — when distance drops below 10 cm, the red LED activates and the dashboard shows `FULL`.
- The **MQ2** gas sensor takes 20 averaged samples per cycle and maps them to a 0–1023 scale — when the mapped value exceeds 600, the buzzer sounds and a pollution alert is displayed.
- The **DHT22** monitors temperature — when it exceeds 35°C, the relay activates, simulating emergency system activation.
- A **push button** allows citizens to report emergencies directly — pressing it latches the system into emergency state, rotating the servo barrier to 90° and showing a full-screen `EMERGENCY REPORTED` message.
- When both gas and temperature thresholds are exceeded simultaneously, **City Emergency Mode** activates all actuators together.

### Automation Logic

| Condition                 | Green LED | Red LED | Buzzer    | Relay | Servo | OLED                  |
| ------------------------- | --------- | ------- | --------- | ----- | ----- | --------------------- |
| All clear, parking free   | ON        | OFF     | OFF       | OFF   | 0°    | Dashboard `NORMAL`    |
| Parking occupied only     | OFF       | ON      | OFF       | OFF   | 0°    | Dashboard `PKG:FULL`  |
| Gas > 600 only            | ON        | OFF     | ON        | OFF   | 0°    | Dashboard `WARN:GAS`  |
| Temp > 35°C only          | ON        | OFF     | ON        | ON    | 0°    | Dashboard `WARN:FIRE` |
| Button pressed            | ON/OFF    | ON/OFF  | ON (hold) | —     | 90°   | `EMERGENCY REPORTED`  |
| Gas > 600 AND Temp > 35°C | OFF       | ON      | ON        | ON    | 90°   | `CITY EMERG EVACUATE` |

### 7.2 Code Quality

- 20-sample MQ2 averaging eliminates ADC noise and prevents false buzzer triggers
- `map()` and `constrain()` remap the raw Wokwi MQ2 range to a standard 0–1023 scale, restoring the 600 threshold
- `analogSetAttenuation(ADC_11db)` and `analogReadResolution(12)` ensure correct ESP32 ADC operation
- `isnan()` guard prevents crashes from failed DHT22 reads
- All thresholds defined with `#define` for easy adjustment without touching logic
- All GPIO pins defined with `#define` for clean, readable code
- OLED updates rate-limited to every 500 ms, preventing screen flicker
- Button uses a simple latch (`emergencyPressed`) — correctly models real emergency reporting where alerts stay active until authority resets
- Buzzer follows `btnPressed` directly (not latched) — sounds only while held, stops on release
- `noTone()` and LOW initial states ensure nothing activates at boot
- `Serial.printf()` provides full debug output every cycle for easy monitoring
- Clean function separation: `readUltrasonic()`, `setServo()`, `setBuzzer()`, `showDashboard()`, `showAlert()`

### 7.3 Wiring Arrangement

The circuit is organized with ten components connected to the ESP32 in a structured layout.

- DHT22, MQ2, OLED, and button use the **3.3V rail**
- HC-SR04 and Servo use the **5V rail**
- Both LEDs connect through **220Ω resistors** on their anodes to GPIO 26 and GPIO 27 respectively, with cathodes to common ground
- The HC-SR04 ECHO line uses a **1kΩ–2kΩ voltage divider** to protect the 3.3V GPIO 18 from the sensor's 5V output
- The relay and buzzer connect directly to their GPIO pins
- All components share a **common ground** with the ESP32

### 7.4 Additional Features

- **MQ2 remapping** — Raw Wokwi analog range 843–4095 mathematically remapped to 0–1023 so the project-specified threshold of 600 works correctly in simulation
- **Specific WARNING labels** — STATUS line shows `WARN: FIRE`, `WARN: GAS`, or `WARN:FIRE+GAS` instead of a generic warning, making fault identification instant
- **Dashboard always visible during WARNING** — Full-screen takeover only happens during EMERGENCY conditions; WARNING keeps all 5 dashboard fields readable
- **OLED priority routing** — City Emergency overrides all; then Emergency Reported; then normal dashboard with embedded warning status
- **Splash screen on boot** — System displays project name and author for 2.5 seconds before entering monitoring mode
- **Real-time Serial Monitor logging** — Every sensor value, raw gas reading, mapped gas reading, and all alert states logged every 100 ms

### 7.5 Observations and Results

| Test Condition            | Green LED | Red LED | Buzzer | Relay | Servo | OLED STATUS         | Result  |
| ------------------------- | --------- | ------- | ------ | ----- | ----- | ------------------- | ------- |
| All sensors normal        | ON        | OFF     | OFF    | OFF   | 0°    | NORMAL              | ✅ Pass |
| Distance < 10 cm          | OFF       | ON      | OFF    | OFF   | 0°    | NORMAL (PKG:FULL)   | ✅ Pass |
| Gas > 600 only            | ON        | OFF     | ON     | OFF   | 0°    | WARN: GAS           | ✅ Pass |
| Temp > 35°C only          | ON        | OFF     | ON     | ON    | 0°    | WARN: FIRE          | ✅ Pass |
| Button pressed and held   | —         | —       | ON     | —     | 90°   | EMERGENCY REPORTED  | ✅ Pass |
| Button released           | —         | —       | OFF    | —     | 0°    | NORMAL              | ✅ Pass |
| Gas > 600 AND Temp > 35°C | OFF       | ON      | ON     | ON    | 90°   | CITY EMERG EVACUATE | ✅ Pass |

---

## 8. Conclusion

This project successfully demonstrates a Smart City Emergency Response and Safety Management System using ESP32, multiple sensors, and actuators — simulated entirely on Wokwi. The system automatically responds to parking occupancy, air quality degradation, fire risk temperature, and citizen emergency reports. Each condition triggers appropriate actuators and OLED messages independently and simultaneously without interference. The MQ2 remapping technique, button latching logic, and OLED priority routing together produce a robust, realistic city safety system that provides a solid foundation for real-world smart city emergency infrastructure.

---

### Try it yourself

Open the live simulation on Wokwi: **[https://wokwi.com/projects/466690577837257729](https://wokwi.com/projects/466690577837257729)**
