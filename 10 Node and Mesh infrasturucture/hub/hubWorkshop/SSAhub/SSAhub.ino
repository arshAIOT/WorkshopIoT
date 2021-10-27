#include "namedMesh.h"
#include "constant.h"
#include "CloudConnectMQTT.h"
#include "esp_system.h"
#include <StringTokenizer.h>

#define DEBUG true

#define   MESH_SSID       "sabudhNodeNetwork"
#define   MESH_PASSWORD   "qwerty123456"
#define   MESH_PORT       5555

TaskHandle_t GsmTask;

hw_timer_t *timer = NULL;

long failSafeOPT = millis();
void IRAM_ATTR resetModule(){
    ets_printf("Fail Safe Trig Period %ld",(millis()-failSafeOPT));
    ets_printf("Fail Safe reboot\n");
    ESP.restart();
}

Scheduler     userScheduler; // to control your personal task
namedMesh  mesh;

String nodeName = "hub"; // Name needs to be unique

bool node_link =false;
int lastInvoked = millis();
long lastDataPublished = millis();

short int loggedData[1440] = {0}; 

bool pushData = false;

String channelID = "";
String writeAPI = "";
float lum = 0.0;
float temperature = 0.0;
float pressure = 0.0;
float humid = 0.0;



String topic ;



void receivedCallback( uint32_t from, String &msg ) {
  timerWrite(timer, 0);
  if(DEBUG)
    Serial.printf("Received from Node %u msg=%s\n", from, msg.c_str()); 
    Serial.printf("Heap Remaining = %d\n",ESP.getFreeHeap());

    int tkn = 0;
    float params[4] ={0.0};
    StringTokenizer tokens(msg, ",");
    channelID = tokens.nextToken();
    writeAPI = tokens.nextToken();
    
    topic = "channels/"+String(channelID)+"/publish/"+String(writeAPI);
    
    while(tokens.hasNext())
     {     
      params[tkn++] = tokens.nextToken().toFloat();

      if(DEBUG) 
      {
        Serial.print(params[tkn - 1] );
        Serial.println("   ");
      }
     }

     lum = params[0];
     temperature = params[1];
     pressure = params[2];
     humid = params[3];
     

     if(DEBUG)     
      Serial.println(channelID);
      Serial.println(writeAPI);
      Serial.println(topic);
}


void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}



void setup() {
  SerialMon.begin(115200);
  // Set GSM module baud rate
  SerialAT.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(3000);

  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name! 
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);

  mesh.onReceive(&receivedCallback);
  
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  timer = timerBegin(0, FAILSAFETIMER_PRESCALER, true); //timer 0, div 80
  timerAttachInterrupt(timer, &resetModule, true);
  timerAlarmWrite(timer, FAILSAFETIMER_INTERVAL, false); //set time in us
  timerAlarmEnable(timer); //enable interrupt
  failSafeOPT = millis();

  //GSM Task  
    xTaskCreatePinnedToCore(
      GsmHandler, /* Function to implement the task */
      "GsmTask", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      2,  /* Priority of the task */
      &GsmTask,  /* Task handle. */
      0); /* Core where the task should run */  
}


void loop() {
  // it will run the user scheduler as well
  mesh.update();
}


//Handles Cloud Connectivity and Data push
void GsmHandler( void * parameter) {
  for(;;) 
  {
    if(networkConnected == false)
    {
      SerialMon.println("Initializing modem...");
      modem.restart();
      // modem.init();
    
      String modemInfo = modem.getModemInfo();
      SerialMon.print("Modem: ");
      SerialMon.println(modemInfo);
    
      SerialMon.print("Waiting for network...");
      if (!modem.waitForNetwork(10000L)) {
        SerialMon.println(" fail");
        //delay(10000);
        //return;
      }
      SerialMon.println(" OK");
    
       if (modem.isNetworkConnected()) {
        SerialMon.println("Network connected");
        networkConnected = true;
      }
    
      if(networkConnected)
      {
        //#if TINY_GSM_USE_GPRS && defined TINY_GSM_MODEM_HAS_GPRS
        SerialMon.print(F("Connecting to "));
        SerialMon.print(apn);
        if (!modem.gprsConnect(apn, user, pass)) {
         SerialMon.println(" fail");
         delay(10000);
         continue;
         //return;
        }
        SerialMon.println(" OK");
        //#endif
        // MQTT Broker setup
        mqtt.setServer(broker, 1883);
        mqtt.setCallback(mqttCallback);

        //Upload Initial State
        publishData(lum, temperature,pressure,humid);
      }
    }

    if((millis()-lastDataPublished) >= CLOUDUPDATEINTERVAL)
      pushData = true;
      
    //Update data to cloud periodically  
    if(pushData  && networkConnected)
    {
      pushData = false;
      Serial.println("Attempted Data Push");
      lastDataPublished = millis();        
      publishData(lum,temperature,pressure,humid);         
      Serial.println("Data Push Ok");
    }
    delay(1000);
  }// Infinite for Loop End
}
