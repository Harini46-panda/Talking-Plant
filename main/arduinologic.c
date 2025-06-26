#include <DHT.h>


#define PIR_GPIO 17       
#define DHTPIN 21       
#define GASPIN 14         


#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);  

void setup() {
  Serial.begin(115200);
  delay(1000);  
  
  pinMode(PIR_GPIO, INPUT);
  Serial.println("[PIR_SENSOR] Setup complete");

  
  dht.begin();
  Serial.println("[DHT_SENSOR] Setup complete");

  
  pinMode(GASPIN, INPUT);
  Serial.println("[GAS_SENSOR] Setup complete");
}

void loop() {
  
  int pir_value = digitalRead(PIR_GPIO);
  if (pir_value == HIGH) {
    Serial.println("[PIR_SENSOR] Motion Detected: Who could it be?");
  } else {
    Serial.println("[PIR_SENSOR] No Motion");
  }

  
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("[DHT_SENSOR] Failed to read from sensor!");
  } else {
    Serial.print("[DHT_SENSOR] Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C | Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  
  int gasValue = analogRead(GASPIN);  
  Serial.print("[GAS_SENSOR] Gas Level: ");
  Serial.println(gasValue);

  Serial.println("-----------------------------------");
  delay(1000); 
}
