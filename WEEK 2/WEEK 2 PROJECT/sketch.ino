/*
  ============================================================
  Smart City Emergency Response & Safety Management System
  Platform : Wokwi (ESP32)
  Author   : Sreya
  ============================================================
  PIN MAP
  -------
  HC-SR04  TRIG → GPIO 5   ECHO → GPIO 18
  MQ2      AOUT → GPIO 34
  DHT22    DATA → GPIO 4
  SSD1306  SDA  → GPIO 21  SCL  → GPIO 22
  Push Btn      → GPIO 15  (one leg → GPIO15, other leg → 3.3V)
  Green LED     → GPIO 26  (via 220Ω)
  Red LED       → GPIO 27  (via 220Ω)
  Buzzer        → GPIO 25
  Servo         → GPIO 13
  Relay         → GPIO 12  (active HIGH)
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
#define GAS_THRESHOLD 600 // applied after map() remapping
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
long readUltrasonic()
{
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
void setServo(int angle)
{
    servoAngle = angle;
    barrierServo.write(angle);
}

// ══════════════════════════════════════════════════════════
// Buzzer ON/OFF
// ══════════════════════════════════════════════════════════
void setBuzzer(bool on)
{
    if (on)
    {
        tone(BUZZER_PIN, 300);
    }
    else
    {
        noTone(BUZZER_PIN);
    }
}

// ══════════════════════════════════════════════════════════
// OLED: Normal 5-line city dashboard
// Shown during NORMAL and WARNING states
// ══════════════════════════════════════════════════════════
void showDashboard(const char *parkStatus,
                   const char *gasStatus,
                   const char *barrierStatus,
                   const char *cityStatus)
{
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.print("PKG : ");
    display.println(parkStatus);

    display.setCursor(0, 12);
    display.print("AIR : ");
    display.print(gasStatus);
    display.print(" (");
    display.print(currentGas);
    display.println(")");

    display.setCursor(0, 24);
    display.print("TMP : ");
    display.print(currentTemp, 1);
    display.println(" C");

    display.setCursor(0, 36);
    display.print("ROAD: ");
    display.println(barrierStatus);

    display.drawFastHLine(0, 47, 128, SSD1306_WHITE);

    // Status line with inverted highlight
    display.setCursor(0, 51);
    display.print("STS:");
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.print(" ");
    display.print(cityStatus);
    display.print(" ");
    display.setTextColor(SSD1306_WHITE);

    display.display();
}

// ══════════════════════════════════════════════════════════
// OLED: Full-screen alert
// Shown only during EMERGENCY conditions
// ══════════════════════════════════════════════════════════
void showAlert(const char *headerLine,
               const char *bigLine1,
               const char *bigLine2)
{
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(headerLine);

    display.setTextSize(2);
    display.setCursor(0, 16);
    display.println(bigLine1);
    display.setCursor(0, 40);
    display.println(bigLine2);

    display.display();
}

// ══════════════════════════════════════════════════════════
// SETUP
// ══════════════════════════════════════════════════════════
void setup()
{
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
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println("SSD1306 failed — check wiring!");
        while (true)
            ;
    }

    // Splash screen
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(8, 10);
    display.println("SMART CITY SYSTEM");
    display.setCursor(8, 26);
    display.println("Initialising...");
    display.setCursor(8, 42);
    display.println("Author: Sreya");
    display.display();
    delay(2500);
}

// ══════════════════════════════════════════════════════════
// MAIN LOOP
// ══════════════════════════════════════════════════════════
void loop()
{

    // ── 1. READ ALL SENSORS ──────────────────────────────────

    currentDist = readUltrasonic();

    currentTemp = dht.readTemperature();
    if (isnan(currentTemp))
        currentTemp = 0.0; // guard NaN on read failure

    // MQ2: 20-sample average to reduce ADC noise
    // Raw Wokwi MQ2 range is 843 (clean air) to 4095 (max gas)
    // map() remaps this to 0–1023 so GAS_THRESHOLD 600 works correctly
    long gasSum = 0;
    for (int i = 0; i < 20; i++)
    {
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
    pollutionAlert = (currentGas > GAS_THRESHOLD); // Condition 2
    fireAlert = (currentTemp > TEMP_THRESHOLD);    // Condition 3
    cityEmergency = (pollutionAlert && fireAlert); // Condition 6

    // ── 3. ACTUATOR CONTROL ──────────────────────────────────

    // LEDs — Red if parking full or city emergency, else Green (Conditions 1 & 6)
    if (cityEmergency || parkingOccupied)
    {
        digitalWrite(RED_LED, HIGH);
        digitalWrite(GREEN_LED, LOW);
    }
    else
    {
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
    if (cityEmergency || btnPressed)
    {
        setServo(90); // Emergency route OPEN
    }
    else
    {
        setServo(0); // Road CLOSED
    }

    // ── 4. SERIAL DEBUG ──────────────────────────────────────
    Serial.printf(
        "Dist:%ld cm | Gas:%d | Temp:%.1f C | Btn:%d | "
        "Park:%s | Poll:%s | Fire:%s | CityEmg:%s | Servo:%d\n",
        currentDist, currentGas, currentTemp, btnPressed,
        parkingOccupied ? "YES" : "NO",
        pollutionAlert ? "YES" : "NO",
        fireAlert ? "YES" : "NO",
        cityEmergency ? "YES" : "NO",
        servoAngle);

    // ── 5. OLED UPDATE every 500ms ───────────────────────────
    if (millis() - lastDisplayUpdate >= DISPLAY_INTERVAL)
    {
        lastDisplayUpdate = millis();

        const char *parkStr = parkingOccupied ? "FULL" : "AVAIL";
        const char *gasStr = pollutionAlert ? "POLL" : "SAFE";
        const char *barrierStr = (servoAngle == 90) ? "OPEN" : "CLOSED";

        // Specific status labels — WARNING shows exact cause
        const char *cityStatus;
        if (cityEmergency)
            cityStatus = "!! EMERGENCY !!";
        else if (btnPressed)
            cityStatus = "!! EMERGENCY !!";
        else if (pollutionAlert && fireAlert)
            cityStatus = "WARN:FIRE+GAS";
        else if (fireAlert)
            cityStatus = "WARN: FIRE";
        else if (pollutionAlert)
            cityStatus = "WARN: GAS";
        else
            cityStatus = "NORMAL";

        // OLED priority:
        // EMERGENCY conditions → full-screen alert
        // WARNING / NORMAL     → dashboard (status line shows current state)
        if (cityEmergency)
        {
            showAlert("!! CITY EMERGENCY !!", "CITY EMERG", "EVACUATE!");
        }
        else if (btnPressed)
        {
            // Full screen only while button is held — clears on release
            showAlert("!! CITIZEN EMERGENCY !!", "EMERGENCY", "REPORTED!!");
        }
        else
        {
            // Dashboard shown for NORMAL and WARNING states
            showDashboard(parkStr, gasStr, barrierStr, cityStatus);
        }
    }

    delay(100); // 10 Hz main loop
}
