// LED pin connections
const int redPin = 12;
const int yellowPin = 11;
const int greenPin = 10;

void setup()
{
    // Initialize digital pins as outputs
    pinMode(redPin, OUTPUT);
    pinMode(yellowPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
}

void loop()
{
    // 1. Red signal active for 3 seconds
    digitalWrite(redPin, HIGH);
    digitalWrite(yellowPin, LOW);
    digitalWrite(greenPin, LOW);
    delay(3000);

    // 2. Yellow signal active for 1 second
    digitalWrite(redPin, LOW);
    digitalWrite(yellowPin, HIGH);
    digitalWrite(greenPin, LOW);
    delay(1000);

    // 3. Green signal active for 3 seconds
    digitalWrite(redPin, LOW);
    digitalWrite(yellowPin, LOW);
    digitalWrite(greenPin, HIGH);
    delay(3000);
}
