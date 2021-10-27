#include "namedMesh.h"

//for BME280
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C

int sensorPin = 36;


//mesh
#define   MESH_SSID       "sabudhNodeNetwork"
#define   MESH_PASSWORD   "qwerty123456"
#define   MESH_PORT       5555


// initials to be stored in eeprom
String  CHANNEL_ID = "1549625";
String  WRITE_API = "80L044TEYCJLTLZE";

// mesh 
Scheduler     userScheduler; // to control your personal task 
namedMesh  mesh;

String nodeName = "SensorNodePrime"; // name of this node


void setup(){
  
  Serial.begin(115200);
  
  sensorSetup();
    delay(3000);
  meshSetup(); 
}


void loop() {
  // it will run the user scheduler as well
  mesh.update();
}



//task
Task taskSendMessage( TASK_SECOND*30, TASK_FOREVER, []() {
  String msg = CHANNEL_ID + String(",") + WRITE_API + String(",") + readLum() + String(",")+ readTemperatureBME() + String(",")+ readPressureBME() + String(",")+ readHumidityBME(); 
  

  
  String to = "hub"; // name of the node to which we are sending message..
  mesh.sendSingle(to, msg); 
  
  bool val = mesh.sendSingle(to, msg);
  Serial.println(val);
}); // start with a one second interval




void meshSetup(){

  Serial.println("Mesh Setup");

  
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name! 

  mesh.setRoot(false);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);

  mesh.onReceive([](uint32_t from, String &msg) {
    Serial.printf("Received message by id from: %u, %s\n", from, msg.c_str());
  });

  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
  });

  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}




void sensorSetup() 
{
  bool status;
  
  status = bme.begin(0x76);// address 0*76 passed to bme.begin method to start communication with bme sensor //  This function initializes I2C interface with given I2C Address and checks if the chip ID is correct. It then resets the chip using soft-reset & waits for the sensor for calibration after wake-up.
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  
  Serial.println("-- BME 280 setup done--");

  Serial.println();
  delay(100);
}


//reading temperture
float readTemperatureBME() 
{
  
  float tempValue = bme.readTemperature();  
  return tempValue;
}


//reading pressure
float readPressureBME() 
{
  float pressureValue = bme.readPressure() / 100.0F;  
  return pressureValue;
}


//reading humidity
float readHumidityBME() 
{
  float humidityValue = 0;
  for(int i = 0; i<= 50; i++){
    humidityValue += bme.readHumidity();
  }
  humidityValue = humidityValue/50;
  return humidityValue;
}

float readLum() 
{
  float lumValue = analogRead(sensorPin);  
  return lumValue;
}
