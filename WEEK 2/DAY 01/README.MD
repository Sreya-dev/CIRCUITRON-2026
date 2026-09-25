# 🌈 ESP32 RGB LED Controller

**Platform:** Wokwi Simulation · **Microcontroller:** ESP32 DevKit C V4

🔗 **Wokwi Simulation:** [https://wokwi.com/projects/466247974476121089](https://wokwi.com/projects/466247974476121089)

---

## 📌 Project Aim

To design and simulate an ESP32-based RGB LED Controller using 16 RGB LEDs (NeoPixel strip) and 2 push buttons, where each LED can be individually selected and its color independently cycled through a predefined palette — simulated entirely within the Wokwi platform.

---

## 📖 Project Overview

This project implements an interactive LED control system on an ESP32 microcontroller. The system manages 16 individually addressable RGB LEDs using the WS2812B NeoPixel protocol. Two push buttons allow the user to navigate between LEDs and change their colors without affecting any other LED in the strip.

The controller supports 8 color states per LED (OFF, RED, GREEN, BLUE, YELLOW, PURPLE, CYAN, WHITE), provides real-time Serial Monitor feedback, and includes a dual-button reset feature. The entire project is designed and simulated using the Wokwi online simulator, removing the need for physical hardware during development.

### Key Behaviors

- All 16 LEDs start in the OFF state
- The currently selected LED glows WHITE as a visual indicator
- **NEXT** button cycles the selection through LED 1→2→…→16→1
- **COLOR** button steps the selected LED through 8 color states
- Color changes are fully independent per LED
- Holding both buttons simultaneously for 2 seconds resets all LEDs and prints `SYSTEM RESET` to Serial Monitor

---

## 🧩 Components Used

| Component       | Specification                           | Quantity |
| --------------- | --------------------------------------- | -------- |
| Microcontroller | ESP32 DevKit C V4                       | 1        |
| RGB LED Strip   | WS2812B NeoPixel (16 LEDs, 1×16 matrix) | 1        |
| Push Button     | Tactile push button (Green — NEXT)      | 1        |
| Push Button     | Tactile push button (Blue — COLOR)      | 1        |
| Power Supply    | 5V via ESP32 onboard regulator          | —        |
| Library         | Adafruit NeoPixel                       | —        |

### GPIO Pin Assignment

| Signal        | GPIO Pin |
| ------------- | -------- |
| NeoPixel Data | GPIO 5   |
| NEXT Button   | GPIO 12  |
| COLOR Button  | GPIO 14  |

---

## 🔌 Construction and Wiring Details

### Power and Ground

- ESP32 5V pin → NeoPixel strip VCC
- ESP32 GND → NeoPixel strip GND
- ESP32 GND → NEXT button pin 2 (ground side)
- ESP32 GND → COLOR button pin 2 (ground side)

### Signal Lines

- ESP32 GPIO 5 → NeoPixel strip DIN (data input)
- ESP32 GPIO 12 → NEXT button pin 1
- ESP32 GPIO 14 → COLOR button pin 1

**Button Configuration:** Both buttons use the ESP32's internal pull-up resistors (`INPUT_PULLUP`). When a button is pressed, the GPIO reads LOW; when released, it reads HIGH. This is standard active-low logic and eliminates the need for external pull-up resistors.

**NeoPixel Strip:** The 16-LED NeoPixel matrix (1 row × 16 columns) is powered directly from the ESP32's 5V pin with brightness set to 80/255 in firmware to stay within safe current limits.

**Serial Monitor:**

- ESP32 TX → Serial Monitor RX
- ESP32 RX → Serial Monitor TX
- Baud rate: 115200

---

## 🔄 Code Structure / Program Flow

### Program Initialization (`setup()`)

1. Serial communication begins at 115200 baud
2. Both button GPIOs configured as `INPUT_PULLUP`
3. NeoPixel strip initialized, brightness set to 80
4. All LED color states set to 0 (OFF)
5. Startup banner printed to Serial Monitor
6. `showStrip()` called to display initial state (selected LED 1 glows white, all others OFF)

### Main Loop (`loop()`)

The loop runs continuously and handles three independent concerns:

**1. Dual-Button Reset Detection**

- Checks if both buttons are pressed simultaneously (LOW)
- Starts a timer on first detection
- If both held for ≥ 2000 ms, calls `resetAll()`
- Timer resets if either button is released before 2 seconds

**2. NEXT Button (with debounce)**

- Raw GPIO read compared to last known stable state
- 50 ms debounce window filters mechanical bounce
- On confirmed falling edge (HIGH→LOW), and only if COLOR button is NOT pressed:
  - `selectedLED = (selectedLED + 1) % 16`
  - `showStrip()` and `printStatus()` called

**3. COLOR Button (with debounce)**

- Same debounce logic as NEXT button
- On confirmed falling edge, and only if NEXT button is NOT pressed:
  - `ledColorState[selectedLED] = (ledColorState[selectedLED] + 1) % 8`
  - `showStrip()` and `printStatus()` called

### Helper Functions

- **`showStrip()`** — Iterates through all 16 LEDs. The selected LED is always rendered WHITE (0xFFFFFF) regardless of its stored color state. All other LEDs are rendered using their stored color from the `COLORS[]` array. Calls `strip.show()` to push data to the strip.
- **`printStatus()`** — Prints the current selected LED number (1-indexed) and its color name to Serial Monitor.
- **`resetAll()`** — Sets all `ledColorState[]` entries to 0 (OFF), resets `selectedLED` to 0, clears the strip, prints the `SYSTEM RESET` banner, then refreshes display and status.

---

## 🎨 Color State Table

| State Index | Color Name | Hex Value |
| ----------- | ---------- | --------- |
| 0           | OFF        | 0x000000  |
| 1           | RED        | 0xFF0000  |
| 2           | GREEN      | 0x00FF00  |
| 3           | BLUE       | 0x0000FF  |
| 4           | YELLOW     | 0xFFFF00  |
| 5           | PURPLE     | 0x800080  |
| 6           | CYAN       | 0x00FFFF  |
| 7           | WHITE      | 0xFFFFFF  |

---

## 💻 Source Code

```cpp
#include <Adafruit_NeoPixel.h>

// ── Pin Definitions ──────────────────────────────────────────────────────────
#define LED_PIN 5      // NeoPixel data pin
#define NUM_LEDS 16    // Total number of RGB LEDs
#define NEXT_BTN 12    // NEXT button GPIO (active LOW)
#define COLOR_BTN 14   // COLOR button GPIO (active LOW)

// ── NeoPixel Strip Initialization ────────────────────────────────────────────
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// ── Color Palette (8 states) ─────────────────────────────────────────────────
const uint32_t COLORS[] = {
  0x000000, // 0 → OFF
  0xFF0000, // 1 → RED
  0x00FF00, // 2 → GREEN
  0x0000FF, // 3 → BLUE
  0xFFFF00, // 4 → YELLOW
  0x800080, // 5 → PURPLE
  0x00FFFF, // 6 → CYAN
  0xFFFFFF  // 7 → WHITE
};

const char* COLOR_NAMES[] = {
  "OFF", "RED", "GREEN", "BLUE", "YELLOW", "PURPLE", "CYAN", "WHITE"
};
const int NUM_COLORS = 8;

// ── State Variables ───────────────────────────────────────────────────────────
int ledColorState[NUM_LEDS]; // Stores each LED's current color index
int selectedLED = 0;         // Index of the currently selected LED (0–15)

// ── Debounce Variables ────────────────────────────────────────────────────────
bool nextLastRaw = HIGH, nextStable = HIGH;
bool colorLastRaw = HIGH, colorStable = HIGH;
unsigned long nextDebounce = 0;
unsigned long colorDebounce = 0;
const unsigned long DEBOUNCE_MS = 50; // 50 ms debounce window

// ── Dual-Button Reset Variables ───────────────────────────────────────────────
unsigned long bothPressedAt = 0;
bool resetTriggered = false;
const unsigned long RESET_HOLD_MS = 2000; // 2-second hold for reset

// ───────────────────────────────────────────────────────────────────────────
// showStrip(): Renders all LEDs. Selected LED shows WHITE; others show stored color.
// ───────────────────────────────────────────────────────────────────────────
void showStrip() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i == selectedLED) {
      strip.setPixelColor(i, 0xFFFFFF); // Selection indicator: WHITE
    } else {
      strip.setPixelColor(i, COLORS[ledColorState[i]]); // Stored color
    }
  }
  strip.show();
}

// ───────────────────────────────────────────────────────────────────────────
// printStatus(): Prints selected LED and its color to Serial Monitor.
// ───────────────────────────────────────────────────────────────────────────
void printStatus() {
  Serial.print("Selected LED: ");
  Serial.print(selectedLED + 1); // 1-indexed for user readability
  Serial.print(" | Color: ");
  Serial.println(COLOR_NAMES[ledColorState[selectedLED]]);
}

// ───────────────────────────────────────────────────────────────────────────
// resetAll(): Resets all LEDs to OFF and re-selects LED 1.
// ───────────────────────────────────────────────────────────────────────────
void resetAll() {
  for (int i = 0; i < NUM_LEDS; i++) ledColorState[i] = 0; // All OFF
  selectedLED = 0;
  strip.clear();
  strip.show();

  Serial.println("\n================================");
  Serial.println("      SYSTEM RESET");
  Serial.println("================================");
  delay(200);

  showStrip(); // Re-display with LED 1 selected (WHITE)
  printStatus();
}

// ───────────────────────────────────────────────────────────────────────────
// setup(): One-time initialization
// ───────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(1000); // Allow Serial Monitor to connect

  // Configure button pins with internal pull-up resistors
  pinMode(NEXT_BTN, INPUT_PULLUP);
  pinMode(COLOR_BTN, INPUT_PULLUP);

  // Initialize NeoPixel strip
  strip.begin();
  strip.setBrightness(80); // ~31% brightness — safe current draw
  strip.clear();
  strip.show();

  // Initialize all LED color states to OFF
  for (int i = 0; i < NUM_LEDS; i++) ledColorState[i] = 0;

  // Print startup banner
  Serial.println("================================");
  Serial.println("   ESP32 RGB LED Controller");
  Serial.println("================================");
  Serial.println("NEXT button  -> GPIO 12");
  Serial.println("COLOR button -> GPIO 14");
  Serial.println("Hold BOTH 2s -> SYSTEM RESET");
  Serial.println("--------------------------------");

  showStrip(); // LED 1 lights WHITE (selection indicator)
  printStatus();
}

// ───────────────────────────────────────────────────────────────────────────
// loop(): Runs continuously — handles button debounce and state transitions
// ───────────────────────────────────────────────────────────────────────────
void loop() {
  bool nextRaw = digitalRead(NEXT_BTN);
  bool colorRaw = digitalRead(COLOR_BTN);

  // ── Dual-Button Reset Detection ──────────────────────────────────────────
  if (nextRaw == LOW && colorRaw == LOW) {
    if (bothPressedAt == 0) {
      bothPressedAt = millis();
      resetTriggered = false;
      Serial.println("[Hold detected -- keep holding 2s for RESET...]");
    } else if (!resetTriggered && (millis() - bothPressedAt >= RESET_HOLD_MS)) {
      resetTriggered = true;
      resetAll();
    }
  } else {
    bothPressedAt = 0; // Reset timer if either button released
    resetTriggered = false;
  }

  // ── NEXT Button — Debounce and Edge Detection ────────────────────────────
  if (nextRaw != nextLastRaw) nextDebounce = millis();
  if ((millis() - nextDebounce) > DEBOUNCE_MS && nextRaw != nextStable) {
    nextStable = nextRaw;
    if (nextStable == LOW && colorRaw == HIGH) { // Only if COLOR not pressed
      selectedLED = (selectedLED + 1) % NUM_LEDS; // Advance selection (wraps at 16)
      showStrip();
      printStatus();
    }
  }
  nextLastRaw = nextRaw;

  // ── COLOR Button — Debounce and Edge Detection ───────────────────────────
  if (colorRaw != colorLastRaw) colorDebounce = millis();
  if ((millis() - colorDebounce) > DEBOUNCE_MS && colorRaw != colorStable) {
    colorStable = colorRaw;
    if (colorStable == LOW && nextRaw == HIGH) { // Only if NEXT not pressed
      ledColorState[selectedLED] = (ledColorState[selectedLED] + 1) % NUM_COLORS;
      showStrip();
      printStatus();
    }
  }
  colorLastRaw = colorRaw;
}
```

---

## 🗂️ Wokwi Diagram JSON

```json
{
  "version": 1,
  "author": "Sreya V",
  "editor": "wokwi",
  "parts": [
    {
      "type": "board-esp32-devkit-c-v4",
      "id": "esp",
      "top": 100,
      "left": 0,
      "attrs": {}
    },
    {
      "type": "wokwi-neopixel-matrix",
      "id": "pixels",
      "top": 0,
      "left": 230,
      "attrs": { "cols": "16", "rows": "1" }
    },
    {
      "type": "wokwi-pushbutton",
      "id": "nextBtn",
      "top": 255.8,
      "left": 230.4,
      "attrs": { "color": "green", "label": "NEXT" }
    },
    {
      "type": "wokwi-pushbutton",
      "id": "colorBtn",
      "top": 260,
      "left": 340,
      "attrs": { "color": "blue", "label": "COLOR" }
    }
  ],
  "connections": [
    ["esp:GND.1", "pixels:GND", "black", []],
    ["esp:5V", "pixels:VCC", "red", []],
    ["esp:5", "pixels:DIN", "green", []],
    ["esp:12", "nextBtn:1.l", "green", []],
    ["esp:GND.2", "nextBtn:2.l", "black", []],
    ["esp:14", "colorBtn:1.l", "blue", []],
    ["esp:GND.3", "colorBtn:2.l", "black", []],
    ["esp:TX", "$serialMonitor:RX", "", []],
    ["esp:RX", "$serialMonitor:TX", "", []]
  ],
  "dependencies": {}
}
```

### Wokwi Libraries File

```
# Wokwi Library List
# See https://docs.wokwi.com/guides/libraries
# Automatically added based on includes:
Adafruit NeoPixel
```

---

## 🔄 Program Flow Diagram

```
STARTUP
  → Initialize Serial, GPIOs, NeoPixel
  → All LEDs = OFF | selectedLED = 0
  → LED 1 lights WHITE (selection indicator)
  → Print startup banner to Serial

MAIN LOOP (continuous)
  ├─► BOTH buttons held ≥ 2s?
  │     YES → resetAll() → All LEDs OFF, selectedLED = 0, print "SYSTEM RESET"
  │
  ├─► NEXT button pressed? (debounced, COLOR not held)
  │     YES → selectedLED = (selectedLED + 1) % 16
  │           → showStrip() → printStatus()
  │
  └─► COLOR button pressed? (debounced, NEXT not held)
        YES → ledColorState[selectedLED] = (state + 1) % 8
              → showStrip() → printStatus()
```

---

## 🖥️ Sample Serial Monitor Output

```
================================
   ESP32 RGB LED Controller
================================
NEXT button  -> GPIO 12
COLOR button -> GPIO 14
Hold BOTH 2s -> SYSTEM RESET
--------------------------------
Selected LED: 1 | Color: OFF
Selected LED: 2 | Color: OFF
Selected LED: 2 | Color: RED
Selected LED: 2 | Color: GREEN
Selected LED: 3 | Color: OFF
[Hold detected -- keep holding 2s for RESET...]
================================
      SYSTEM RESET
================================
Selected LED: 1 | Color: OFF
```

---

## ✨ Additional Features / Enhancements

Beyond the basic requirements, the following enhancements were implemented:

1. **Software Debouncing** — A 50 ms debounce algorithm filters mechanical button noise on both GPIOs, preventing false triggers without requiring any external capacitors or resistors.
2. **Mutual Exclusion Logic** — The NEXT and COLOR button handlers each check that the other button is not simultaneously pressed. This prevents accidental LED navigation or color changes during the 2-second reset hold sequence.
3. **Informative Reset Feedback** — On detection of both buttons being held, an intermediate message `[Hold detected — keep holding 2s for RESET...]` is immediately printed, giving the user real-time confirmation that the gesture has been recognized.
4. **Brightness Control** — `strip.setBrightness(80)` limits current draw to a safe level for the ESP32's onboard 5V regulator, making the design hardware-safe without external power circuitry.
5. **1-Indexed Serial Output** — Serial Monitor displays LED numbers as 1–16 (not 0–15), which is more intuitive and user-friendly for demonstration and evaluation purposes.

---

**Author:** Sreya Vinod
