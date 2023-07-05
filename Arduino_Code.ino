int analogInput = 0;
float Vout = 0.00;
float Vin = 0.00;
float R1 = 10000.00; // resistance of R1 (10K) 
float R2 = 1000.00; // resistance of R2 (1K) 
int val = 0;
float Vacross,Iamp = 0;
const int Shunt_Res = 1000; //for 1k resistor

byte statusLed    = 13;

byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;

float calibrationFactor = 4.5;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;

//Arduino to NodeMCU Lib
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

//Initialise Arduino to NodeMCU (5=Rx & 6=Tx)
SoftwareSerial nodemcu(5, 6);

void setup(){
   pinMode(analogInput, INPUT); //assigning the input port
   Serial.begin(115200); //BaudRate
   nodemcu.begin(115200);
  delay(1000);

  Serial.println("Program started");

  // Set up the status LED line as an output
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}

void loop(){
   
   val = analogRead(analogInput);//reads the analog input
   Vout = (val * 5.00) / 1024.00; // formula for calculating voltage out i.e. V+, here 5.00
   Vin = Vout / (R2/(R1+R2)); // formula for calculating voltage in i.e. GND
   if (Vin<0.8)//condition 
   {
     Vin=0.00;//statement to quash undesired reading !
  } 

  Vacross = analogRead(A1);   
  Vacross = (Vacross * 5.0) / 1024.0;
  Iamp = (Vacross * 1000) / Shunt_Res;

  float power = Iamp*Vin;
 
  Serial.print("Voltage = ");
  Serial.print(Vin);
  Serial.println("V");
  Serial.print("Current = ");
  Serial.print(Iamp);
  Serial.println("mA");
  Serial.print("Power = ");
  Serial.print(power);
  Serial.println("mW");

if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
   
  // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(flowRate);
    Serial.println("L/min");
    
    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");        
    Serial.print(totalMilliLitres);
    Serial.print("mL"); 
    Serial.print("\t");       // Print tab space
  Serial.print(totalMilliLitres/1000);
  Serial.println("L");
  Serial.println("-----------------------------------------");

  // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}
      
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  
  data["Voltage"] = Vin;
  data["Current"] = Iamp;
  data["Power"] = power;
  data["Flow rate"] = flowRate;
  data["Volume"] = totalMilliLitres;
  

  //Send data to NodeMCU
  data.printTo(nodemcu);
  jsonBuffer.clear();
  delay(1000);
}

void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
