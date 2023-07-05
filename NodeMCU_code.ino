//ThatsEngineering
//Sending Data from Arduino to NodeMCU Via Serial Communication
//NodeMCU code

//Include Lib for Arduino to Nodemcu
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

SoftwareSerial nodemcu(12, 14);

const int trigPin = 2;
const int echoPin = 0;
#include "Ubidots.h"
const char* UBIDOTS_TOKEN = "BBFF-uF0jDQHIR87OkFexLhkDujzuBQCvIJ";  // Put here your Ubidots TOKEN
const char* WIFI_SSID = "Semo";      // Put here your Wi-Fi SSID
const char* WIFI_PASS = "semo5704";      // Put here your Wi-Fi password
Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);

//define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034

long duration;
float distanceCm;

void setup() {
  // Initialize Serial port
  Serial.begin(115200);
  nodemcu.begin(115200);
  while (!Serial) continue;
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
}

void loop() {
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(nodemcu);

  if (data == JsonObject::invalid()) {
    //Serial.println("Invalid Json Object");
    jsonBuffer.clear();
    return;
  }
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_VELOCITY/2;

  float height = 50 - distanceCm;
  float vol = 380.13 * height;

  Serial.println("JSON Object Recieved");
  Serial.print("Voltage:  ");
  float Vin = data["Voltage"];
  Serial.print(Vin);
  Serial.println("V");
  Serial.print("Current:  ");
  float Iamp = data["Current"];
  Serial.print(Iamp);
  Serial.println("mA");
  Serial.print("Power:  ");
  float power = data["Power"];
  Serial.print(power);
  Serial.println("mW");
  Serial.print("Flow rate: ");
  float flowRate = data["Flow rate"];
    Serial.print(flowRate);
    Serial.println("L/min");
    Serial.print("Output Liquid Quantity: "); 
    float totalMilliLitres = data["Volume"];
    Serial.print(totalMilliLitres);
    Serial.print("mL"); 
    Serial.print("\t");       // Print tab space
  Serial.print(totalMilliLitres/1000);
  Serial.println("L");
  Serial.print("Total Volume: ");
  Serial.print(vol);
  Serial.print("mL");
  Serial.print("\t");       // Print tab space
  Serial.print(vol/1000);
  Serial.println("L");
  Serial.println("-----------------------------------------");

  ubidots.add("Voltage", Vin);
  ubidots.add("Current", Iamp);
  ubidots.add("Power", power);
  ubidots.add("Flow rate", flowRate);
  ubidots.add("Volume mL", totalMilliLitres);
  ubidots.add("Volume L", totalMilliLitres/1000);
  ubidots.add("Total Volume mL", vol);
  ubidots.add("Total Volume L", vol/1000);
  bool bufferSent = false;
  bufferSent = ubidots.send();  // Will send data to a device label that matches the device Id

  if (bufferSent = true) {
    // Do something if values were sent properly
    Serial.println("Values sent by the device");
  }
}
