#include "CloudConnectMQTT.h"



// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "www"; //APN for Vodafone
//const char apn[]  = "airtelgprs.com"; //APN for Airtel
const char user[] = "";
const char pass[] = "";

// MQTT details

//Test Site
//char* topic = "channels/1501274/publish/RE8E1QGR8L5ITW15";

                                    
char* mqttserver = "mqtt.thingspeak.com";
const char* broker = "mqtt.thingspeak.com";
String clientName = "sabudhFarms";

TinyGsm modem(SerialAT);

TinyGsmClient client(modem);
PubSubClient mqtt(mqttserver,1883,client);
bool networkConnected = false;

long lastReconnectAttempt = 0;

bool mqttConnect(float lum,float temperature, float pressure, float humid){
  String payload="&field1="+ String(lum)+ "&field2=" + String(temperature) +
  "&field3=" + String(pressure)+"&field4=" + String(humid)+"&status=MQTTPUBLISH";


  
  
 
  SerialMon.print("Connecting to server");
  //SerialMon.print(broker);

  // Connect to MQTT Broker
  boolean status = mqtt.connect((char*) clientName.c_str());

  // Or, if you want to authenticate MQTT:
  //boolean status = mqtt.connect("GsmClientName", "mqtt_user", "mqtt_pass");

  if (status == false) {
    SerialMon.println(" fail");
    networkConnected = false;
    return false;
  }
  SerialMon.println(" OK");
  mqtt.publish((char*) topic.c_str(), (char*) payload.c_str());
  SerialMon.println(payload);
  
  //Resetting all Variables
  
  return mqtt.connected();
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  SerialMon.print("Message arrived [");
  SerialMon.print(topic);
  SerialMon.print("]: ");
  SerialMon.write(payload, len);
  SerialMon.println();

  
  
}

void publishData(float lum,float temperature,float pressure,float humid)
{       
  if (!mqtt.connected()) {
    SerialMon.println("=== MQTT NOT CONNECTED ===");
    // Reconnect every 10 seconds
    unsigned long t = millis();
    if (t - lastReconnectAttempt > 10000L) {
      lastReconnectAttempt = t;
      //--------------Bug Found-------------- To Be Tested
      if (mqttConnect(lum,temperature,pressure,humid)){
        lastReconnectAttempt = 0;
      }
    }
    delay(10);
    return;
  }
  else
  {
    String payload="&field1="+ String(lum)+ "&field2=" + String(temperature) +
  "&field3=" + String(pressure)+"&field4=" + String(humid)+"&status=MQTTPUBLISH";
    
      
    SerialMon.println(" OK");
    mqtt.publish(topic.c_str(), (char*) payload.c_str());
    SerialMon.println(payload);
  }
  
}
