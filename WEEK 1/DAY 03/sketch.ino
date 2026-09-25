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

// System States
enum GateState
{
    CLOSED,
    NIGHT_DELAY,
    OPEN,
    OVERSTAY_ALARM
};
GateState currentGateState = CLOSED;

void setup()
{
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(pirPin, INPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(greenLed, OUTPUT);
    pinMode(redLed, OUTPUT);

    // Detaching/Writing before attach completely stops the Tinkercad servo jump glitch
    gateServo.write(0);
    gateServo.attach(servoPin);

    // Initialize Default LED Status (Closed = Red)
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, LOW);

    Serial.begin(9600);
    Serial.println("=====================================");
    Serial.println("  Smart Parking Gate System Online   ");
    Serial.println("=====================================");
}

void loop()
{
    unsigned long currentMillis = millis();

    // 1. Fetch Real-time Sensor Data
    int distance = readDistance();
    int motion = digitalRead(pirPin);
    int ldrValue = analogRead(ldrPin);

    // Day/Night Threshold Evaluation
    // UPDATED FIX: Adjusted threshold from 300 to 750
    // (Bright yields 1021 -> DAY | Dark yields 563 -> NIGHT)
    bool isDay = (ldrValue > 750);

    // 2. Continuous Proximity Tracking
    if (distance > 0 && distance <= 20)
    {
        if (!vehiclePresent)
        {
            vehicleArrivalTime = currentMillis; // Mark exact entry timestamp
            vehiclePresent = true;
        }
    }
    else
    {
        vehiclePresent = false;
        vehicleArrivalTime = 0; // Clear timer when car completely backs away
    }

    // 3. Serial Monitor System Logs (Runs every 500ms for tracking)
    static unsigned long lastLog = 0;
    if (currentMillis - lastLog > 500)
    {
        Serial.print("Dist: ");
        Serial.print(distance);
        Serial.print("cm");
        Serial.print(" | PIR: ");
        Serial.print(motion);
        Serial.print(" | LDR: ");
        Serial.print(ldrValue);
        Serial.print(isDay ? " (DAY)" : " (NIGHT)");
        Serial.print(" | State: ");
        switch (currentGateState)
        {
        case CLOSED:
            Serial.println("GATE_CLOSED");
            break;
        case NIGHT_DELAY:
            Serial.println("NIGHT_BEEP_DELAY (2s)");
            break;
        case OPEN:
            Serial.println("GATE_OPEN (3s)");
            break;
        case OVERSTAY_ALARM:
            Serial.println("OVERSTAY_ALARM");
            break;
        }
        lastLog = currentMillis;
    }

    // 4. Central Logic State Machine (Handles Constraints without collisions)
    switch (currentGateState)
    {

    case CLOSED:
        gateServo.write(0);          // Gate Closed
        digitalWrite(redLed, HIGH);  // Red LED -> ON
        digitalWrite(greenLed, LOW); // Green LED -> OFF
        noTone(buzzerPin);           // Mute Buzzer

        if (vehiclePresent)
        {
            // Condition: Vehicle blocks gate without triggering entry for > 5 seconds
            if (currentMillis - vehicleArrivalTime > 5000)
            {
                currentGateState = OVERSTAY_ALARM;
            }
            // Daytime Rule: Vehicle present (<20cm) AND motion detected
            else if (isDay && motion == HIGH)
            {
                gateTimer = currentMillis;
                currentGateState = OPEN;
            }
            // Nighttime Rule: Vehicle present (<20cm) -> Trigger 2-second alert delay
            else if (!isDay)
            {
                gateTimer = currentMillis;
                currentGateState = NIGHT_DELAY;
            }
        }
        break;

    case NIGHT_DELAY:
        // Condition: Sound warning tone for exactly 2 seconds BEFORE gate opens
        if (currentMillis - gateTimer < 2000)
        {
            tone(buzzerPin, 440); // 440Hz alert tone
        }
        else
        {
            noTone(buzzerPin);
            gateTimer = currentMillis; // Reset timer for the 3-second opening window
            currentGateState = OPEN;   // Proceed to open gate
        }

        // Safety exit if vehicle backs away during the countdown
        if (!vehiclePresent)
        {
            currentGateState = CLOSED;
        }
        break;

    case OPEN:
        gateServo.write(90);          // Gate Open
        digitalWrite(redLed, LOW);    // Red LED -> OFF
        digitalWrite(greenLed, HIGH); // Green LED -> ON
        noTone(buzzerPin);            // Ensure beep is off

        // Condition: Keep open for exactly 3 seconds, then close automatically
        if (currentMillis - gateTimer >= 3000)
        {
            // Once 3 seconds pass, check if vehicle has exceeded its total 5 seconds stay limit
            if (vehiclePresent && (currentMillis - vehicleArrivalTime > 5000))
            {
                currentGateState = OVERSTAY_ALARM;
            }
            else
            {
                currentGateState = CLOSED;
            }
        }
        break;

    case OVERSTAY_ALARM:
        tone(buzzerPin, 880); // Condition: Sound continuously until vehicle moves away
        gateServo.write(90);  // Keep gate open for safety while vehicle blocks path
        digitalWrite(greenLed, LOW);
        digitalWrite(redLed, HIGH); // Alert status indicators

        // Release system instantly once vehicle moves past 20cm away
        if (!vehiclePresent)
        {
            noTone(buzzerPin);
            currentGateState = CLOSED;
        }
        break;
    }
}

// Cleaned 4-pin Ultrasonic Driver Function
int readDistance()
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 25000);
    if (duration == 0)
        return 999;

    return duration * 0.034 / 2;
}
