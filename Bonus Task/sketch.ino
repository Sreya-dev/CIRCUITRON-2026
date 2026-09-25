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

// ─── Pin Definitions ────────────────────────────────────────
#define PIN_PH_SENSOR 34     // ADC1 - pH potentiometer
#define PIN_EC_SENSOR 35     // ADC1 - EC potentiometer
#define PIN_WLEVEL_SENSOR 32 // ADC1 - Water Level pot
#define PIN_DHT 4            // DHT22 data pin

#define RELAY_PH_UP 16    // Peristaltic pump - pH Up solution
#define RELAY_PH_DOWN 17  // Peristaltic pump - pH Down solution
#define RELAY_NUTRIENT 18 // Peristaltic pump - Nutrient solution
#define RELAY_FAN 19      // Exhaust fan relay

#define LED_PH_UP 25
#define LED_PH_DOWN 26
#define LED_NUTRIENT 27
#define LED_FAN 33

// ─── I2C LCD ────────────────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ─── DHT22 ──────────────────────────────────────────────────
DHT dht(PIN_DHT, DHT22);

// ─── Setpoints (target values) ───────────────────────────────
const float PH_SETPOINT = 6.2;      // Ideal pH for hydroponics
const float EC_SETPOINT = 1.8;      // mS/cm - nutrient conductivity
const float TEMP_MAX = 28.0;        // °C - max greenhouse temp
const float WATER_LEVEL_MIN = 40.0; // % minimum water level

// ─── PID Parameters ─────────────────────────────────────────
// Tune these for your simulation response speed
struct PIDConfig
{
    float Kp, Ki, Kd;
    float integral;
    float prevError;
    float outputMin, outputMax;
};

PIDConfig pidPH = {
    .Kp = 2.0,
    .Ki = 0.05,
    .Kd = 0.8,
    .integral = 0,
    .prevError = 0,
    .outputMin = -255,
    .outputMax = 255};

PIDConfig pidEC = {
    .Kp = 3.0,
    .Ki = 0.1,
    .Kd = 0.5,
    .integral = 0,
    .prevError = 0,
    .outputMin = 0,
    .outputMax = 255};

// ─── Pump pulse state ────────────────────────────────────────
struct PumpState
{
    bool active;
    unsigned long onTime;  // ms to keep on
    unsigned long offTime; // ms to keep off
    unsigned long lastToggle;
    bool isOn;
};

PumpState pHUpPump = {false, 0, 1000, 0, false};
PumpState pHDownPump = {false, 0, 1000, 0, false};
PumpState nutrientPump = {false, 0, 1000, 0, false};

// ─── Timing ─────────────────────────────────────────────────
unsigned long lastSensorRead = 0;
unsigned long lastLCDUpdate = 0;
unsigned long lastPIDRun = 0;
const unsigned long SENSOR_INTERVAL = 500;
const unsigned long LCD_INTERVAL = 800;
const unsigned long PID_INTERVAL = 2000;

// ─── Sensor Readings ─────────────────────────────────────────
float currentPH = 6.2;
float currentEC = 1.8;
float currentWLevel = 80.0;
float currentTemp = 24.0;
float currentHumid = 60.0;
int lcdPage = 0;

// ─── Relay control (active LOW) ──────────────────────────────
void setRelay(int pin, int ledPin, bool state)
{
    digitalWrite(pin, state ? LOW : HIGH);
    digitalWrite(ledPin, state ? HIGH : LOW);
}

// ─── PID Compute ─────────────────────────────────────────────
float computePID(PIDConfig &cfg, float setpoint, float measured, float dt)
{
    float error = setpoint - measured;

    cfg.integral += error * dt;
    // Anti-windup clamp
    cfg.integral = constrain(cfg.integral, cfg.outputMin / cfg.Ki, cfg.outputMax / cfg.Ki);

    float derivative = (error - cfg.prevError) / dt;
    cfg.prevError = error;

    float output = cfg.Kp * error + cfg.Ki * cfg.integral + cfg.Kd * derivative;

    return constrain(output, cfg.outputMin, cfg.outputMax);
}

// ─── Pulse pump based on PID output magnitude ─────────────────
// Maps |output| → on-time pulse (5ms–500ms per cycle)
// AFTER
void schedulePump(PumpState &pump, float magnitude)
{
    if (magnitude < 0.3)
    {
        pump.active = false;
        pump.isOn = false;
        return;
    }
    pump.active = true;
    // Map 0.3–20.0 range to 50ms–500ms pulse
    // constrain first so small errors still get minimum 50ms
    float clamped = constrain(magnitude, 0.3, 20.0);
    pump.onTime = (unsigned long)map((long)(clamped * 10), 3, 200, 50, 500);
    pump.offTime = 1000;
}

