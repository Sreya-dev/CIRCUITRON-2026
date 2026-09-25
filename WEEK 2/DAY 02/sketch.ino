#define BLYNK_TEMPLATE_ID "TMPL3k3svVwwW"
#define BLYNK_TEMPLATE_NAME "LilyGo T PCIe 2"
#define BLYNK_AUTH_TOKEN "bQBunqCuUnUqX--c3AZCfXrHFOqp6DzS"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define LIGHT_PIN 23
#define STEP_PIN 14
#define DIR_PIN 12
#define EN_PIN 27

int fanRunning = 0;
int stepDelayUs = 1500;

void stepMotor()
{
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelayUs);
}

BLYNK_WRITE(V0)
{
    int lightState = param.asInt();
    digitalWrite(LIGHT_PIN, lightState);
    Serial.println(lightState ? "Room Light: ON" : "Room Light: OFF");
}

BLYNK_WRITE(V1)
{
    int fanState = param.asInt();
    if (fanState == 1)
    {
        fanRunning = 1;
        digitalWrite(EN_PIN, LOW);
        digitalWrite(DIR_PIN, HIGH);
        Serial.println("Room Fan: ON");
    }
    else
    {
        fanRunning = 0;
        digitalWrite(EN_PIN, HIGH);
        Serial.println("Room Fan: OFF");
    }
}

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("\n--- [START] Serial Monitor Active ---");
    Serial.println("Initializing pins...");

    pinMode(LIGHT_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(EN_PIN, OUTPUT);

    digitalWrite(LIGHT_PIN, LOW);
    digitalWrite(STEP_PIN, LOW);
    digitalWrite(DIR_PIN, HIGH);
    digitalWrite(EN_PIN, HIGH);

    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, pass);

    // Asynchronous Blynk configuration to prevent freezing
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
    Serial.println("Blynk configuration loaded. Running loop...");
}

void loop()
{
    // Only process cloud updates if network connection is established
    if (WiFi.status() == WL_CONNECTED)
    {
        Blynk.run();
    }

    if (fanRunning == 1)
    {
        stepMotor();
    }
}
