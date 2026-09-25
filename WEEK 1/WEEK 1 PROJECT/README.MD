# 🛰️ Smart Surveillance and Response Network

**Platform:** Tinkercad Simulation · **Microcontrollers:** 3x Arduino Uno (UART-linked)

🔗 **Tinkercad Simulation:** [View Project](https://www.tinkercad.com/things/2AUZHA7V7Wl-distributedsurveillance-system?sharecode=ltMlGHgRWwbyFRbAIelcFrWL2v11-MEVTb8QZNJfrKI)

---

## 📌 Project Aim

To design and simulate a distributed surveillance system using three Arduino Uno boards communicating through UART Serial Communication, capable of detecting environmental threats using sensors and responding automatically with appropriate alert actions based on classified threat levels.

---

## 📖 Project Overview

The Smart Surveillance and Response Network is a multi-node embedded system that mimics a real-world distributed security setup. The system is divided into three roles — a **Surveillance Node (Slave 1)**, a **Master Controller**, and a **Response Node (Slave 2)** — all communicating through UART Serial Communication.

Slave 1 continuously reads data from a PIR motion sensor, a TMP36 temperature sensor, and an LDR light sensor, then transmits the readings to the Master every 2 seconds. The Master receives and parses this data, applies threat classification logic, and sends a threat command to Slave 2. Slave 2 receives the command and activates the appropriate output — Green LED for low threat, Yellow LED for medium threat, Red LED with Buzzer for high threat, and Red LED with Buzzer and Servo Motor rotation for critical threat. The entire system operates automatically and continuously without any manual intervention during runtime.

---

## 🧩 Components Used

| Component                      | Quantity    |
| ------------------------------ | ----------- |
| Arduino Uno                    | 3           |
| PIR Motion Sensor              | 1           |
| TMP36 Temperature Sensor       | 1           |
| LDR (Light Dependent Resistor) | 1           |
| 10kΩ Resistor (LDR pull-down)  | 1           |
| Green LED                      | 1           |
| Yellow LED                     | 1           |
| Red LED                        | 1           |
| 220Ω Resistor (per LED)        | 3           |
| Piezo Buzzer                   | 1           |
| Micro Servo Motor              | 1           |
| Breadboard                     | 2           |
| Jumper Wires                   | As required |

---

## 🔌 Construction and Wiring Details

### Slave 1 — Surveillance Node

| Component              | Arduino 1 Pin        |
| ---------------------- | -------------------- |
| PIR Sensor VCC         | 5V                   |
| PIR Sensor GND         | GND                  |
| PIR Sensor OUT         | Pin 7                |
| TMP36 Left pin (VCC)   | 5V                   |
| TMP36 Middle pin (OUT) | A0                   |
| TMP36 Right pin (GND)  | GND                  |
| LDR one leg            | 5V                   |
| LDR other leg          | A1                   |
| 10kΩ resistor          | A1 to GND            |
| SoftwareSerial TX      | Pin 3 → Master Pin 2 |
| GND                    | Common GND rail      |

### Master Controller

| Connection                     | Pin             |
| ------------------------------ | --------------- |
| SoftwareSerial RX from Slave 1 | Pin 2           |
| Hardware Serial TX to Slave 2  | Pin 1           |
| GND                            | Common GND rail |

### Slave 2 — Response Node

| Component                      | Arduino 3 Pin   |
| ------------------------------ | --------------- |
| Green LED anode (+) via 220Ω   | Pin 4           |
| Yellow LED anode (+) via 220Ω  | Pin 5           |
| Red LED anode (+) via 220Ω     | Pin 6           |
| All LED cathodes (−)           | GND             |
| Buzzer positive (+)            | Pin 7           |
| Buzzer negative (−)            | GND             |
| Servo signal (orange)          | Pin 9           |
| Servo VCC (red)                | 5V              |
| Servo GND (brown)              | GND             |
| Hardware Serial RX from Master | Pin 0           |
| GND                            | Common GND rail |

### Serial Communication Links

| Link             | From                   | To                 |
| ---------------- | ---------------------- | ------------------ |
| Slave 1 → Master | Slave 1 Pin 3 (TX)     | Master Pin 2 (RX)  |
| Master → Slave 2 | Master Pin 1 (TX)      | Slave 2 Pin 0 (RX) |
| Common Ground    | All three Arduino GNDs | Shared GND rail    |

---

## 🔄 Code Structure / Program Flow

### Slave 1 — Program Flow

1. Initialize Serial Monitor at 9600 baud and SoftwareSerial to Master at 4800 baud.
2. Wait 2 seconds for PIR sensor to stabilize on startup.
3. Every 2 seconds, read PIR (digital), TMP36 (analog A0), and LDR (analog A1).
4. Convert raw TMP36 analog value to Celsius using voltage formula.
5. Format readings as a string: `M:x,T:xx,L:xxx`
6. Transmit formatted string to Master via SoftwareSerial on Pin 3.
7. Print sensor values to Serial Monitor.
8. Repeat continuously.

### Master Controller — Program Flow

1. Initialize Hardware Serial at 4800 baud (for Slave 2) and SoftwareSerial at 4800 baud (from Slave 1).
2. Continuously listen on SoftwareSerial Pin 2 for incoming data from Slave 1.
3. Parse received string to extract motion, temperature, and light values.
4. Apply threat classification logic based on the four conditions.
5. Print analysis result to Serial Monitor.
6. Send threat command string (`LOW` / `MEDIUM` / `HIGH` / `CRITICAL`) to Slave 2 via Hardware Serial Pin 1.

### Slave 2 — Program Flow

1. Initialize Hardware Serial at 4800 baud to receive commands from Master.
2. Initialize all output pins and set everything OFF on startup.
3. Continuously listen for incoming command string.
4. On receiving a command, reset all outputs first using `allOff()`.
5. Activate the appropriate output based on the command received.
6. Print response action to Serial Monitor.

---

## 🎯 Threat Classification Logic

| Motion | Temperature     | Light | Threat Level | Response                     |
| ------ | --------------- | ----- | ------------ | ---------------------------- |
| 0      | ≤ 35°C          | ≥ 300 | LOW          | Green LED ON                 |
| 0      | > 35°C or < 300 | —     | MEDIUM       | Yellow LED ON                |
| 1      | ≤ 35°C          | ≥ 300 | HIGH         | Red LED + Buzzer ON          |
| 1      | > 35°C or < 300 | —     | CRITICAL     | Red LED + Buzzer + Servo 90° |

---

## 🗺️ System Flow Chart

```
SLAVE 1 (Surveillance)         MASTER (Controller)              SLAVE 2 (Response)
─────────────────────         ────────────────────              ──────────────────
START/POWER ON                 START/POWER ON                    START/POWER ON
Init Serial(9600)               Init HW Serial(4800)              Init HW Serial(4800)
SoftSerial→Master(4800)         SoftSerial from Slave1(4800)      Attach Servo, all OFF
PIR stabilize delay 2s          Start listening
        │                              │                                 │
Read Sensors                    Data received from Slave1?        Command received?
PIR→Pin7 | TMP36→A0 | LDR→A1           │ YES                             │ YES
        │                       Parse String: M,T,L                allOff() reset
Format Data String                     │                                 │
"M:x,T:xx,L:xxx"                Classify Threat Level             Which command?
        │                       ┌─────┬────────┬──────┬─────────┐        │
Send to Master via Pin 3        LOW   MEDIUM   HIGH   CRITICAL   ┌───┬─────┬─────┬────────┐
(SoftwareSerial)                                                  LOW MEDIUM HIGH CRITICAL
        │                       Send Command via Pin 1           Green  Yellow  Red+   Red+
Print to Serial Monitor         (Hardware Serial)                LED    LED    Buzzer Buzzer+
        │                              │                                        Servo90°
Wait 2s, repeat loop             Listen for next data              Wait for next command
```

---

## 💻 Source Code

### Arduino 1 — Slave 1 (Surveillance Node)

```cpp
#include <SoftwareSerial.h>

// SoftwareSerial: TX on Pin 3 to Master Pin 2
SoftwareSerial toMaster(4, 3); // RX=4(unused), TX=3

const int PIR_PIN = 7;   // PIR motion sensor output
const int TEMP_PIN = A0; // TMP36 temperature sensor
const int LDR_PIN = A1;  // LDR light sensor

unsigned long lastSendTime = 0;

void setup() {
  Serial.begin(9600);   // Serial Monitor
  toMaster.begin(4800); // Communication to Master
  pinMode(PIR_PIN, INPUT);
  delay(2000); // Allow PIR to stabilize

  Serial.println("=== Slave 1: Surveillance Node ===");
  Serial.println("Sending data every 2 seconds...");
}

// Convert raw analog reading to Celsius
int readTemperature() {
  int raw = analogRead(TEMP_PIN);
  float voltage = raw * (5.0 / 1023.0);
  return (int)((voltage - 0.5) * 100.0);
}

void loop() {
  unsigned long now = millis();

  // Send sensor data every 2 seconds
  if (now - lastSendTime >= 2000) {
    lastSendTime = now;

    int motion = digitalRead(PIR_PIN);
    int temp = readTemperature();
    int light = analogRead(LDR_PIN);

    // Format and send data string to Master
    String data = "M:" + String(motion) +
                  ",T:" + String(temp) +
                  ",L:" + String(light);
    toMaster.println(data);

    // Display on Serial Monitor
    Serial.println("--- Sensor Reading ---");
    Serial.print("Motion : "); Serial.println(motion);
    Serial.print("Temp   : "); Serial.print(temp); Serial.println("C");
    Serial.print("Light  : "); Serial.println(light);
  }
}
```

### Arduino 2 — Master Controller

```cpp
#include <SoftwareSerial.h>

// Receive from Slave 1 on Pin 2
SoftwareSerial fromSlave1(2, 6); // RX=Pin2, TX=Pin6(unused)

// Send to Slave 2 via Hardware Serial Pin 1 (TX)
void setup() {
  Serial.begin(4800);    // Hardware Serial to Slave 2
  fromSlave1.begin(4800); // SoftwareSerial from Slave 1
  fromSlave1.listen();
}

// Classify threat based on sensor values
String classify(int motion, int temp, int light) {
  if (motion == 0 && temp <= 35 && light >= 300) return "LOW";
  if (motion == 0 && (temp > 35 || light < 300)) return "MEDIUM";
  if (motion == 1 && temp <= 35 && light >= 300) return "HIGH";
  return "CRITICAL";
}

void loop() {
  fromSlave1.listen();

  if (fromSlave1.available()) {
    String data = fromSlave1.readStringUntil('\n');
    data.trim();
    if (data.length() < 5) return;

    // Parse incoming data string
    int mIdx = data.indexOf("M:");
    int tIdx = data.indexOf(",T:");
    int lIdx = data.indexOf(",L:");
    if (mIdx == -1 || tIdx == -1 || lIdx == -1) return;

    int motion = data.substring(mIdx + 2, tIdx).toInt();
    int temp = data.substring(tIdx + 3, lIdx).toInt();
    int light = data.substring(lIdx + 3).toInt();

    // Determine threat level
    String threat = classify(motion, temp, light);

    // Display analysis on Serial Monitor
    Serial.println("--- Master Analysis ---");
    Serial.print("Motion : "); Serial.println(motion);
    Serial.print("Temp   : "); Serial.print(temp); Serial.println("C");
    Serial.print("Light  : "); Serial.println(light);
    Serial.print("Threat : "); Serial.println(threat);
    Serial.println("-----------------------");

    // Send command to Slave 2
    Serial.println(threat);
  }
}
```

### Arduino 3 — Slave 2 (Response Node)

```cpp
#include <Servo.h>

// Receives commands from Master via Hardware Serial Pin 0
Servo myServo;

// Output pin definitions
const int GREEN = 4;   // Green LED
const int YELLOW = 5;  // Yellow LED
const int RED = 6;     // Red LED
const int BUZZER = 7;  // Buzzer
const int SERVO = 9;   // Servo Motor

// Turn all outputs OFF
void allOff() {
  digitalWrite(GREEN, LOW);
  digitalWrite(YELLOW, LOW);
  digitalWrite(RED, LOW);
  digitalWrite(BUZZER, LOW);
  myServo.write(0);
}

void setup() {
  Serial.begin(4800); // Receive from Master
  myServo.attach(SERVO);
  pinMode(GREEN, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  allOff(); // Safe initial state

  Serial.println("=== Slave 2 Ready ===");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() == 0) return;

    allOff(); // Reset before applying new response
    Serial.print("CMD: "); Serial.println(cmd);

    if (cmd == "LOW") {
      digitalWrite(GREEN, HIGH); // Green LED ON
      Serial.println("Green LED ON");
    }
    else if (cmd == "MEDIUM") {
      digitalWrite(YELLOW, HIGH); // Yellow LED ON
      Serial.println("Yellow LED ON");
    }
    else if (cmd == "HIGH") {
      digitalWrite(RED, HIGH);    // Red LED ON
      digitalWrite(BUZZER, HIGH); // Buzzer ON
      Serial.println("Red LED + Buzzer ON");
    }
    else if (cmd == "CRITICAL") {
      digitalWrite(RED, HIGH);    // Red LED ON
      digitalWrite(BUZZER, HIGH); // Buzzer ON
      myServo.write(90);          // Servo rotates 90 degrees
      Serial.println("Red + Buzzer + Servo 90");
    }
  }
}
```

---

## 🖥️ Sample Serial Monitor Output

### Slave 1 Serial Monitor

```
=== Slave 1: Surveillance Node ===
Sending data every 2 seconds...
--- Sensor Reading ---
Motion : 0
Temp   : 24C
Light  : 650
--- Sensor Reading ---
Motion : 1
Temp   : 24C
Light  : 650
```

### Master Serial Monitor

```
--- Master Analysis ---
Motion : 0
Temp   : 24C
Light  : 650
Threat : LOW
-----------------------
--- Master Analysis ---
Motion : 1
Temp   : 24C
Light  : 650
Threat : HIGH
-----------------------
```

### Slave 2 Serial Monitor

```
=== Slave 2 Ready ===
CMD: LOW
Green LED ON
CMD: HIGH
Red LED + Buzzer ON
CMD: CRITICAL
Red + Buzzer + Servo 90
```

---

## ✨ Additional Features and Enhancements

The following enhancements were implemented beyond the basic requirements to improve the system:

1. **Non-blocking timing** — The system uses `millis()` instead of `delay()` in Slave 1, allowing continuous sensor monitoring without freezing the microcontroller between transmissions.
2. **Robust data parsing** — The Master uses `indexOf()` string parsing to extract individual sensor values from the formatted data string, making the communication protocol reliable and error-tolerant with length validation before parsing.
3. **`allOff()` safety function** — Slave 2 resets all outputs before applying any new command, preventing overlapping states such as multiple LEDs being ON simultaneously.
4. **Hybrid Serial architecture** — The Master uses SoftwareSerial to receive from Slave 1 and Hardware Serial to transmit to Slave 2, eliminating the two-SoftwareSerial conflict that causes communication failure in Tinkercad simulation, making the design more stable and reliable.
5. **Startup stabilization delay** — Slave 1 waits 2 seconds on startup before reading the PIR sensor, preventing false motion detections during power-on initialization.

---

**Author:** Sreya Vinod
