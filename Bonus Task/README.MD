# 🌱 Next-Gen Agri-Tech: Automated Hydroponic/Aquaponic Guardian

**Platform:** Wokwi Simulation · **Microcontroller:** ESP32 DevKit V1 · **Control System:** PID Control Loop

🔗 **Wokwi Simulation:** [https://wokwi.com/projects/466783068637489153](https://wokwi.com/projects/466783068637489153)

---

## 📌 Project Aim

To design and simulate an intelligent automated monitoring and control system for hydroponic and aquaponic farming environments using an ESP32 microcontroller, PID control algorithms, and multiple sensors — capable of autonomously maintaining optimal pH, nutrient concentration, water level, and temperature conditions without any manual intervention.

## 📖 Project Overview

The Automated Hydroponic/Aquaponic Guardian is an embedded systems project that simulates a smart agricultural control unit for soil-free plant cultivation systems. In hydroponic and aquaponic farming, maintaining precise chemical and environmental parameters is critical for plant health and yield. Manual monitoring is impractical at scale — this system automates the entire process.

The system uses three potentiometers to simulate real-world analog sensors for pH level, electrical conductivity (EC), and water level. A DHT22 sensor provides real ambient temperature and humidity readings. An ESP32 microcontroller continuously reads these values, runs independent PID control loops for pH and EC, and activates the appropriate peristaltic pump relays or exhaust fan relay based on computed outputs.

A 16x2 I2C LCD rotates across three display pages showing live sensor values and system status. Four LED indicators provide instant visual feedback on which pumps and the fan are active. All activity is logged in real time to the Serial Monitor for debugging and verification.

The system is fully **non-blocking** — all timing is handled using `millis()` so sensor reading, PID computation, LCD updates, and pump pulsing all run concurrently without any component stalling another.

---

## 🧩 Components Used

| Component               | Model / Type         | Purpose                                   |
| ----------------------- | -------------------- | ----------------------------------------- |
| Microcontroller         | ESP32 DevKit V1      | Main controller, ADC reads, relay control |
| pH Sensor (simulated)   | Potentiometer (pot1) | Simulates pH sensor — GPIO 34             |
| EC Sensor (simulated)   | Potentiometer (pot3) | Simulates EC sensor — GPIO 35             |
| Water Level (simulated) | Potentiometer (pot2) | Simulates water level — GPIO 32           |
| Temperature & Humidity  | DHT22                | Real ambient temp and humidity — GPIO 4   |
| Display                 | 16x2 LCD via I2C     | Shows sensor values and pump status       |
| Relay Module x4         | Wokwi Relay Module   | Controls pumps and fan                    |
| LED (Red)               | Wokwi LED            | Indicates pH Up pump active               |
| LED (Green)             | Wokwi LED            | Indicates Nutrient pump active            |
| LED (Blue)              | Wokwi LED            | Indicates pH Down pump active             |
| LED (Orange)            | Wokwi LED            | Indicates Exhaust Fan active              |
| Resistors               | 330Ω x4              | Current limiting for LEDs                 |
| Pull-up Resistor        | 10kΩ                 | DHT22 data line pull-up                   |

---

## 🔌 Wiring Details

### ESP32 Pin Assignments

| Component                       | ESP32 Pin        | Wire Color |
| ------------------------------- | ---------------- | ---------- |
| pH Potentiometer (SIG)          | GPIO 34          | Orange     |
| EC Potentiometer (SIG)          | GPIO 35          | Orange     |
| Water Level Potentiometer (SIG) | GPIO 32          | Orange     |
| DHT22 Data (SDA)                | GPIO 4           | Gold       |
| LCD SDA                         | GPIO 21          | Green      |
| LCD SCL                         | GPIO 22          | Green      |
| Relay 1 IN (pH Up pump)         | GPIO 16          | Green      |
| Relay 2 IN (pH Down pump)       | GPIO 17          | Green      |
| Relay 3 IN (Nutrient pump)      | GPIO 18          | Green      |
| Relay 4 IN (Exhaust fan)        | GPIO 19          | Green      |
| LED pH Up (Red)                 | GPIO 25 via 330Ω | Green      |
| LED pH Down (Blue)              | GPIO 26 via 330Ω | Green      |
| LED Nutrient (Green)            | GPIO 27 via 330Ω | Green      |
| LED Fan (Orange)                | GPIO 33 via 330Ω | Green      |

### Power Connections

| Component          | Power      | Ground      |
| ------------------ | ---------- | ----------- |
| All Potentiometers | ESP32 3.3V | ESP32 GND.1 |
| DHT22              | ESP32 3.3V | ESP32 GND.2 |
| LCD                | ESP32 5V   | ESP32 GND.2 |
| All Relay Modules  | ESP32 5V   | ESP32 GND.3 |
| All LEDs (cathode) | —          | ESP32 GND.1 |

### I2C LCD Connection

- LCD SDA → ESP32 GPIO 21
- LCD SCL → ESP32 GPIO 22
- LCD VCC → ESP32 5V
- LCD GND → ESP32 GND
- I2C Address: `0x27`

### Relay Control Logic (Active LOW)

```
digitalWrite(relayPin, LOW)  → Relay ON  → Pump/Fan starts
digitalWrite(relayPin, HIGH) → Relay OFF → Pump/Fan stops
```

---

## 🎯 System Setpoints

| Parameter   | Setpoint | Unit  | Reason                                           |
| ----------- | -------- | ----- | ------------------------------------------------ |
| pH          | 6.2      | —     | Optimal for hydroponic plant nutrient absorption |
| EC          | 1.8      | mS/cm | Ideal nutrient concentration                     |
| Temperature | Max 28.0 | °C    | Maximum safe greenhouse temperature              |
| Water Level | Min 40.0 | %     | Minimum safe reservoir level                     |

## ⚙️ PID Tuning Parameters

| Controller | Kp  | Ki   | Kd  | Output Range |
| ---------- | --- | ---- | --- | ------------ |
| pH PID     | 2.0 | 0.05 | 0.8 | −255 to +255 |
| EC PID     | 3.0 | 0.1  | 0.5 | 0 to +255    |

---

## 🔄 Code Structure / Program Flow

### Startup Sequence

```
ESP32 Powers ON
  → Initialize Serial at 115200 baud
  → Set all relay pins HIGH (OFF), LED pins LOW
  → Set ADC resolution to 12-bit (0–4095)
  → Initialize DHT22
  → Initialize I2C LCD → Show splash screen "Hydroponic Guardian!"
  → Enter main loop
```

### Main Loop — Three Independent Timers

- Every **500ms** → `readSensors()`
- Every **2000ms** → `runPIDControl()` + `printDebug()`
- Every **800ms** → `updateLCD()`
- Every `loop()` → `handlePump()` for all three pumps (non-blocking)

### Sensor Reading Flow

```
readSensors():
  Raw ADC pH (0–4095) → scale to 0.0–14.0 pH
  Raw ADC EC (0–4095) → scale to 0.0–5.0 mS/cm
  Raw ADC WL (0–4095) → scale to 0.0–100.0 %
  DHT22 read → currentTemp, currentHumid (with NaN check)
```

### PID Control Flow

**pH Control**

- Compute pH error = 6.2 − currentPH
- If sign changed → reset integral
- If |error| < 0.1 → dead-band, reset integral
- Run PID → `phOutput`
- `phOutput > 0.3` → pH Up pump ON
- `phOutput < -0.3` → pH Down pump ON
- `|phOutput| ≤ 0.3` → both pumps OFF

**EC Control**

- Compute EC error = 1.8 − currentEC
- If |error| < 0.1 → dead-band, reset integral
- Run PID → `ecOutput`
- `ecOutput > 0.3` → Nutrient pump ON
- `ecOutput ≤ 0.3` → Nutrient pump OFF

**Fan Control (Hysteresis)**

- Temp > 28.5°C → Fan ON
- Temp < 27.5°C → Fan OFF
- Between → maintain current state

**Water Level**

- WL < 40% → LCD displays "LOW!" warning

### Pump Pulse Scheduling

```
schedulePump():
  magnitude < 0.3     → pump inactive
  magnitude 0.3–20     → map to pulse ON time 50ms–500ms
  offTime fixed at 1000ms between pulses

handlePump() (called every loop):
  Non-blocking pulse handler using millis()
  If pump active and ON time elapsed  → turn OFF
  If pump active and OFF time elapsed → turn ON again
```

### LCD Page Rotation

| Page | Content (every 0.8s)                             |
| ---- | ------------------------------------------------ |
| 1    | pH value & setpoint, EC value & setpoint         |
| 2    | Temperature, Humidity, Water Level + LOW warning |
| 3    | Pump status (UP/DN/NUT) and Fan status           |

Cycles back to Page 1 continuously.

---

## 📊 Automation Logic Summary

| Condition                              | pH Up Pump   | pH Down Pump | Nutrient Pump | Fan | LCD Alert |
| -------------------------------------- | ------------ | ------------ | ------------- | --- | --------- |
| pH = 6.2, EC = 1.8, T = 24°C, WL = 75% | OFF          | OFF          | OFF           | OFF | Normal    |
| pH < 6.2 (low)                         | ON (pulsing) | OFF          | —             | —   | —         |
| pH > 6.2 (high)                        | OFF          | ON (pulsing) | —             | —   | —         |
| EC < 1.8 (low)                         | —            | —            | ON (pulsing)  | —   | —         |
| EC > 1.8 (high)                        | —            | —            | OFF           | —   | —         |
| Temp > 28.5°C                          | —            | —            | —             | ON  | —         |
| Temp < 27.5°C                          | —            | —            | —             | OFF | —         |
| Water Level < 40%                      | —            | —            | —             | —   | "LOW!"    |

---

## 💻 Source Code

```cpp
/*
============================================================
  Automated Hydroponic/Aquaponic Guardian
  ESP32 + PID Control Loop
  Wokwi Simulation Project
============================================================
  Components:
  - ESP32 DevKit V1
  - Potentiometers (simulate pH, EC, Water Level sensors)
  - DHT22 (ambient temp/humidity)
  - 16x2 LCD via I2C (SDA=GPIO21, SCL=GPIO22)
  - Relay Module x4 (pH Up pump, pH Down pump,
    Nutrient pump, Exhaust fan)
  - LED indicators for each relay
============================================================
*/
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ─── Pin Definitions ─────────────────────────────────────────
#define PIN_PH_SENSOR      34  // ADC1 - pH potentiometer
#define PIN_EC_SENSOR      35  // ADC1 - EC potentiometer
#define PIN_WLEVEL_SENSOR  32  // ADC1 - Water Level pot
#define PIN_DHT            4   // DHT22 data pin

#define RELAY_PH_UP        16  // Peristaltic pump - pH Up solution
#define RELAY_PH_DOWN      17  // Peristaltic pump - pH Down solution
#define RELAY_NUTRIENT     18  // Peristaltic pump - Nutrient solution
#define RELAY_FAN          19  // Exhaust fan relay

#define LED_PH_UP          25
#define LED_PH_DOWN        26
#define LED_NUTRIENT       27
#define LED_FAN            33

// ─── I2C LCD ─────────────────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ─── DHT22 ───────────────────────────────────────────────────
DHT dht(PIN_DHT, DHT22);

// ─── Setpoints ───────────────────────────────────────────────
const float PH_SETPOINT = 6.2;
const float EC_SETPOINT = 1.8;
const float TEMP_MAX = 28.0;
const float WATER_LEVEL_MIN = 40.0;

// ─── PID Config ──────────────────────────────────────────────
struct PIDConfig {
  float Kp, Ki, Kd;
  float integral, prevError;
  float outputMin, outputMax;
};

PIDConfig pidPH = {2.0, 0.05, 0.8, 0, 0, -255, 255};
PIDConfig pidEC = {3.0, 0.10, 0.5, 0, 0, 0, 255};

// ─── Pump State ──────────────────────────────────────────────
struct PumpState {
  bool active;
  unsigned long onTime, offTime, lastToggle;
  bool isOn;
};

PumpState pHUpPump    = {false, 0, 1000, 0, false};
PumpState pHDownPump  = {false, 0, 1000, 0, false};
PumpState nutrientPump = {false, 0, 1000, 0, false};

// ─── Timing ──────────────────────────────────────────────────
unsigned long lastSensorRead = 0, lastLCDUpdate = 0, lastPIDRun = 0;
const unsigned long SENSOR_INTERVAL = 500;
const unsigned long LCD_INTERVAL = 800;
const unsigned long PID_INTERVAL = 2000;

// ─── Sensor Values ───────────────────────────────────────────
float currentPH = 6.2, currentEC = 1.8;
float currentWLevel = 80.0;
float currentTemp = 24.0, currentHumid = 60.0;
int lcdPage = 0;

// ─── Relay control (active LOW) ──────────────────────────────
void setRelay(int pin, int ledPin, bool state) {
  digitalWrite(pin, state ? LOW : HIGH);
  digitalWrite(ledPin, state ? HIGH : LOW);
}

// ─── PID Compute ─────────────────────────────────────────────
float computePID(PIDConfig &cfg, float setpoint, float measured, float dt) {
  float error = setpoint - measured;
  cfg.integral += error * dt;
  cfg.integral = constrain(cfg.integral, cfg.outputMin / cfg.Ki, cfg.outputMax / cfg.Ki);
  float derivative = (error - cfg.prevError) / dt;
  cfg.prevError = error;
  return constrain(
    cfg.Kp * error + cfg.Ki * cfg.integral + cfg.Kd * derivative,
    cfg.outputMin, cfg.outputMax);
}

// ─── Schedule pump pulse from PID magnitude ──────────────────
void schedulePump(PumpState &pump, float magnitude) {
  if (magnitude < 0.3) { pump.active = pump.isOn = false; return; }
  pump.active = true;
  float clamped = constrain(magnitude, 0.3, 20.0);
  pump.onTime = (unsigned long)map((long)(clamped * 10), 3, 200, 50, 500);
  pump.offTime = 1000;
}

// ─── Non-blocking pump handler ───────────────────────────────
void handlePump(PumpState &pump, int relayPin, int ledPin) {
  if (!pump.active) { setRelay(relayPin, ledPin, false); return; }
  unsigned long now = millis();
  if (pump.isOn && now - pump.lastToggle >= pump.onTime) {
    pump.isOn = false; pump.lastToggle = now;
    setRelay(relayPin, ledPin, false);
  } else if (!pump.isOn && now - pump.lastToggle >= pump.offTime) {
    pump.isOn = true; pump.lastToggle = now;
    setRelay(relayPin, ledPin, true);
  }
}

// ─── Read sensors ────────────────────────────────────────────
void readSensors() {
  currentPH = (analogRead(PIN_PH_SENSOR) / 4095.0) * 14.0;
  currentEC = (analogRead(PIN_EC_SENSOR) / 4095.0) * 5.0;
  currentWLevel = (analogRead(PIN_WLEVEL_SENSOR) / 4095.0) * 100.0;
  float t = dht.readTemperature(), h = dht.readHumidity();
  if (!isnan(t)) currentTemp = t;
  if (!isnan(h)) currentHumid = h;
}

// ─── LCD update (3-page rotation) ────────────────────────────
void updateLCD() {
  lcd.clear();
  if (lcdPage == 0) {
    lcd.setCursor(0,0); lcd.print("pH:"); lcd.print(currentPH,1);
    lcd.print(" SP:"); lcd.print(PH_SETPOINT,1);
    lcd.setCursor(0,1); lcd.print("EC:"); lcd.print(currentEC,2);
    lcd.print(" SP:"); lcd.print(EC_SETPOINT,1);
  } else if (lcdPage == 1) {
    lcd.setCursor(0,0); lcd.print("T:"); lcd.print(currentTemp,1);
    lcd.print("C H:"); lcd.print(currentHumid,0); lcd.print("%");
    lcd.setCursor(0,1); lcd.print("WLvl:"); lcd.print(currentWLevel,0);
    lcd.print("%");
    if (currentWLevel < WATER_LEVEL_MIN) lcd.print(" LOW!");
  } else {
    lcd.setCursor(0,0);
    lcd.print("UP:"); lcd.print(pHUpPump.active ? "ON " : "OFF");
    lcd.print(" DN:"); lcd.print(pHDownPump.active ? "ON" : "OFF");
    lcd.setCursor(0,1);
    lcd.print("NUT:"); lcd.print(nutrientPump.active ? "ON " : "OFF");
    lcd.print(" FAN:"); lcd.print(currentTemp > TEMP_MAX ? "ON" : "OF");
  }
  lcdPage = (lcdPage + 1) % 3;
}

// ─── PID control runner ───────────────────────────────────────
void runPIDControl(float dt) {
  // pH control
  float phError = PH_SETPOINT - currentPH;
  if ((phError > 0 && pidPH.prevError < 0) ||
      (phError < 0 && pidPH.prevError > 0)) pidPH.integral = 0;
  if (abs(phError) < 0.1) pidPH.integral = 0;
  float phOutput = computePID(pidPH, PH_SETPOINT, currentPH, dt);
  if (phOutput > 0.3) {
    schedulePump(pHUpPump, phOutput);
    schedulePump(pHDownPump, 0); pHDownPump.active = false;
  } else if (phOutput < -0.3) {
    schedulePump(pHDownPump, abs(phOutput));
    schedulePump(pHUpPump, 0); pHUpPump.active = false;
  } else {
    pidPH.integral = 0;
    schedulePump(pHUpPump, 0); schedulePump(pHDownPump, 0);
    pHUpPump.active = pHDownPump.active = false;
  }

  // EC control
  float ecOutput = computePID(pidEC, EC_SETPOINT, currentEC, dt);
  if (abs(EC_SETPOINT - currentEC) < 0.1) pidEC.integral = 0;
  if (ecOutput > 0.3) schedulePump(nutrientPump, ecOutput);
  else { pidEC.integral = 0; schedulePump(nutrientPump, 0);
    nutrientPump.active = false; }

  // Fan control with hysteresis
  static bool fanOn = false;
  if (currentTemp > TEMP_MAX + 0.5) fanOn = true;
  else if (currentTemp < TEMP_MAX - 0.5) fanOn = false;
  setRelay(RELAY_FAN, LED_FAN, fanOn);
}

// ─── Serial debug ─────────────────────────────────────────────
void printDebug() {
  Serial.printf("[SENSOR] pH=%.2f | EC=%.2f | WL=%.1f%% | T=%.1fC | H=%.1f%%\n",
    currentPH, currentEC, currentWLevel, currentTemp, currentHumid);
  Serial.printf("[PID] pH_err=%.2f | EC_err=%.2f\n",
    PH_SETPOINT - currentPH, EC_SETPOINT - currentEC);
  Serial.printf("[PUMPS] pH_UP=%s | pH_DN=%s | NUT=%s | FAN=%s\n",
    pHUpPump.active ? "ON" : "off", pHDownPump.active ? "ON" : "off",
    nutrientPump.active ? "ON" : "off", (currentTemp > TEMP_MAX) ? "ON" : "off");
  Serial.println("─────────────────────────────────────");
}

void setup() {
  Serial.begin(115200);
  int relays[] = {RELAY_PH_UP, RELAY_PH_DOWN, RELAY_NUTRIENT, RELAY_FAN};
  int leds[] = {LED_PH_UP, LED_PH_DOWN, LED_NUTRIENT, LED_FAN};
  for (int i = 0; i < 4; i++) {
    pinMode(relays[i], OUTPUT); pinMode(leds[i], OUTPUT);
    digitalWrite(relays[i], HIGH); digitalWrite(leds[i], LOW);
  }
  analogReadResolution(12);
  dht.begin();
  Wire.begin(21, 22);
  lcd.init(); lcd.backlight();
  lcd.setCursor(0,0); lcd.print(" Hydroponic ");
  lcd.setCursor(0,1); lcd.print(" Guardian! ");
  delay(2000); lcd.clear();
  Serial.println("=== Hydroponic Guardian Boot ===");
}

void loop() {
  unsigned long now = millis();
  if (now - lastSensorRead >= SENSOR_INTERVAL) {
    readSensors(); lastSensorRead = now; }
  if (now - lastPIDRun >= PID_INTERVAL) {
    runPIDControl(PID_INTERVAL / 1000.0);
    printDebug(); lastPIDRun = now; }
  if (now - lastLCDUpdate >= LCD_INTERVAL) {
    updateLCD(); lastLCDUpdate = now; }
  handlePump(pHUpPump, RELAY_PH_UP, LED_PH_UP);
  handlePump(pHDownPump, RELAY_PH_DOWN, LED_PH_DOWN);
  handlePump(nutrientPump, RELAY_NUTRIENT, LED_NUTRIENT);
}
```

---

## 🖥️ Sample Serial Monitor Output

```
=== Hydroponic Guardian Boot ===
[SENSOR] pH=5.80 | EC=1.20 | WL=75.0% | T=24.5C | H=62.0%
[PID] pH_err=+0.40 | EC_err=+0.60
[PUMPS] pH_UP=ON | pH_DN=off | NUT=ON | FAN=off
─────────────────────────────────────
[SENSOR] pH=6.10 | EC=1.65 | WL=75.0% | T=29.2C | H=60.0%
[PID] pH_err=+0.10 | EC_err=+0.15
[PUMPS] pH_UP=ON | pH_DN=off | NUT=ON | FAN=ON
─────────────────────────────────────
[SENSOR] pH=6.20 | EC=1.80 | WL=75.0% | T=27.0C | H=60.0%
[PID] pH_err=0.00 | EC_err=0.00
[PUMPS] pH_UP=off | pH_DN=off | NUT=off | FAN=off
─────────────────────────────────────
```

---

## ✅ Test Cases

| TC ID | Scenario               | Input Condition                | Output                              | Pass Criteria              |
| ----- | ---------------------- | ------------------------------ | ----------------------------------- | -------------------------- |
| TC01  | Normal Stable State    | pH=6.2, EC=1.8, WL=75%, T=24°C | All pumps OFF                       | System in equilibrium      |
| TC02  | pH Too Low (Small)     | pH≈5.8                         | pH Up pump short pulse              | Proportional pulse         |
| TC03  | pH Too Low (Large)     | pH≈2.0                         | pH Up pump long pulse               | Max pulse 500ms            |
| TC04  | pH Too High (Small)    | pH≈6.7                         | pH Down pump short pulse            | pH Down only               |
| TC05  | pH Too High (Large)    | pH≈14.0                        | pH Down pump long pulse             | Max pH Down pulse          |
| TC06  | EC Too Low             | EC≈0.5                         | Nutrient pump pulsing               | Proportional to deficit    |
| TC07  | EC Too High            | EC≈3.5                         | Nutrient pump OFF                   | No activation              |
| TC08  | Low Water Level        | WL≈32%                         | LCD shows "LOW!"                    | Warning below 40%          |
| TC09  | Critical Water Level   | WL≈5%                          | LCD shows "LOW!"                    | No system crash            |
| TC10  | High Temperature       | T≈29.5°C                       | Fan ON, orange LED ON               | Fan above 28.5°C           |
| TC11  | Fan Hysteresis         | T drops 29.5→27°C              | Fan OFF at 27.5°C                   | No rapid toggling          |
| TC12  | pH and EC Low Together | pH≈4.0, EC≈0.3                 | pH Up + Nutrient both ON            | Independent operation      |
| TC13  | Worst Case Fault       | pH≈0.5, EC≈0.1, WL≈5%, T≈30°C  | All subsystems activate             | No freeze or crash         |
| TC14  | PID Pulse Verification | pH sweep 2.0→6.2               | Pulse width reduces gradually       | Proportional PID confirmed |
| TC15  | LCD Page Rotation      | System idle                    | Page 1→2→3→1 every 0.8s             | Smooth rotation            |
| TC16  | System Boot            | ESP32 Power ON                 | Splash screen then normal operation | Successful boot            |

---

## ✨ Additional Features and Enhancements

- **PID Control with Anti-Windup** — Both pH and EC controllers implement integral clamping to prevent the integral term from accumulating excessively when the actuator is saturated, avoiding overshoot and oscillation.
- **Dead-band Logic** — A ±0.1 dead-band around both pH and EC setpoints prevents micro-corrections from continuously pulsing the pumps at equilibrium, extending pump lifespan.
- **Integral Reset on Sign Change** — The pH PID resets its integral when the error crosses zero direction, giving a cleaner step response.
- **Hysteresis on Fan Control** — A ±0.5°C hysteresis band around the 28°C setpoint prevents rapid ON/OFF switching.
- **Non-blocking Pump Pulsing** — A dedicated `PumpState` struct with `millis()`-based timing lets all three pumps pulse independently and simultaneously.
- **Three-page LCD Rotation** — Cycles through pH/EC, temperature/humidity/water level, and pump status every 800ms.
- **Structured Serial Debug Output** — Clearly labelled SENSOR, PID, and PUMPS sections for easy tracing and diagnosis.
- **Modular Code Architecture** — Separated into `readSensors()`, `computePID()`, `schedulePump()`, `handlePump()`, `updateLCD()`, `runPIDControl()`, and `printDebug()` for independent testability and maintainability.

---

## 🏁 Conclusion

The Automated Hydroponic/Aquaponic Guardian successfully demonstrates an intelligent, fully autonomous agricultural control system simulated on the Wokwi platform using an ESP32 microcontroller. The project integrates real-time multi-sensor data acquisition, dual-loop PID control, relay-driven actuator management, and a rotating LCD display into a single cohesive embedded system.

The pH PID controller accurately detects deviations from the 6.2 setpoint and activates the appropriate peristaltic pump — dosing pH Up solution when the water is too acidic and pH Down solution when it is too alkaline — with pulse widths proportional to the magnitude of the error. The EC PID controller independently monitors nutrient concentration and activates the nutrient pump only when levels fall below the 1.8 mS/cm setpoint. The exhaust fan operates with a hysteresis band around 28°C, and water level monitoring provides a low reservoir warning on the LCD when levels drop below 40%.

Key engineering principles demonstrated: non-blocking multitasking using `millis()` timers, anti-windup integral clamping, dead-band logic, integral reset on sign change, and a modular code architecture separating sensing, computation, actuation, and display.

In real-world deployment, this system would replace manual testing and dosing routines that farmers currently perform multiple times daily — reducing labour costs, eliminating human error, and ensuring plants consistently receive optimal growing conditions regardless of time of day or environmental fluctuations. The design is fully extensible, allowing additional sensors such as dissolved oxygen, turbidity, or CO₂ concentration to be integrated with minimal code changes.

---

**Author:** Sreya Vinod
