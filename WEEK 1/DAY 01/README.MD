# 🚦 Traffic Signalling System

**Platform:** Tinkercad Simulation · **Microcontroller:** Arduino Uno R3

🔗 **Tinkercad Simulation:** [View Project](https://www.tinkercad.com/things/jaaxHdgUusS-traffic-signallingsystem?sharecode=EFUD-unyDzBhu7DM3X_kc_BFVnrhZys1_wt5-tBK1RQ)

---

## 📌 Overview

A simple Arduino-based simulation of a traffic light system, cycling through Red, Yellow, and Green signals with realistic timing — Red and Green held for 3 seconds each, Yellow for 1 second — repeating continuously.

---

## 🧩 Components Used

| Component                    | Purpose                                                                              |
| ---------------------------- | ------------------------------------------------------------------------------------ |
| Arduino Uno R3               | The microcontroller that acts as the brain, executing the code and controlling power |
| 3x LEDs (Red, Yellow, Green) | Visual light indicators that mimic the actual traffic signal phases                  |
| 3x 220Ω Resistors            | Safety devices placed in series with the LEDs to limit current and prevent burnout   |
| Breadboard & Jumper Wires    | Prototyping tools used to establish connections without soldering                    |

---

## 💻 Source Code

```cpp
// LED pin connections
const int redPin = 12;
const int yellowPin = 11;
const int greenPin = 10;

void setup() {
  // Initialize digital pins as outputs
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
}

void loop() {
  // 1. Red signal active for 3 seconds
  digitalWrite(redPin, HIGH);
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, LOW);
  delay(3000);

  // 2. Yellow signal active for 1 second
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, HIGH);
  digitalWrite(greenPin, LOW);
  delay(1000);

  // 3. Green signal active for 3 seconds
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, HIGH);
  delay(3000);
}
```

---

## 🔍 Code Explanation

- **Pin Definitions** — The code assigns easy-to-remember names (`redPin`, `yellowPin`, `greenPin`) to Arduino digital pins 12, 11, and 10 so they can be controlled individually.
- **The `setup()` Function** — Runs once at startup to configure pins 12, 11, and 10 as `OUTPUT` channels, allowing the board to send voltage out to the LEDs.
- **The `loop()` Function** — Runs infinitely, executing a top-to-bottom sequence that switches power between the pins while keeping the other lights turned off.
- **The `digitalWrite()` Commands** — Control the electricity: `HIGH` delivers 5V to light up an LED, `LOW` cuts the power to turn it off.
- **The `delay()` Commands** — Pause the program in its current state, holding the Red light for 3 seconds, the Yellow light for 1 second, and the Green light for 3 seconds before restarting the cycle.

---

## 🔄 Signal Cycle

```
RED (3s) → YELLOW (1s) → GREEN (3s) → repeat
```

---

**Author:** Sreya Vinod
