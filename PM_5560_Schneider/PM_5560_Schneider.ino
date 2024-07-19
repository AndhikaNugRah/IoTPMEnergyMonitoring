#include <PubSubClient.h>
#include <AsyncMqttClient.h>
#include "SimpleModbusMaster.h"
#include <WiFi.h>

#define baud 19200
#define timeout 1000
#define polling 200 // the scan rate
#define retry_count 10
#define TxEnablePin 0

AsyncMqttClient mqttClient;
const char* ssid = "Wifi-ID";
const char* password = "Input Password Here";
const char* mqtt_server = "MQTT Address"; 

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
unsigned long timer; 

enum {
  PACKET1,
  PACKET2,
  TOTAL_NO_OF_PACKETS
};

// Create an array of Packets for modbus_update()
// reconstruct the float from 2 unsigned integers

Packet packets[TOTAL_NO_OF_PACKETS];
packetPointer packet1 = &packets[PACKET1];
packetPointer packet2 = &packets[PACKET2];

unsigned int registers_ap_5560_1[2];
unsigned int registers_rp_5560_1[2];

float f_2uint_float(unsigned int uint1, unsigned int uint2) {    
  union f_2uint {
    float f;
    uint16_t i[2];
  };
  union f_2uint f_number;
  f_number.i[0] = uint1;
  f_number.i[1] = uint2;
  return f_number.f;
}

void setup() {
  Serial.begin(115200);
  packet1->id = 1;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = 2699; 
  packet1->no_of_registers = 2; 
  packet1->register_array = registers_ap_5560_1;
  
  packet2->id = 1;
  packet2->function = READ_HOLDING_REGISTERS;
  packet2->address = 2707; 
  packet2->no_of_registers = 2;
  packet2->register_array = registers_rp_5560_1;

  modbus_configure(baud, timeout, polling, retry_count,TxEnablePin, packets, TOTAL_NO_OF_PACKETS);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, 1883);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state()); 
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  unsigned int connection_status = modbus_update(packets);
  long newTimer = millis();
  if (newTimer - millis() >= 1000) { // Use millis() directly
   
    float AP_1_Modbus = f_2uint_float(registers_ap_5560_1[1],registers_ap_5560_1[0]);
    timer = newTimer;

    float RP_1_Modbus = f_2uint_float(registers_rp_5560_1[1],registers_rp_5560_1[0]);
    timer = newTimer;

    char ActivePower1[8];
    float AP_1_Converted = AP_1_Modbus/1000 ;
    dtostrf (AP_1_Converted , 1, 2, ActivePower1);
    Serial.print("Active Power - 5560 Unit 1 (MWH): ");
    Serial.println(ActivePower1);
    client.publish("ActivePower1", ActivePower1);

    char ReactivePower1[8];
    float RP_1_Converted = RP_1_Modbus/1000 ;  
    dtostrf(RP_1_Converted, 1, 2, ReactivePower1); 
    Serial.print("Reactive Power - 5560 Unit 1 (MVARH): ");   
    Serial.println(ReactivePower1); 
    client.publish("ReactivePower1", ReactivePower1);
    }

if (!client.connected()) {
    reconnect();
  }
  client.loop(); 
}