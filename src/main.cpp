#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> 
#include "./ttgo.h"

#include "./cloudconn.h"

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
int rele = 4;
char bufferJ[256];

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin("Popo", "batatinha1235");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_device_user, mqtt_device_user, mqtt_device_pass)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("data/63o295ahqu6i/sub/command");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void initCloud(void mqttReceiveCallback(char* topic, byte* message, unsigned int length)){
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttReceiveCallback);
}




void gatewayCommand(char* command){
    const int capacity = JSON_OBJECT_SIZE(3);

   StaticJsonDocument<capacity> jsonMSG;
   DeserializationError err = deserializeJson(jsonMSG, command);
   if (err)
   {
     Serial.println("ERRO.");
     return;
   }
   if (jsonMSG.containsKey("value")) 
   { 

     Serial.println(jsonMSG["value"].as<char*>());

    if (strcmp("seco",jsonMSG["value"].as<char*>())==0){
      digitalWrite(rele, HIGH);   
      delay(1000);                       // wait for a second
      digitalWrite(rele, LOW);    
      delay(1000);                       // wait for a second
      digitalWrite(rele, HIGH);   
      delay(1000);                       // wait for a second
      digitalWrite(rele, LOW);    
      delay(1000);                       // wait for a second
      digitalWrite(rele, HIGH);   
      delay(1000);                       // wait for a second
      digitalWrite(rele, LOW);    
      delay(1000);                       // wait for a second
    }
     }  

}

void mqttReceive(char* topic, byte* message, unsigned int length){
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  char messageTemp[length];
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp[i] = (char)message[i];
  }
  Serial.println();

  if(strcmp(topic,"data/63o295ahqu6i/sub/command")==0){
    gatewayCommand(messageTemp);
  }

}


void initDisplay(){
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW); // low to reset OLED
    delay(50);
    digitalWrite(OLED_RST, HIGH); // must be high to turn on OLED

    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
}

void initLora()
{

  initDisplay();

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init())
  {
    Serial.println("LoRa radio init failed");
    while (1)
      ;
  }
  Serial.println("LoRa radio init OK!");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ))
  {
    Serial.println("setFrequency failed");
    while (1)
      ;
  }
  Serial.print("Set Freq to: ");
  Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

void sendLora(char* msg){
  const size_t STR_LEN = sizeof(msg) - 1;

  if(STR_LEN> 251){
    char msgtrunc[251];
    memcpy ( msgtrunc, msg, 251 );
    rf95.send((uint8_t *)msgtrunc, 251);
    rf95.waitPacketSent();
  }else{
    rf95.send((uint8_t *)msg, STR_LEN);
    rf95.waitPacketSent();
  }


}

void receiveLora(void (*actionLoraCallback)(char* msg)){

    if (rf95.available())
    {
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);

        if (rf95.recv(buf, &len))
        {

            char message[len];
            memcpy(message,buf,len);
            message[len]='\0';

            
            actionLoraCallback(message);

        }
    }
}


char *jsonMQTTmsgDATA(const char *device_id, const char *metric, const char * value) {
	const int capacity = JSON_OBJECT_SIZE(3);
	StaticJsonDocument<capacity> jsonMSG;
	jsonMSG["deviceId"] = device_id;
	jsonMSG["metric"] = metric;
	jsonMSG["value"] = value;
	serializeJson(jsonMSG, bufferJ);
	return bufferJ;
}


void actionLora(char* msg){


  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, msg);
  display.display();

  Serial.println("");
  Serial.println("Received lora message: " + (String)msg);

  char* char_topic;
  String topic = topico_pub + "umidade";
  topic.toCharArray(char_topic, topic.length());  

  char *mensagem;
  mensagem = jsonMQTTmsgDATA("lavoura", "", msg);
  client.publish("data/63o295ahqu6i/pub/umidade", mensagem); 
}




// the setup function runs once when you press reset or power the board
void setup() {

  Serial.begin(9600);
  Serial.println("Setup");

  initLora();

  initCloud(mqttReceive);

  pinMode(rele, OUTPUT);

  Serial.println("Setup finished");
}


// the loop function runs over and over again forever
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();



  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    receiveLora(actionLora);
  }


}