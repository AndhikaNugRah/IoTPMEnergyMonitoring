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
const char* mqtt_server = "MQTT server";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// Uncomment the line below if you intend to use a timer variable
unsigned long timer; 

enum {
  PACKET1,
  PACKET2,
  TOTAL_NO_OF_PACKETS
};

// Create an array of Packets for modbus_update()
Packet packets[TOTAL_NO_OF_PACKETS];
packetPointer packet1 = &packets[PACKET1];
packetPointer packet2 = &packets[PACKET2];

unsigned int registers_ap_5350[4]; // Array to store all 4 register values
unsigned int registers_rp_5350[4];

float f_2uint_float(unsigned int uint1, unsigned int uint2) {    
  // reconstruct the float from 2 unsigned integers
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

  packet1->id = 42;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = 3204;
  packet1->no_of_registers = 4; // Read 4 registers at once
  packet1->register_array = registers_ap_5350;

  packet2->id = 42;
  packet2->function = READ_HOLDING_REGISTERS;
  packet2->address = 3220;
  packet2->no_of_registers = 4; // Read 4 registers at once
  packet2->register_array = registers_rp_5350;


  modbus_configure(baud, timeout, polling, retry_count, TxEnablePin, packets, TOTAL_NO_OF_PACKETS);

  // Use millis() directly if timer is not needed
  // timer = millis();

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
      // Subscribe
      client.subscribe("esp32/output");
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
  double finalValue=0; 
  unsigned int connection_status = modbus_update(packets);
  long newTimer = millis();
  if (newTimer - millis() >= 1000) { // Use millis() directly

    // Combine registers into a single INT64 value
    int64_t combinedValueAP = 0;
    combinedValueAP |= ((int64_t)registers_ap_5350[1] << 16);
    double finalValueAP=double(combinedValueAP)/1000000;//MWH
    // Print the combined value
    char formattedValueAP[16]; // Adjust size based on your expected value range
    dtostrf(finalValueAP, 6, 2, formattedValueAP); // Format with 6 total digits, 2 decimals
    Serial.print("Combined value (MWH): ");
    Serial.println(formattedValueAP); 
    client.publish("ActivePower_5350", formattedValueAP);

    int64_t combinedValueRP = 0;
    combinedValueRP |= ((int64_t)registers_rp_5350[1] << 16);
    double finalValueRP=double(combinedValueRP)/1000000;//MWH
    // Print the combined value
    char formattedValueRP[16]; // Adjust size based on your expected value range
    dtostrf(finalValueRP, 6, 2, formattedValueRP); // Format with 6 total digits, 2 decimals
    Serial.print("Combined value (KVARH): ");
    Serial.println(formattedValueRP); 
    client.publish("ReactivePower_5350", formattedValueRP);
   }
  client.loop();
} 