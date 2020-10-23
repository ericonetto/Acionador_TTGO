#include <Arduino.h>

// Replace the next variables with your SSID/Password combination
char* ssid = "Popo";
char* password = "batatinha1235";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "mqtt.prod.konkerlabs.net";

const char* mqtt_device_user = "63o295ahqu6i";
const char* mqtt_device_pass= "7kvMBBeVqCSz";

const String topico_pub = "data/63o295ahqu6i/pub/";
const String topico_sub = "data/63o295ahqu6i/sub/";

const int mqtt_port = 1883;