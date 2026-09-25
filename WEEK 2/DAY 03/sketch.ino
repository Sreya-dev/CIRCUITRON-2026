#define BLYNK_TEMPLATE_ID "TMPL3k3svVwwW"
#define BLYNK_TEMPLATE_NAME "Smart Temp Monitor"
#define BLYNK_AUTH_TOKEN "jsa9ToD2x8PAu5nGOGFNc-p_lfa6AQ7W"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHTesp.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST"; // Wokwi virtual Wi-Fi
char pass[] = "";            // No password

DHTesp dht;
BlynkTimer timer;

const int DHT_PIN = 15;
const int FAN_PIN = 13;

void sendSensorData()
{
    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();

    if (isnan(humidity) || isnan(temperature))
    {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    // Send to Blynk Virtual Pins
    Blynk.virtualWrite(V0, temperature);
    Blynk.virtualWrite(V1, humidity);

    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print("°C | Humidity: ");
    Serial.print(humidity);
    Serial.println("%");

    // Fan Control Logic
    if (temperature > 30.0)
    {
        digitalWrite(FAN_PIN, HIGH);
        Blynk.virtualWrite(V2, "ON"); // Send fan status to Blynk
        Serial.println("Fan Status: ON");
    }
    else
    {
        digitalWrite(FAN_PIN, LOW);
        Blynk.virtualWrite(V2, "OFF"); // Send fan status to Blynk
        Serial.println("Fan Status: OFF");
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(FAN_PIN, OUTPUT);

    dht.setup(DHT_PIN, DHTesp::DHT22);
    Blynk.begin(auth, ssid, pass);

    // Read sensor every 2 seconds
    timer.setInterval(2000L, sendSensorData);
}

void loop()
{
    Blynk.run();
    timer.run();
}