// ─── Non-blocking pump pulse handler ─────────────────────────
void handlePump(PumpState &pump, int relayPin, int ledPin)
{
    if (!pump.active)
    {
        setRelay(relayPin, ledPin, false);
        return;
    }
    unsigned long now = millis();
    if (pump.isOn && (now - pump.lastToggle >= pump.onTime))
    {
        pump.isOn = false;
        pump.lastToggle = now;
        setRelay(relayPin, ledPin, false);
    }
    else if (!pump.isOn && (now - pump.lastToggle >= pump.offTime))
    {
        pump.isOn = true;
        pump.lastToggle = now;
        setRelay(relayPin, ledPin, true);
    }
}

// ─── Read and scale analog sensors ────────────────────────────
void readSensors()
{
    // pH: 0–14 range, potentiometer maps 0–4095 → 0.0–14.0
    int rawPH = analogRead(PIN_PH_SENSOR);
    currentPH = (rawPH / 4095.0) * 14.0;

    // EC: 0–5 mS/cm
    int rawEC = analogRead(PIN_EC_SENSOR);
    currentEC = (rawEC / 4095.0) * 5.0;

    // Water Level: 0–100%
    int rawWL = analogRead(PIN_WLEVEL_SENSOR);
    currentWLevel = (rawWL / 4095.0) * 100.0;

    // DHT22
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t))
        currentTemp = t;
    if (!isnan(h))
        currentHumid = h;
}

// ─── LCD Display (two-page rotation) ──────────────────────────
void updateLCD()
{
    lcd.clear();
    if (lcdPage == 0)
    {
        // Page 1: pH and EC
        lcd.setCursor(0, 0);
        lcd.print("pH:");
        lcd.print(currentPH, 1);
        lcd.print(" SP:");
        lcd.print(PH_SETPOINT, 1);

        lcd.setCursor(0, 1);
        lcd.print("EC:");
        lcd.print(currentEC, 2);
        lcd.print(" SP:");
        lcd.print(EC_SETPOINT, 1);
    }
    else if (lcdPage == 1)
    {
        // Page 2: Temp, Humidity, Water Level
        lcd.setCursor(0, 0);
        lcd.print("T:");
        lcd.print(currentTemp, 1);
        lcd.print("C H:");
        lcd.print(currentHumid, 0);
        lcd.print("%");

        lcd.setCursor(0, 1);
        lcd.print("WLvl:");
        lcd.print(currentWLevel, 0);
        lcd.print("%");
        if (currentWLevel < WATER_LEVEL_MIN)
        {
            lcd.print(" LOW!");
        }
    }
    else
    {
        // Page 3: Pump/system status
        lcd.setCursor(0, 0);
        lcd.print("UP:");
        lcd.print(pHUpPump.active ? "ON " : "OFF");
        lcd.print(" DN:");
        lcd.print(pHDownPump.active ? "ON" : "OFF");

        lcd.setCursor(0, 1);
        lcd.print("NUT:");
        lcd.print(nutrientPump.active ? "ON " : "OFF");
        lcd.print(" FAN:");
        lcd.print(currentTemp > TEMP_MAX ? "ON" : "OF");
    }
    lcdPage = (lcdPage + 1) % 3;
}

