#include <Arduino.h>
#include <DHT.h>

#define DHT_SENSOR_PIN 4
#define DHT_SENSOR_TYPE DHT11

DHT dhtSensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

void setup()
{
  Serial.begin(115200);
  dhtSensor.begin();
  delay(2000);
}

void loop()
{
  float humidity = dhtSensor.readHumidity();
  float tempC = dhtSensor.readTemperature();
  float tempF = dhtSensor.readTemperature(true);

  if (isnan(tempC) || isnan(tempF) || isnan(humidity))
  {
    Serial.println("Failed to read from DHT sensor!");
  }
  else
  {
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("%");
    Serial.print("  |  ");
    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.print("°C  ~  ");
    Serial.print(tempF);
    Serial.println("°F");
  }

  delay(2000);
}
