#include <Servo.h>

Servo myServo;

const int GREEN = 4;
const int YELLOW = 5;
const int RED = 6;
const int BUZZER = 7;
const int SERVO = 9;

void allOff()
{
    digitalWrite(GREEN, LOW);
    digitalWrite(YELLOW, LOW);
    digitalWrite(RED, LOW);
    digitalWrite(BUZZER, LOW);
    myServo.write(0);
}

void setup()
{
    Serial.begin(4800);
    myServo.attach(SERVO);
    pinMode(GREEN, OUTPUT);
    pinMode(YELLOW, OUTPUT);
    pinMode(RED, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    allOff();
    Serial.println("=== Slave 2 Ready ===");
}

void loop()
{
    if (Serial.available())
    {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd.length() == 0)
            return;

        allOff();
        Serial.print("CMD: ");
        Serial.println(cmd);

        if (cmd == "LOW")
        {
            digitalWrite(GREEN, HIGH);
            Serial.println("Green LED ON");
        }
        else if (cmd == "MEDIUM")
        {
            digitalWrite(YELLOW, HIGH);
            Serial.println("Yellow LED ON");
        }
        else if (cmd == "HIGH")
        {
            digitalWrite(RED, HIGH);
            digitalWrite(BUZZER, HIGH);
            Serial.println("Red LED + Buzzer ON");
        }
        else if (cmd == "CRITICAL")
        {
            digitalWrite(RED, HIGH);
            digitalWrite(BUZZER, HIGH);
            myServo.write(90);
            Serial.println("Red + Buzzer + Servo 90");
        }
    }
}