// ─── Run PID controllers ──────────────────────────────────────
void runPIDControl(float dt)
{

    // ── pH Control ─────────────────────────────────────────
    float phError = PH_SETPOINT - currentPH;

    // Hard reset integral when error crosses zero (sign change)
    // This prevents old accumulated integral from overriding new direction
    if ((phError > 0 && pidPH.prevError < 0) ||
        (phError < 0 && pidPH.prevError > 0))
    {
        pidPH.integral = 0;
    }

    // Reset integral inside dead-band
    if (abs(phError) < 0.1)
    {
        pidPH.integral = 0;
    }

    float phOutput = computePID(pidPH, PH_SETPOINT, currentPH, dt);

    if (phOutput > 0.3)
    {
        // pH too LOW → dose pH Up
        schedulePump(pHUpPump, phOutput);
        schedulePump(pHDownPump, 0);
        pHDownPump.active = false;
    }
    else if (phOutput < -0.3)
    {
        // pH too HIGH → dose pH Down
        schedulePump(pHDownPump, abs(phOutput));
        schedulePump(pHUpPump, 0);
        pHUpPump.active = false;
    }
    else
    {
        // Inside dead-band → stop both, clear integral
        pidPH.integral = 0;
        schedulePump(pHUpPump, 0);
        schedulePump(pHDownPump, 0);
        pHUpPump.active = false;
        pHDownPump.active = false;
    }

    // ── EC / Nutrient Control ───────────────────────────────
    float ecError = EC_SETPOINT - currentEC;

    if (abs(ecError) < 0.1)
    {
        pidEC.integral = 0;
    }

    float ecOutput = computePID(pidEC, EC_SETPOINT, currentEC, dt);

    if (ecOutput > 0.3)
    {
        schedulePump(nutrientPump, ecOutput);
    }
    else
    {
        pidEC.integral = 0;
        schedulePump(nutrientPump, 0);
        nutrientPump.active = false;
    }

    // ── Fan Control ─────────────────────────────────────────
    static bool fanOn = false;
    if (currentTemp > TEMP_MAX + 0.5)
        fanOn = true;
    else if (currentTemp < TEMP_MAX - 0.5)
        fanOn = false;
    setRelay(RELAY_FAN, LED_FAN, fanOn);

    // ── Water level alert ───────────────────────────────────
    // (handled on LCD already)
}

// ─── Serial debug output ──────────────────────────────────────
void printDebug()
{
    Serial.printf(
        "[SENSOR] pH=%.2f | EC=%.2f | WL=%.1f%% | T=%.1fC | H=%.1f%%\n",
        currentPH, currentEC, currentWLevel, currentTemp, currentHumid);
    Serial.printf(
        "[PID]    pH_err=%.2f | EC_err=%.2f\n",
        PH_SETPOINT - currentPH, EC_SETPOINT - currentEC);
    Serial.printf(
        "[PUMPS]  pH_UP=%s | pH_DN=%s | NUT=%s | FAN=%s\n",
        pHUpPump.active ? "ON" : "off",
        pHDownPump.active ? "ON" : "off",
        nutrientPump.active ? "ON" : "off",
        (currentTemp > TEMP_MAX) ? "ON" : "off");
    Serial.println("─────────────────────────────────────");
}

// ═══════════════════════════════════════════════════════════════
void setup()
{
    Serial.begin(115200);
    Serial.println("\n=== Hydroponic Guardian Boot ===");

    // Relays & LEDs
    int relays[] = {RELAY_PH_UP, RELAY_PH_DOWN, RELAY_NUTRIENT, RELAY_FAN};
    int leds[] = {LED_PH_UP, LED_PH_DOWN, LED_NUTRIENT, LED_FAN};
    for (int i = 0; i < 4; i++)
    {
        pinMode(relays[i], OUTPUT);
        pinMode(leds[i], OUTPUT);
        digitalWrite(relays[i], HIGH); // Relays off (active LOW)
        digitalWrite(leds[i], LOW);
    }

    // ADC pins (already input by default on ESP32)
    analogReadResolution(12); // 0–4095

    // DHT22
    dht.begin();

    // LCD
    Wire.begin(21, 22);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("  Hydroponic    ");
    lcd.setCursor(0, 1);
    lcd.print("   Guardian!    ");
    delay(2000);
    lcd.clear();

    Serial.println("System ready. Adjust pots to simulate sensors.");
}

// ═══════════════════════════════════════════════════════════════
void loop()
{
    unsigned long now = millis();

    // Read sensors every 500ms
    if (now - lastSensorRead >= SENSOR_INTERVAL)
    {
        readSensors();
        lastSensorRead = now;
    }

    // Run PID every 2 seconds
    if (now - lastPIDRun >= PID_INTERVAL)
    {
        float dt = PID_INTERVAL / 1000.0;
        runPIDControl(dt);
        printDebug();
        lastPIDRun = now;
    }

    // Update LCD every 800ms
    if (now - lastLCDUpdate >= LCD_INTERVAL)
    {
        updateLCD();
        lastLCDUpdate = now;
    }

    // Handle pump pulsing (non-blocking)
    handlePump(pHUpPump, RELAY_PH_UP, LED_PH_UP);
    handlePump(pHDownPump, RELAY_PH_DOWN, LED_PH_DOWN);
    handlePump(nutrientPump, RELAY_NUTRIENT, LED_NUTRIENT);
}
