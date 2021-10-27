#include "arduino.h"

#define TINY_GSM_MODEM_SIM800

#define TINY_GSM_DEBUG SerialMon

// Range to attempt to autobaud
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 38400

// Add a reception delay, if needed
#define TINY_GSM_YIELD() { delay(2); }

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
#define SerialAT Serial1

#include <TinyGsmClient.h>
#include <PubSubClient.h>

#define RXD2 16
#define TXD2 17

// Your GPRS credentials
// Leave empty, if missing user or pass
extern const char apn[] ;
extern const char user[];
extern const char pass[];

// MQTT details
extern String topic ; 
extern char* mqttserver ;
extern const char* broker ;
extern String clientName ;

extern TinyGsm modem;
extern TinyGsmClient client;
extern PubSubClient mqtt;
extern bool networkConnected ;

void mqttCallback(char* topic, byte* payload, unsigned int len);
bool mqttConnect(float lum,float temperature, float pressure,float humid);
void publishData(float lum,float temperature, float pressure,float humid);
