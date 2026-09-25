// Pin Definitions
const int btn1Pin = 2;
const int btn2Pin = 3;
const int btn3Pin = 4;
const int buzzerPin = 8;
const int greenLedPin = 9;
const int redLedPin = 10;

// Password Configuration (Button 2 -> Button 1 -> Button 3)
const int password[3] = {2, 1, 3};
int inputSequence[3];
int currentIndex = 0;

// Debounce Tracking Arrays
const unsigned long debounceDelay = 50;
int buttonState[11] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
int lastButtonState[11] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
unsigned long lastDebounceTime[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void setup()
{
    Serial.begin(9600);

    pinMode(btn1Pin, INPUT);
    pinMode(btn2Pin, INPUT);
    pinMode(btn3Pin, INPUT);

    pinMode(buzzerPin, OUTPUT);
    pinMode(greenLedPin, OUTPUT);
    pinMode(redLedPin, OUTPUT);

    noTone(buzzerPin);
    Serial.println("System Ready. Enter Password...");
}

void loop()
{
    checkButton(btn1Pin, 1);
    checkButton(btn2Pin, 2);
    checkButton(btn3Pin, 3);
}

void checkButton(int pin, int buttonNumber)
{
    int reading = digitalRead(pin);

    if (reading != lastButtonState[pin])
    {
        lastDebounceTime[pin] = millis();
    }

    if ((millis() - lastDebounceTime[pin]) > debounceDelay)
    {
        if (reading != buttonState[pin])
        {
            buttonState[pin] = reading;

            if (buttonState[pin] == HIGH)
            {
                inputSequence[currentIndex] = buttonNumber;
                currentIndex++;

                Serial.print("Button ");
                Serial.print(buttonNumber);
                Serial.println(" registered.");

                tone(buzzerPin, 800, 50); // Short click sound for feedback

                if (currentIndex == 3)
                {
                    verifyPassword();
                }
            }
        }
    }
    lastButtonState[pin] = reading;
}

void verifyPassword()
{
    bool correct = true;
    for (int i = 0; i < 3; i++)
    {
        if (inputSequence[i] != password[i])
        {
            correct = false;
            break;
        }
    }

    if (correct)
    {
        Serial.println("ACCESS GRANTED");
        digitalWrite(greenLedPin, HIGH);
        tone(buzzerPin, 1200, 500);
        delay(1500);
        digitalWrite(greenLedPin, LOW);
    }
    else
    {
        Serial.println("ACCESS DENIED");
        digitalWrite(redLedPin, HIGH);
        tone(buzzerPin, 300, 500);
        delay(1500);
        digitalWrite(redLedPin, LOW);
    }

    resetSystem();
}

void resetSystem()
{
    currentIndex = 0;
    Serial.println("-------------------------");
    Serial.println("System Ready. Enter Password...");
}