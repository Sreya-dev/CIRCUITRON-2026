# 💡 IoT-Based Room Automation System

**Platform:** Wokwi Simulation (with local VS Code option) · **Microcontroller:** ESP32 DevKit V1 · **Cloud:** Blynk IoT

🔗 **Wokwi Simulation:** [https://wokwi.com/projects/466342877042915329](https://wokwi.com/projects/466342877042915329)

> If the Wokwi website simulation isn't working, download the shared "Room Automation" folder and open it in VS Code (see [VS Code Setup](#-vs-code-setup) below).

### Blynk HTTP API Endpoints

| Device     | Action | Endpoint                                                                              |
| ---------- | ------ | ------------------------------------------------------------------------------------- |
| Light (V0) | ON     | `https://blynk.cloud/external/api/update?token=bQBunqCuUnUqX--c3AZCfXrHFOqp6DzS&v0=1` |
| Light (V0) | OFF    | `https://blynk.cloud/external/api/update?token=bQBunqCuUnUqX--c3AZCfXrHFOqp6DzS&v0=0` |
| Fan (V1)   | ON     | `https://blynk.cloud/external/api/update?token=bQBunqCuUnUqX--c3AZCfXrHFOqp6DzS&v1=1` |
| Fan (V1)   | OFF    | `https://blynk.cloud/external/api/update?token=bQBunqCuUnUqX--c3AZCfXrHFOqp6DzS&v1=0` |

---

## 📌 Project Aim

To design and implement an IoT-based Room Automation System using an ESP32 microcontroller and Blynk Cloud platform, simulated in Wokwi, that allows a user to remotely and independently control a room light (LED) and a room fan (stepper motor) in real time through the Blynk mobile application over Wi-Fi.

---

## 📖 Project Overview

This project demonstrates a smart home automation scenario where two appliances — a room light and a room fan — are controlled wirelessly from a smartphone. The ESP32 connects to the internet via Wi-Fi and communicates with the Blynk Cloud server. When a user toggles a button on the Blynk app, the cloud sends a command to the ESP32, which immediately acts on it. The light and fan operate completely independently: toggling one has no effect on the other.

The entire system is simulated inside Wokwi, a browser-based electronics simulator, using:

- A virtual LED for the light
- A stepper motor + A4988 driver for the fan
- The Wokwi-GUEST Wi-Fi hotspot for network connectivity

**Additional Enhancement (beyond requirements):** Instead of a simple DC motor, this project uses a stepper motor driven by an A4988 motor driver chip, demonstrating microstepping control and direction management — a more realistic and technically advanced fan implementation.

---

## 🧩 Components Used

| Component            | Quantity | Purpose                            |
| -------------------- | -------- | ---------------------------------- |
| ESP32 DevKit V1      | 1        | Main microcontroller, Wi-Fi, logic |
| Blue LED             | 1        | Represents room light              |
| 220Ω Resistor        | 1        | Current limiter for LED            |
| Stepper Motor        | 1        | Represents room fan                |
| A4988 Stepper Driver | 1        | Controls stepper motor             |

### Virtual / Cloud Components

- Blynk Cloud (`BLYNK_TEMPLATE_ID`: `TMPL3k3svVwwW`)
- Blynk Mobile App — Button widget on V0 (Light), Button widget on V1 (Fan)
- Wokwi Simulator (wokwi.com) — Simulation environment

---

## 🔌 Construction and Wiring Details

### LED Circuit (Room Light)

- ESP32 Pin D23 → 220Ω Resistor → LED Anode (A)
- LED Cathode (K) → ESP32 GND
- When D23 goes HIGH, current flows through the resistor and the LED lights up.

### Fan Circuit (Room Fan via Stepper Motor)

| ESP32 Pin | A4988 Pin | Purpose                               |
| --------- | --------- | ------------------------------------- |
| D14       | STEP      | Each HIGH pulse = one motor step      |
| D12       | DIR       | Sets rotation direction               |
| D27       | EN        | LOW = driver enabled, HIGH = disabled |
| GND       | GND       | Common ground                         |
| 3V3       | VDD       | Logic power for A4988                 |
| VIN (5V)  | VMOT      | Motor power supply                    |

- A4988 RESET is bridged to SLEEP to keep the driver active.
- Motor coil wires: 1A/1B → Phase A+/A−; 2A/2B → Phase B+/B−

### Serial Monitor

- TX0 → Serial Monitor RX (for debug output)
- RX0 → Serial Monitor TX

---

## 🔄 Code Structure / Program Flow

1. **Definitions & Libraries** — `BLYNK_TEMPLATE_ID`, `BLYNK_TEMPLATE_NAME`, `BLYNK_AUTH_TOKEN`, `<WiFi.h>`, `<BlynkSimpleEsp32.h>` are defined at the top before any code runs.
2. **Pin & Variable Definitions** — `LIGHT_PIN 23` (LED output pin); `STEP_PIN 14`, `DIR_PIN 12`, `EN_PIN 27` (stepper motor control pins); `fanRunning` (global flag tracking fan state); `stepDelayUs 1500` (speed control for motor stepping).
3. **`stepMotor()` Function** — Sends a single STEP pulse to the A4988 driver. Called continuously in `loop()` when `fanRunning == 1`, producing smooth continuous rotation.
4. **`BLYNK_WRITE(V0)` — Light Handler** — Triggered whenever the user presses the light button in the Blynk app. Reads the integer value (0 or 1) and writes it directly to `LIGHT_PIN`.
5. **`BLYNK_WRITE(V1)` — Fan Handler** — Triggered when the user presses the fan button. If `fanState == 1`: enables the driver (EN LOW), sets direction (DIR HIGH), sets `fanRunning = 1`. If `fanState == 0`: disables driver (EN HIGH), sets `fanRunning = 0`.
6. **`setup()`**
   - Starts serial at 115200 baud
   - Sets all pins to OUTPUT mode
   - Sets safe initial states (light OFF, motor disabled)
   - Starts Wi-Fi with `WiFi.begin()`
   - Configures Blynk with `Blynk.config()` and `Blynk.connect()` (asynchronous, avoids freezing)
7. **`loop()`**
   - Calls `Blynk.run()` only when Wi-Fi is confirmed connected — prevents cloud errors if connection drops
   - If `fanRunning == 1`, calls `stepMotor()` every loop iteration to keep the motor spinning

---

## 💻 Source Code

```cpp
#define BLYNK_TEMPLATE_ID    "TMPL3k3svVwwW"
#define BLYNK_TEMPLATE_NAME  "LilyGo T PCIe 2"
#define BLYNK_AUTH_TOKEN     "bQBunqCuUnUqX--c3AZCfXrHFOqp6DzS"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// WiFi credentials for Wokwi simulation
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Pin assignments
#define LIGHT_PIN 23  // LED for room light
#define STEP_PIN  14  // A4988 STEP signal
#define DIR_PIN   12  // A4988 DIR signal
#define EN_PIN    27  // A4988 EN signal (LOW = enabled)

int fanRunning = 0;      // Fan state flag
int stepDelayUs = 1500;  // Step delay in microseconds (controls speed)

// Sends one step pulse to the A4988 stepper driver
void stepMotor() {
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(10);            // Minimum pulse width
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(stepDelayUs);   // Controls motor speed
}

// V0 handler — controls room light
BLYNK_WRITE(V0) {
  int lightState = param.asInt();
  digitalWrite(LIGHT_PIN, lightState); // 1 = ON, 0 = OFF
  Serial.println(lightState ? "Room Light: ON" : "Room Light: OFF");
}

// V1 handler — controls room fan
BLYNK_WRITE(V1) {
  int fanState = param.asInt();
  if (fanState == 1) {
    fanRunning = 1;
    digitalWrite(EN_PIN, LOW);   // Enable the A4988 driver
    digitalWrite(DIR_PIN, HIGH); // Set rotation direction
    Serial.println("Room Fan: ON");
  } else {
    fanRunning = 0;
    digitalWrite(EN_PIN, HIGH);  // Disable driver (motor holds position)
    Serial.println("Room Fan: OFF");
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n--- [START] Serial Monitor Active ---");
  Serial.println("Initializing pins...");

  // Set all control pins as output
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);

  // Safe initial states: light OFF, motor disabled
  digitalWrite(LIGHT_PIN, LOW);
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, HIGH);
  digitalWrite(EN_PIN, HIGH); // HIGH = driver disabled

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, pass);

  // Async Blynk config prevents setup() from freezing
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();
  Serial.println("Blynk configuration loaded. Running loop...");
}

void loop() {
  // Only process Blynk if WiFi is active
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
  }

  // Continuously step motor if fan is running
  if (fanRunning == 1) {
    stepMotor();
  }
}
```

---

## ✨ Additional Features

| Enhancement                   | Description                                                                                                                            |
| ----------------------------- | -------------------------------------------------------------------------------------------------------------------------------------- |
| Stepper Motor + A4988 driver  | More realistic and technically advanced than a basic DC motor; demonstrates microstepping and enable/disable control                   |
| Asynchronous Blynk connection | `Blynk.config()` + `Blynk.connect()` used instead of blocking `Blynk.begin()`, preventing the ESP32 from freezing if the cloud is slow |
| WiFi status guard in loop     | `WiFi.status() == WL_CONNECTED` check ensures no crash if connection drops during runtime                                              |
| Serial Monitor logging        | Real-time debug messages confirm every state change — useful for demonstration                                                         |
| Motor speed control           | `stepDelayUs` variable allows fan speed to be easily adjusted                                                                          |

---

## 📱 Blynk Mobile App Setup

To replicate this project on the Blynk app:

1. Create a new template with the Template ID and name shown in the code.
2. Add a Button widget → set to Virtual Pin **V0** → Mode: Switch → Label: "Room Light"
3. Add a Button widget → set to Virtual Pin **V1** → Mode: Switch → Label: "Room Fan"
4. Copy the Auth Token into the `BLYNK_AUTH_TOKEN` define.
5. Run the Wokwi simulation — both buttons will control devices in real time.

---

## 🖥️ VS Code Setup

To get your Wokwi Simulator workspace set up locally in VS Code, install the extension, activate your free license, and configure your local hardware and firmware paths.

### Prerequisites

Ensure you have a toolchain installed to compile your microcontroller code. The most common and easiest option is the **PlatformIO IDE Extension** for VS Code. Ensure PlatformIO is installed and fully functional.

### Step 1: Install the Wokwi Extension

1. Open Visual Studio Code.
2. Click on the Extensions icon on the left sidebar.
3. Search for **Wokwi**.
4. Click **Install** on the official extension by Wokwi.

### Step 2: Request and Activate Your License

Wokwi for VS Code requires a license to run. A personal license is free and can be renewed indefinitely.

1. Open the VS Code Command Palette.
2. Type and select the Wokwi license command.
3. A prompt will ask to open an external website — click **Open**.
4. Your browser will open the Wokwi website. Log into your account and click **GET YOUR LICENSE**.
5. The browser will request to redirect back to VS Code. Confirm the redirect to automatically activate your license.

### Step 3: Create or Open an Embedded Project

Wokwi relies on your local project compilation.

1. Open your firmware project folder in VS Code (e.g., an ESP32 or Arduino project managed by PlatformIO).
2. Build/Compile your project once so the compiler generates the binary files.

### Step 4: Configure the `wokwi.toml` File

Wokwi needs to know exactly where your compiled binary files reside.

1. In the root directory of your project, create a new file named `wokwi.toml`.
2. Add the template mapping configuration for your environment (PlatformIO or Arduino CLI). For PlatformIO, set the environment profile name to match your `platformio.ini`. For Arduino CLI, always use forward slashes in paths, even on Windows.

### Step 5: Add the `diagram.json` File

This file defines your virtual hardware components (microcontrollers, LEDs, sensors) and their electrical connections.

1. In the root directory of your project, create a new file named `diagram.json`.
2. The fastest way to generate this layout is to build your circuit visually on the Wokwi Web Simulator, then copy the diagram JSON and paste it into your local file.

### Step 6: Run the Simulation

1. Ensure your latest code changes are successfully compiled.
2. Right-click your `diagram.json` file in the Explorer pane and select **Open With... → Wokwi Diagram Editor**.
3. Alternatively, open the Command Palette and choose the Wokwi simulation command.
4. The interactive simulation panel will load inside VS Code. Click the **Play (▶)** button to run your code — serial print logs pipe directly into your VS Code output terminal.

---

**Author:** Sreya Vinod
