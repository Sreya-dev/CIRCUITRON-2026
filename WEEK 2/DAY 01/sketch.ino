#include <Adafruit_NeoPixel.h>

#define LED_PIN 5
#define NUM_LEDS 16
#define NEXT_BTN 12
#define COLOR_BTN 14

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

const uint32_t COLORS[] = {
    0x000000, // 0 OFF
    0xFF0000, // 1 RED
    0x00FF00, // 2 GREEN
    0x0000FF, // 3 BLUE
    0xFFFF00, // 4 YELLOW
    0x800080, // 5 PURPLE
    0x00FFFF, // 6 CYAN
    0xFFFFFF  // 7 WHITE
};
const char *COLOR_NAMES[] = {
    "OFF", "RED", "GREEN", "BLUE", "YELLOW", "PURPLE", "CYAN", "WHITE"};
const int NUM_COLORS = 8;

int ledColorState[NUM_LEDS];
int selectedLED = 0;

bool nextLastRaw = HIGH, nextStable = HIGH;
bool colorLastRaw = HIGH, colorStable = HIGH;
unsigned long nextDebounce = 0, colorDebounce = 0;
const unsigned long DEBOUNCE_MS = 50;

unsigned long bothPressedAt = 0;
bool resetTriggered = false;
const unsigned long RESET_HOLD_MS = 2000;

void showStrip()
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        strip.setPixelColor(i, (i == selectedLED) ? 0xFFFFFF : COLORS[ledColorState[i]]);
    }
    strip.show();
}

void printStatus()
{
    Serial.print("Selected LED: ");
    Serial.print(selectedLED + 1);
    Serial.print("  |  Color: ");
    Serial.println(COLOR_NAMES[ledColorState[selectedLED]]);
}

void resetAll()
{
    for (int i = 0; i < NUM_LEDS; i++)
        ledColorState[i] = 0;
    selectedLED = 0;
    strip.clear();
    strip.show();
    Serial.println("\n================================");
    Serial.println("         SYSTEM RESET           ");
    Serial.println("================================");
    delay(200);
    showStrip();
    printStatus();
}

void setup()
{
    Serial.begin(115200);
    delay(1000); // Guard time to make sure console logging catches initialization frames
    Serial.println("Hello, Wokwi!");

    pinMode(NEXT_BTN, INPUT_PULLUP);
    pinMode(COLOR_BTN, INPUT_PULLUP);

    strip.begin();
    strip.setBrightness(80);
    strip.clear();
    strip.show();

    for (int i = 0; i < NUM_LEDS; i++)
        ledColorState[i] = 0;

    Serial.println("================================");
    Serial.println("  ESP32 RGB LED Controller      ");
    Serial.println("================================");
    Serial.println("NEXT  button -> GPIO 12");
    Serial.println("COLOR button -> GPIO 14");
    Serial.println("Hold BOTH 2s -> SYSTEM RESET");
    Serial.println("--------------------------------");
    showStrip();
    printStatus();
}

void loop()
{
    bool nextRaw = digitalRead(NEXT_BTN);
    bool colorRaw = digitalRead(COLOR_BTN);

    // Core system reset timer tracking logic
    if (nextRaw == LOW && colorRaw == LOW)
    {
        if (bothPressedAt == 0)
        {
            bothPressedAt = millis();
            resetTriggered = false;
            Serial.println("[Hold detected - keep holding 2s for RESET...]");
        }
        else if (!resetTriggered && (millis() - bothPressedAt >= RESET_HOLD_MS))
        {
            resetTriggered = true;
            resetAll();
        }
    }
    else
    {
        bothPressedAt = 0;
        resetTriggered = false;
    }

    // ── NEXT button debounce ─────────────────────────────────────────
    if (nextRaw != nextLastRaw)
        nextDebounce = millis();
    if ((millis() - nextDebounce) > DEBOUNCE_MS && nextRaw != nextStable)
    {
        nextStable = nextRaw;
        if (nextStable == LOW && colorRaw == HIGH)
        {
            selectedLED = (selectedLED + 1) % NUM_LEDS;
            showStrip();
            printStatus();
        }
    }
    nextLastRaw = nextRaw;

    // ── COLOR button debounce ────────────────────────────────────────
    if (colorRaw != colorLastRaw)
        colorDebounce = millis();
    if ((millis() - colorDebounce) > DEBOUNCE_MS && colorRaw != colorStable)
    {
        colorStable = colorRaw;
        if (colorStable == LOW && nextRaw == HIGH)
        {
            ledColorState[selectedLED] = (ledColorState[selectedLED] + 1) % NUM_COLORS;
            showStrip();
            printStatus();
        }
    }
    colorLastRaw = colorRaw;
}
