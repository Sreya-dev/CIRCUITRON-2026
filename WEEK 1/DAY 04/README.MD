# 🌡️ Automated Smart Greenhouse System

**Platform:** Tinkercad Simulation · **Microcontroller:** Arduino Uno R3

🔗 **Tinkercad Simulation:** [View Project](https://www.tinkercad.com/things/0es096CtZNc-smartgreenhouse-automationsystem/editel?sharecode=pp0GokY3MtzBMqUmNrRWaXyO5Mz85oPPL6-ebtSmpeg)

---

## 📌 Project Aim

To design, implement, and simulate an automated Smart Greenhouse Automation System using an Arduino Uno microcontroller. The system is engineered to autonomously regulate internal environmental factors — specifically microclimate temperature and ambient light intensity — to optimize crop growth conditions, prevent thermal stress, conserve energy, and provide visual/audible safety alerts during critical system thresholds.

---

## 📖 Project Overview

The Automated Smart Greenhouse System acts as a closed-loop environmental control unit. By continuously polling data from an analog ambient light sensor (LDR) and a linear temperature sensor (TMP36), the microcontroller makes real-time operational adjustments without human intervention.

### Core System Logic

- **Microclimate Lighting Control** — Automatically illuminates a high-efficiency White LED when ambient sunlight drops below acceptable thresholds.
- **Proportional Thermal Regulation** — Employs Pulse Width Modulation (PWM) to scale the cooling fan speed dynamically while coordinating physical ventilation window adjustments via a Servo motor across staggered thermal boundaries (30°C, 35°C, and 40°C).
- **Critical Overheat Containment** — Triggers a synchronous emergency protocol if internal temperatures breach a dangerous ceiling (>40°C). This activates an audible buzzer hazard array, a flashing strobe alert, and places the cooling mechanics into maximum fallback mode.
- **System Status Telemetry** — A dedicated visual health indicator (Green LED) monitors nominal operations, turning off instantly during critical system exceptions to signal a breach of regular bounds.

---

## 🧩 Components Used

| Component Name      | Quantity | Function in System                                           |
| ------------------- | -------- | ------------------------------------------------------------ |
| Arduino Uno R3      | 1        | Central Processing Unit / Microcontroller                    |
| TMP36 Sensor        | 1        | Precision Low-Voltage Temperature Sensor                     |
| Photoresistor (LDR) | 1        | Ambient Light Intensity Sensor                               |
| Servo Motor         | 1        | Automated Vent / Ventilation Window Actuator                 |
| DC Motor (Fan)      | 1        | Forced Convection Airflow and Cooling Mechanism              |
| N-Channel MOSFET    | 1        | High-Current Driver for PWM Fan Speed Control                |
| White LED           | 1        | Auxiliary Crop Growth Photoperiod Light Source               |
| Green LED           | 1        | System Status Indicator (Normal Operation)                   |
| Red LED             | 1        | Critical Temperature Hazard/Alert Light                      |
| Piezo Buzzer        | 1        | Audible Critical Threshold Overheat Alarm                    |
| Resistors (220Ω)    | 4        | Current-Limiting Resistors for LEDs & MOSFET Gate Protection |
| Resistor (10kΩ)     | 1        | Pull-Down Resistor for LDR Voltage Divider Network           |
| External 12V Power  | 1        | Separate Power Supply for High-Draw DC Fan Motor             |
| Breadboard & Wires  | 1        | Circuit Prototyping Platform & Interconnects                 |

---

## 🔌 Construction and Wiring Details

### 1. Sensor Array Network

- **TMP36 Sensor** — Left pin to Arduino 5V, middle pin to Analog Input A0, right pin to common GND.
- **LDR Network** — Arranged as a voltage divider. One leg connects directly to 5V. The opposing leg ties into Analog Input A1 and bridges to ground through a 10kΩ pull-down resistor.

### 2. Actuator Control Network

- **Servo Motor** — Command signal (Orange/White) to Digital PWM Pin 9, Power (Red) to 5V, Ground (Brown/Black) to common GND.
- **DC Fan Drive (MOSFET)** — Arduino Digital PWM Pin 3 through a 220Ω current-limiting gate resistor to the Gate pin of the MOSFET. Source pin wired directly to common GND. DC Motor attached across the Drain pin and the positive terminal of the external 12V DC power source.

### 3. Telemetry & Signalling Array

- **White LED** — Anode to Digital Pin 4 via a 220Ω inline resistor; Cathode to GND.
- **Green LED** — Anode to Digital Pin 5 via a 220Ω inline resistor; Cathode to GND.
- **Red LED** — Anode to Digital Pin 6 via a 220Ω inline resistor; Cathode to GND.
- **Piezo Buzzer** — Positive terminal to Digital Pin 7; Negative terminal to GND.

> **Note:** The Ground (GND) of the external 12V power supply and the Ground of the Arduino Uno must be tied together to establish a unified reference plane across the control gates.

---

## 🔄 Code Structure and Program Flow

The Arduino firmware operates via a non-blocking synchronous polling loop architecture. It avoids structural hardware delays (`delay()`) during critical alerts, allowing the microcontroller to calculate timing windows concurrently.

```
START: Power On / System Initialisation
  → Configure I/O Pins & Attach Servo
  → LOOP START
      → Read Sensor Analog Inputs
          - Read Voltage from A0 (TMP36)
          - Read Voltage from A1 (LDR)
      → Convert Voltages to Environmental Units
          - Compute Temp in Celsius
          - Determine Light Status (Dark/Bright)
      → Evaluate Light Condition
          - Light Low?  → YES → Turn ON White LED (or blink if Overheat)
          - Light High? → NO  → Turn OFF White LED
      → Evaluate Temperature State Machine
          ├── Case 1: Temp < 30°C
          │     Fan OFF (0 PWM) | Window Closed (0°) | Buzzer/Red LED OFF | Green LED ON
          ├── Case 2: 30°C ≤ Temp ≤ 35°C
          │     Fan Mid-Speed (128 PWM) | Window Half-Open (45°) | Buzzer/Red LED OFF | Green LED ON
          ├── Case 3: 35°C < Temp ≤ 40°C
          │     Fan Scaled-Speed (Proportional PWM) | Window Open (90°) | Buzzer/Red LED OFF | Green LED ON
          └── Case 4: Temp > 40°C [CRITICAL TRIPPED]
                Fan Max-Speed (255 PWM) | Window Open (90°) | Buzzer/Red LED ON
                Green LED OFF | White LED Emergency Flashing Loop Mode
      → Acknowledge Hardware Refresh Step
  ← Repeat
```

---

## 💻 Source Code

```cpp
#include <Servo.h>

// ==========================================
// PIN ALLOCATIONS & ASSIGNMENTS
// ==========================================
const int tempPin      = A0; // Analog input pin for temperature sensor (TMP36)
const int ldrPin       = A1; // Analog input pin for photoresistor (LDR)
const int servoPin     = 9;  // Digital PWM pin for ventilation window servo
const int fanPin       = 3;  // Digital PWM pin for DC cooling fan motor drive
const int whiteLedPin  = 4;  // Digital pin output for White LED light source
const int greenLedPin  = 5;  // Digital pin output for Green LED state indicator
const int redLedPin    = 6;  // Digital pin output for Red LED emergency light
const int buzzerPin    = 7;  // Digital pin output for audible alarm piezo buzzer

// Instantiation of the Servo Control Instance
Servo ventServo;

// ==========================================
// SYSTEM VARIABLE DEFINITIONS
// ==========================================
int ldrValue = 0;              // Variable to hold raw 10-bit light sensor data
float tempC = 0.0;             // Variable to store converted Celsius temperature
unsigned long prevMs = 0;      // Stores execution epoch for non-blocking timer
const long blinkRate = 250;    // Strobe frequency (ms) for the emergency light
bool emergencyStrobe = LOW;    // State toggle variable for flashing functions

void setup() {
  // Initialize device directional parameters
  pinMode(fanPin, OUTPUT);
  pinMode(whiteLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  // Attach servo to designated control pin and force start state to closed
  ventServo.attach(servoPin);
  ventServo.write(0);

  // Open Serial interface channel for calibration diagnostics
  Serial.begin(9600);
}

void loop() {
  // ==========================================
  // STEP 1: SENSOR TELEMETRY ACQUISITION
  // ==========================================

  // Fetch raw analog readings and map mathematical values to Celsius
  int tempReading = analogRead(tempPin);
  float voltage = tempReading * 5.0;
  voltage /= 1024.0;                  // Quantize voltage against 10-bit dynamic scale
  tempC = (voltage - 0.5) * 100.0;    // Shift by offset and apply step scaling factor

  // Read atmospheric light value from LDR
  ldrValue = analogRead(ldrPin);

  // Define boundary conditions for dark cycles (Tinkercad environmental value < 300)
  bool isDark = (ldrValue < 300);

  // ==========================================
  // STEP 2: LUMINANCE SUBSYSTEM CONTROL
  // ==========================================
  if (isDark) {
    if (tempC > 40.0) {
      // Innovation / Enhancement feature: Run non-blocking flash timing window
      unsigned long currentMs = millis();
      if (currentMs - prevMs >= blinkRate) {
        prevMs = currentMs;
        emergencyStrobe = !emergencyStrobe;
      }
      digitalWrite(whiteLedPin, emergencyStrobe);
    } else {
      digitalWrite(whiteLedPin, HIGH); // Steady low-light compensation state
    }
  } else {
    digitalWrite(whiteLedPin, LOW);    // Energy saving disable window
  }

  // ==========================================
  // STEP 3: THERMAL REGULATION STATE MACHINE
  // ==========================================

  // STATE A: NOMINAL COOLING STATE (Below 30°C)
  if (tempC < 30.0) {
    analogWrite(fanPin, 0);          // Spin down active convection fan
    ventServo.write(0);              // Return mechanical structural sash to 0 degrees
    digitalWrite(buzzerPin, LOW);    // Silence alert buzzer
    digitalWrite(redLedPin, LOW);    // Silence emergency visual light
    digitalWrite(greenLedPin, HIGH); // Assert system functional health index
  }

  // STATE B: LOW THRESHOLD COOLING (30°C to 35°C)
  else if (tempC >= 30.0 && tempC <= 35.0) {
    analogWrite(fanPin, 128);        // Drive cooling fan at 50% PWM performance load
    ventServo.write(45);             // Command ventilation sash to 45 degree vector
    digitalWrite(buzzerPin, LOW);
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, HIGH);
  }

  // STATE C: MID THRESHOLD COOLING (35°C to 40°C)
  else if (tempC > 35.0 && tempC <= 40.0) {
    // ENHANCEMENT FEATURE: Dynamic scaling mapping equation
    int dynamicFanSpeed = map(tempC, 35.1, 40.0, 128, 255);
    analogWrite(fanPin, dynamicFanSpeed); // Dynamically scale cooling performance
    ventServo.write(90);             // Command ventilation sash to full 90 degree vector
    digitalWrite(buzzerPin, LOW);
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, HIGH);
  }

  // STATE D: CRITICAL ENVIRONMENTAL HAZARD OVERHEAT (Above 40°C)
  else if (tempC > 40.0) {
    analogWrite(fanPin, 255);        // Drive cooling fan at 100% duty cycle limit
    ventServo.write(90);             // Keep ventilation window wide open
    digitalWrite(buzzerPin, HIGH);   // Raise audible alarm tone
    digitalWrite(redLedPin, HIGH);   // Strike visual danger indicator light
    digitalWrite(greenLedPin, LOW);  // Disengage system operational health indicator
  }

  // Brief loop settling step to promote overall system convergence stability
  delay(50);
}
```

---

## 🌡️ Thermal State Summary

| State                 | Temp Range  | Fan (PWM)        | Window (Servo)  | Buzzer / Red LED | Green LED |
| --------------------- | ----------- | ---------------- | --------------- | ---------------- | --------- |
| A — Nominal           | < 30°C      | OFF (0)          | Closed (0°)     | OFF              | ON        |
| B — Low Threshold     | 30°C – 35°C | Mid (128)        | Half-Open (45°) | OFF              | ON        |
| C — Mid Threshold     | 35°C – 40°C | Scaled (128–255) | Open (90°)      | OFF              | ON        |
| D — Critical Overheat | > 40°C      | Max (255)        | Open (90°)      | ON               | OFF       |

---

**Author:** Sreya Vinod
