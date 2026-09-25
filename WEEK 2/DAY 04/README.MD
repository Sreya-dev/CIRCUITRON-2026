# 🔐 Smart Door Lock System using ESP32, Servo Motor & Blynk IoT

**Platform:** Wokwi Simulation · **Microcontroller:** ESP32 Dev Board · **Cloud:** Blynk IoT

🔗 **Wokwi Simulation:** [https://wokwi.com/projects/466507328925840385](https://wokwi.com/projects/466507328925840385)

---

## 📌 Project Aim

To design and implement a Smart Door Lock System using an ESP32 microcontroller, a servo motor, and the Blynk IoT platform that allows users to remotely lock and unlock a door in real time via a mobile/web dashboard over Wi-Fi.

---

## 📖 Project Overview

The Smart Door Lock System is an IoT-based security solution built on the ESP32 microcontroller. The system connects to the Blynk Cloud over Wi-Fi. A Button Widget on the Blynk dashboard controls a servo motor: pressing the button rotates the servo to 90° (unlocked), and releasing it returns the servo to 0° (locked). The current door state ("Door Locked" / "Door Unlocked") is reflected on a Label Widget in real time.

The project was simulated using the Wokwi online simulator. It demonstrates practical IoT integration, remote actuation, and real-time status feedback — core pillars of smart home automation.

---

## 🧩 Components Used

| Component          | Specification                 | Quantity |
| ------------------ | ----------------------------- | -------- |
| ESP32 Dev Board    | 38-pin, dual-core, Wi-Fi + BT | 1        |
| Servo Motor        | SG90 / compatible (0–180°)    | 1        |
| Jumper Wires       | Male-to-Male                  | 3        |
| Power Supply       | USB 5V via ESP32              | 1        |
| Blynk IoT Platform | Cloud dashboard (blynk.cloud) | —        |
| Wokwi Simulator    | Online ESP32 simulation       | —        |

---

## 🔌 Construction and Wiring Details

| Servo Wire | Color           | ESP32 Pin |
| ---------- | --------------- | --------- |
| Signal     | Orange / Yellow | GPIO 18   |
| VCC        | Red             | 5V (VBUS) |
| GND        | Brown / Black   | GND       |

### Wiring Notes

- The servo signal wire connects directly to GPIO 18 on the ESP32.
- Power is drawn from the ESP32's 5V VBUS pin (adequate for simulation; for physical builds use a dedicated 5V supply for the servo).
- No additional resistors or capacitors are needed for simulation.
- In Wokwi: drag ESP32 + Servo Motor components onto the canvas and draw wires per the table above.

---

## 🔄 Code Structure / Program Flow

**File:** `src/main.cpp`

| Section              | Description                                          |
| -------------------- | ---------------------------------------------------- |
| `#define` block      | Blynk Template ID, Template Name, Auth Token         |
| `#include` block     | WiFi, BlynkSimpleEsp32, ESP32Servo libraries         |
| `setDoorState(bool)` | Core function — moves servo, updates V1 label        |
| `BLYNK_WRITE(V0)`    | Callback triggered by the Blynk button widget        |
| `BLYNK_CONNECTED()`  | Syncs V0 state on reconnect to prevent stale UI      |
| `setup()`            | Servo attach, initial lock position, `Blynk.begin()` |
| `loop()`             | Runs `Blynk.run()` — handles all Blynk communication |

### Virtual Pins

| Pin | Direction        | Widget               | Purpose                     |
| --- | ---------------- | -------------------- | --------------------------- |
| V0  | Input (from app) | Button (Switch mode) | Send 1 = unlock, 0 = lock   |
| V1  | Output (to app)  | Label                | Display current door status |

---

## 💻 Source Code

```cpp
#define BLYNK_TEMPLATE_ID   "TMPL3qbnnkctp"
#define BLYNK_TEMPLATE_NAME "Smart Door Lock"
#define BLYNK_AUTH_TOKEN    "0JRDFU6A2MQWxUKCYhffgoJT7hcqCbNV"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// ── Wi-Fi credentials
const char* ssid     = "Wokwi-GUEST"; // Wokwi's built-in Wi-Fi
const char* password = "";            // No password for Wokwi-GUEST

// ── Hardware
#define SERVO_PIN 18 // GPIO18 → Servo signal wire

Servo doorServo;

// ── Blynk Virtual Pins
// V0 → Button Widget (Switch mode, 0/1)
// V1 → Label Widget  (shows lock status)

// ── Door state
bool isDoorUnlocked = false;

// Helper: move servo + push status to Blynk dashboard
void setDoorState(bool unlock) {
  isDoorUnlocked = unlock;

  if (unlock) {
    doorServo.write(90); // Unlock position
    Blynk.virtualWrite(V1, "Door Unlocked");
    Serial.println("[DOOR] Unlocked -> Servo @ 90 degrees");
  } else {
    doorServo.write(0); // Lock position
    Blynk.virtualWrite(V1, "Door Locked");
    Serial.println("[DOOR] Locked   -> Servo @ 0 degrees");
  }
}

// Blynk: V0 Button handler
//    value == 1 → button pressed  (unlock)
//    value == 0 → button released (lock)
BLYNK_WRITE(V0) {
  int buttonState = param.asInt();
  setDoorState(buttonState == 1);
}

// Blynk: sync state when app connects / reconnects
BLYNK_CONNECTED() {
  Serial.println("[Blynk] Connected - syncing V0");
  Blynk.syncVirtual(V0);
}

// setup
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Smart Door Lock System ===");

  // Attach servo
  doorServo.attach(SERVO_PIN, 500, 2400); // min/max pulse us
  doorServo.write(0);                     // Start locked
  Serial.println("[SERVO] Initialised at 0 degrees (Locked)");

  // Connect to Blynk (handles Wi-Fi internally)
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  Serial.println("[WiFi] Connecting via Blynk.begin ...");
}

// loop
void loop() {
  Blynk.run();
}
```

---

## ✨ Additional Features

1. **`BLYNK_CONNECTED()` Reconnect Sync** — The system automatically re-syncs the button state whenever the app reconnects, preventing a mismatch between the servo position and the dashboard display.
2. **Servo Pulse Calibration** — `doorServo.attach(SERVO_PIN, 500, 2400)` sets precise pulse widths for accurate 0°/90° movement across different servo models.
3. **Serial Monitor Logging** — All state changes are printed to the Serial Monitor with labels, enabling easy debugging and demonstration.
4. **Blynk External REST API** — The system is compatible with Blynk's HTTP API, allowing the door to be controlled or monitored from any browser or external script without the Blynk app.

---

## 🌐 Blynk Cloud API Links

| Purpose                    | URL                                                                                             |
| -------------------------- | ----------------------------------------------------------------------------------------------- |
| Read V0 (button state)     | `https://blynk.cloud/external/api/get?token=0JRDFU6A2MQWxUKCYhffgoJT7hcqCbNV&pin=V0`            |
| Read V1 (door status text) | `https://blynk.cloud/external/api/get?token=0JRDFU6A2MQWxUKCYhffgoJT7hcqCbNV&pin=V1`            |
| Write V0 → Unlock (1)      | `https://blynk.cloud/external/api/update?token=0JRDFU6A2MQWxUKCYhffgoJT7hcqCbNV&pin=V0&value=1` |
| Write V0 → Lock (0)        | `https://blynk.cloud/external/api/update?token=0JRDFU6A2MQWxUKCYhffgoJT7hcqCbNV&pin=V0&value=0` |

---

**Author:** Sreya Vinod
