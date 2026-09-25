#include <Servo.h>

// Pin Definitions
const int ldrPin = A0;
const int tempPin = A1;
const int servoPin = 9;
const int fanPin = 3;
const int greenLedPin = 4;
const int whiteLedPin = 5;
const int redLedPin = 6;
const int buzzerPin = 7;

Servo ventServo;

// Variables
int ldrValue = 0;
float temperature = 0.0;
unsigned long previousMillis = 0;
const long interval = 500; // Blink interval for White LED (500ms)
bool whiteLedState = LOW;

void setup()
{
    pinMode(ldrPin, INPUT);
    pinMode(tempPin, INPUT);
    pinMode(fanPin, OUTPUT);
    pinMode(greenLedPin, OUTPUT);
    pinMode(whiteLedPin, OUTPUT);
    pinMode(redLedPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);

    ventServo.attach(servoPin);
    ventServo.write(0); // Start with vent closed

    Serial.begin(9600);
}

void loop()
{
    // 1. Read Temperature (TMP36 formula)
    int tempReading = analogRead(tempPin);
    float voltage = tempReading * 5.0 / 1024.0;
    temperature = (voltage - 0.5) * 100.0;

    // 2. Read Light Sensor
    ldrValue = analogRead(ldrPin);

    // 3. System Automation Logic

    // CONDITION 5: Temperature exceeds 40°C
    if (temperature > 40.0)
    {
        // FIXED: 240 bypasses the Tinkercad 255/HIGH motor stall glitch
        analogWrite(fanPin, 240);
        digitalWrite(buzzerPin, HIGH);  // Activate buzzer
        digitalWrite(redLedPin, HIGH);  // Turn ON Red LED
        ventServo.write(90);            // Vent open to 90°
        digitalWrite(greenLedPin, LOW); // Turn OFF normal operation indicator

        blinkWhiteLed(); // Force White LED to blink (Overrides LDR)
    }

    // CONDITION 4: Temperature exceeds 35°C (and <= 40°C)
    else if (temperature > 35.0 && temperature <= 40.0)
    {
        analogWrite(fanPin, 160); // Fixed Tinkercad speed step (Medium-High)
        ventServo.write(90);      // Open ventilation window to 90°

        // Reset emergency elements
        digitalWrite(buzzerPin, LOW);
        digitalWrite(redLedPin, LOW);
        digitalWrite(greenLedPin, HIGH); // Green LED indicates normal operation

        handleLdrLighting(); // Light intensity controls White LED
    }

    // CONDITION 3: Temperature is between 30°C and 35°C
    else if (temperature >= 30.0 && temperature <= 35.0)
    {
        analogWrite(fanPin, 80); // Safe lower PWM value to prevent reverse scaling
        ventServo.write(45);     // Open ventilation window to 45°

        // Reset emergency elements
        digitalWrite(buzzerPin, LOW);
        digitalWrite(redLedPin, LOW);
        digitalWrite(greenLedPin, HIGH); // Green LED indicates normal operation

        handleLdrLighting(); // Light intensity controls White LED
    }

    // CONDITION 6: Temperature falls below 30°C
    else
    {
        analogWrite(fanPin, 0);       // Hard turn OFF the fan (0 RPM)
        ventServo.write(0);           // Close the ventilation window
        digitalWrite(buzzerPin, LOW); // Turn OFF the buzzer
        digitalWrite(redLedPin, LOW); // Turn OFF the Red LED

        digitalWrite(greenLedPin, HIGH); // Green LED indicates normal operation

        handleLdrLighting(); // Light intensity controls White LED
    }

    delay(30);
}

// Helper function to handle Condition 1 (LDR Light Control)
void handleLdrLighting()
{
    if (ldrValue < 300)
    {
        digitalWrite(whiteLedPin, HIGH); // Turn ON White LED
    }
    else
    {
        digitalWrite(whiteLedPin, LOW); // Turn OFF White LED
    }
}

// Helper function to handle Condition 5 (Blinking White LED) safely
void blinkWhiteLed()
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;
        whiteLedState = !whiteLedState;
        digitalWrite(whiteLedPin, whiteLedState);
    }
}