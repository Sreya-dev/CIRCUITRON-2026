# 🅿️ Smart Parking Gate System

**Platform:** Tinkercad Simulation · **Microcontroller:** Arduino Uno R3

🔗 **Tinkercad Simulation:** [View Project](https://www.tinkercad.com/things/8nZjaLdhWIX-smartparking-gatesystem/editel?sharecode=K1IvKPa849jcZ33FIbddE1QIXUbya8kxRpVyEi6s9Zo)

---

## 📌 Project Aim

To design, implement, and verify an automated Smart Parking Gate System using an Arduino Uno. The project integrates multiple sensors to automate gate entry operations, dynamically adapt behavior based on ambient light levels (Day vs. Night), and enforce safety overrides via an integrated vehicle overstay alarm.

---

## 📖 Project Overview

The system manages a parking entrance by processing environmental data through three sensor layers:

- **Daytime Protocol** — When high light intensity is detected, the gate opens to 90° for exactly 3 seconds, only if a vehicle is within 20 cm AND the PIR sensor detects motion.
- **Nighttime Protocol** — When low light intensity is detected, a buzzer sounds an alert tone for exactly 2 seconds before the servo operates, signaling the gate to open for 3 seconds.
- **Safety Protocols** — LEDs indicate gate status (Green for Open, Red for Closed). If any vehicle blocks the gate perimeter for more than 5 seconds, a continuous critical alarm triggers until the path is cleared.

---

## 🧩 Components Used

| Component         | Type                            | Quantity |
| ----------------- | ------------------------------- | -------- |
| Microcontroller   | Arduino Uno R3                  | 1        |
| Proximity Sensor  | HC-SR04 4-Pin Ultrasonic Sensor | 1        |
| Motion Sensor     | PIR Motion Sensor               | 1        |
| Light Sensor      | LDR Photoresistor               | 1        |
| Actuator          | Micro Servo Motor               | 1        |
| Audio Indicator   | Piezo Buzzer                    | 1        |
| Visual Indicators | 5mm LEDs (Green, Red)           | 2        |
| Resistors         | 220Ω (LEDs) x2, 10kΩ (LDR) x1   | 3        |
| Prototyping       | Breadboard & Jumper Wires       | —        |

---

## 🔌 Construction and Wiring Details

All components share a common ground (GND) and power bus (+5V) connected to the Arduino Uno.

| Component Group       | Component Pin      | Arduino Uno Pin / Power Rail    | Wiring Practice                                                  |
| --------------------- | ------------------ | ------------------------------- | ---------------------------------------------------------------- |
| Microcontroller Power | 5V Out / GND       | Breadboard Rail (+) / (-)       | Shared power plane for all components                            |
| HC-SR04 Proximity     | VCC / GND          | Power Rail (+) / (-)            | Direct power rail connection                                     |
|                       | Trig / Echo        | Digital Pin 4 / Pin 5           | Short signal pathways to minimize noise                          |
| PIR Motion Sensor     | Power / Ground     | Power Rail (+) / (-)            | Connected to shared power rails                                  |
|                       | Signal Out         | Digital Pin 3                   | Wired as a direct digital input                                  |
| LDR Photoresistor     | Terminal 1         | Power Rail (+)                  | Connected to the 5V supply line                                  |
|                       | Terminal 2         | Analog Pin A0                   | Anchored with a 10kΩ resistor to GND to create a voltage divider |
| Micro Servo Motor     | Power / Ground     | Power Rail (+) / (-)            | Runs to main rails to isolate peak motor currents                |
|                       | PWM Signal         | Digital Pin 9                   | Managed via standard hardware timer frequencies                  |
| Acoustic Buzzer       | Positive (+) / (-) | Digital Pin 8 / Ground Rail (-) | Driven via frequency output cycles                               |
| LED Indicators        | Green Anode (+)    | Digital Pin 6                   | Wired in series with a 220Ω resistor to limit current            |
|                       | Red Anode (+)      | Digital Pin 7                   | Wired in series with a 220Ω resistor to limit current            |

---

## 🔄 Program Flow

```
START
  → POLL SENSORS LOOP
  → Is Vehicle Within 20 cm?
        YES → Track Arrival Time
        NO  → Reset Arrival Timer
  → EVALUATE STATE MATRIX
        ├── CLOSED        : Servo 0°, Red LED, checks sensors
        ├── NIGHT_DELAY    : Buzzer on for 2s, then move to OPEN
        ├── OPEN           : Servo 90°, Green LED, holds for 3s, then closes
        └── BLOCKED_ALM    : Buzzer on until path is cleared
```

---

## 💻 Source Code

```cpp
#include <Servo.h>

// Pin Definitions
const int trigPin = 4;
const int echoPin = 5;
const int pirPin = 3;
const int ldrPin = A0;
const int buzzerPin = 8;
const int greenLed = 6;
const int redLed = 7;
const int servoPin = 9;

Servo gateServo;

// Non-blocking Timers & Tracking Variables
unsigned long vehicleArrivalTime = 0;
unsigned long gateTimer = 0;
bool vehiclePresent = false;

// [INNOVATION ENHANCEMENT]: Software-defined Asset Tracker Feature
int totalVehiclesPassed = 0;
bool countIncremented = false;

// System States Configuration
enum GateState { CLOSED, NIGHT_DELAY, OPEN, OVERSTAY_ALARM };
GateState currentGateState = CLOSED;

void setup() {
  // Initialize Input/Output Node Directives
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pirPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);

  // Enforce baseline servo orientation before attachment to prevent startup jerk
  gateServo.write(0);
  gateServo.attach(servoPin);

  // Set initial safe system indicators (Closed = Red LED)
  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, LOW);

  Serial.begin(9600);
  Serial.println("=============================================");
  Serial.println("   SMART PARKING GATE SYSTEM ONLINE");
  Serial.println("   DESIGN ARCHITECTURE: NON-BLOCKING FSM");
  Serial.println("=============================================");
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Fetch Real-Time Sensor Data
  int distance = readDistance();
  int motion = digitalRead(pirPin);
  int ldrValue = analogRead(ldrPin);

  // Day/Night Threshold Evaluation
  // Tinkercad LDRs read low values in the dark.
  // Bright range yields 1021 (DAY) | Dark range drops to 563 (NIGHT)
  bool isDay = (ldrValue > 750);

  // 2. Continuous Proximity Validation Filter
  if (distance > 0 && distance <= 20) {
    if (!vehiclePresent) {
      vehicleArrivalTime = currentMillis; // Log secure arrival reference timestamp
      vehiclePresent = true;
    }
  } else {
    vehiclePresent = false;
    vehicleArrivalTime = 0; // Reset variables instantly when targets back clear
  }

  // 3. Serial Dashboard Telemetry Output (Updates precisely every 500ms)
  static unsigned long lastLog = 0;
  if (currentMillis - lastLog > 500) {
    Serial.print("Dist: "); Serial.print(distance); Serial.print("cm");
    Serial.print(" | PIR: "); Serial.print(motion);
    Serial.print(" | LDR: "); Serial.print(ldrValue);
    Serial.print(isDay ? " (DAY)" : " (NIGHT)");
    Serial.print(" | System State: ");
    switch (currentGateState) {
      case CLOSED: Serial.print("CLOSED"); break;
      case NIGHT_DELAY: Serial.print("NIGHT_BEEP_DELAY (2s)"); break;
      case OPEN: Serial.print("GATE_OPEN (3s)"); break;
      case OVERSTAY_ALARM: Serial.print("OVERSTAY_ALARM (CRITICAL)"); break;
    }
    Serial.print(" | Total Vehicle Count: "); Serial.println(totalVehiclesPassed);
    lastLog = currentMillis;
  }

  // 4. Finite State Machine Evaluation Logic Matrix
  switch (currentGateState) {

    case CLOSED:
      gateServo.write(0);           // Physical lock down at 0 degrees
      digitalWrite(redLed, HIGH);   // Enforce Red LED ON
      digitalWrite(greenLed, LOW);  // Enforce Green LED OFF
      noTone(buzzerPin);            // Mute audio channels
      countIncremented = false;     // Reset tracking flag

      if (vehiclePresent) {
        // Condition: Enforce continuous perimeter blocking overstay rule (> 5s)
        if (currentMillis - vehicleArrivalTime > 5000) {
          currentGateState = OVERSTAY_ALARM;
        }
        // Condition: Daytime entry criteria validation (Requires active PIR loop)
        else if (isDay && motion == HIGH) {
          gateTimer = currentMillis;
          currentGateState = OPEN;
        }
        // Condition: Nighttime entry criteria validation (Triggers immediate acoustic warning)
        else if (!isDay) {
          gateTimer = currentMillis;
          currentGateState = NIGHT_DELAY;
        }
      }
      break;

    case NIGHT_DELAY:
      // Condition: Pulse acoustic beacon warning for exactly 2 seconds prior to barrier lift
      if (currentMillis - gateTimer < 2000) {
        tone(buzzerPin, 440); // Sound 440Hz standard clear pulse tone
      } else {
        noTone(buzzerPin);
        gateTimer = currentMillis;      // Shift internal time frame to Open Duration
        currentGateState = OPEN;        // Execute entry transition
      }

      // Safety Intercept: Clear state tracking instantly if target rolls back away mid-beep
      if (!vehiclePresent) {
        currentGateState = CLOSED;
      }
      break;

    case OPEN:
      gateServo.write(90);          // Drive barrier mechanism clear (90 Degrees)
      digitalWrite(redLed, LOW);    // Red LED OFF
      digitalWrite(greenLed, HIGH); // Enforce Green LED ON
      noTone(buzzerPin);            // Protect buzzer from hanging signal errors

      // [INNOVATION LOGIC]: Registers vehicle transit entry once safely inside
      if (!countIncremented) {
        totalVehiclesPassed++;
        countIncremented = true;
      }

      // Condition: Hold barrier frame open for exactly 3 seconds, then drop automatically
      if (currentMillis - gateTimer >= 3000) {
        // Enforce safety scan prior to dropping gate line
        if (vehiclePresent && (currentMillis - vehicleArrivalTime > 5000)) {
          currentGateState = OVERSTAY_ALARM;
        } else {
          currentGateState = CLOSED;
        }
      }
      break;

    case OVERSTAY_ALARM:
      tone(buzzerPin, 880);         // Condition: Sound high-frequency distress tone continuously
      gateServo.write(90);          // Keep barrier clear at 90 Degrees for vehicle protection
      digitalWrite(greenLed, LOW);
      digitalWrite(redLed, HIGH);   // Assert emergency light visual indicators

      // Exit condition: Release alert loops immediately once physical parameters clear
      if (!vehiclePresent) {
        noTone(buzzerPin);
        currentGateState = CLOSED;
      }
      break;
  }
}

// Low-overhead Ultrasonic Driver Engine Block
int readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Strict 25ms timeout cutoff avoids simulation deadlocks and lag spikes
  long duration = pulseIn(echoPin, HIGH, 25000);
  if (duration == 0) return 999;

  return duration * 0.034 / 2;
}
```

---

## 🚦 State Summary

| State            | Servo | LEDs              | Buzzer            | Behavior                                         |
| ---------------- | ----- | ----------------- | ----------------- | ------------------------------------------------ |
| `CLOSED`         | 0°    | Red ON, Green OFF | Off               | Default; checks sensors for entry conditions     |
| `NIGHT_DELAY`    | 0°    | Red ON            | 440 Hz for 2s     | Pre-entry warning at night before gate opens     |
| `OPEN`           | 90°   | Green ON, Red OFF | Off               | Gate open for 3s; vehicle count incremented      |
| `OVERSTAY_ALARM` | 90°   | Red ON            | 880 Hz continuous | Critical alert if a vehicle blocks the gate > 5s |

---

**Author:** Sreya Vinod
