#include <Servo.h>

Servo doorServo;

const int SERVO_PIN = 9;
const int LED_PIN = 7;
const int MOTOR_IN1 = 5;
const int MOTOR_IN2 = 6;

String command = "";

void openDoor()
{
    doorServo.write(90);
    digitalWrite(LED_PIN, HIGH);
}

void closeDoor()
{
    doorServo.write(0);
    digitalWrite(LED_PIN, LOW);
}

void fanOn()
{
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
}

void fanOff()
{
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);
}

void setup()
{
    Serial.begin(9600);
    doorServo.attach(SERVO_PIN);
    pinMode(LED_PIN, OUTPUT);
    pinMode(MOTOR_IN1, OUTPUT);
    pinMode(MOTOR_IN2, OUTPUT);

    // Safe initial state
    closeDoor();
    fanOff();
    digitalWrite(LED_PIN, LOW);
}

void loop()
{
    if (Serial.available() > 0)
    {
        command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "OPEN")
        {
            openDoor();
        }
        else if (command == "CLOSE")
        {
            closeDoor();
        }
        else if (command == "FAN_ON")
        {
            fanOn();
        }
        else if (command == "FAN_OFF")
        {
            fanOff();
        }
    }
}