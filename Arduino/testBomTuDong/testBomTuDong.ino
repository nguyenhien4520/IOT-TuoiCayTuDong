#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>

WebSocketsClient webSocket;
#define CAMBIEN_PIN A0   //chân độ ẩm đất
#define RELAY_PIN 5 

const char* ssid = "GWC";
const char* password = "01234567";
const char* ip_host = "192.168.1.14";
const uint16_t port = 3000;

int h = 0;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 2000; // Gửi dữ liệu mỗi 1000 ms (1 giây)
int autoState=0;
int onState=0;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED:
      {
        Serial.printf("[WSc] Connected to url: %s\n", payload);
      }
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get text: %s\n", payload);
      if (strcmp((char*)payload, "MANUAL") == 0) {
        autoState=0;
      }else if (strcmp((char*)payload, "AUTO") == 0) {
        autoState=1;
      }else if (strcmp((char*)payload, "START") == 0){
        onState=1;
      }else if (strcmp((char*)payload, "STOP") == 0){
        onState=0;
      }
      break;
    //case WStype_BIN:
      //Serial.printf("[WSc] get binary length: %u\n", length);
      //break;
  }
}

void sendSensor() {
  unsigned long currentTime = millis();
  if (currentTime - lastSendTime >= sendInterval) {
    // Gửi giá trị lên WebSocket
    // String message = String(h);
    // webSocket.sendTXT(message);
    int hu = analogRead(CAMBIEN_PIN);
    h = map(hu, 0, 1023, 100, 0);

    if (isnan(h)) {
    Serial.println("Failed to read from sensor!");
    return;
  }
    Serial.println(h);
    String message = String(h);
    webSocket.sendTXT(message);
    // Cập nhật thời gian lần cuối gửi
    lastSendTime = currentTime;
  }
  // Serial.println(h);
}
void handle(){
  if(autoState==0){
    if(onState==1){
      digitalWrite(RELAY_PIN,HIGH);
    }else if(onState==0){
      digitalWrite(RELAY_PIN,LOW); 
    }
  }else if(autoState==1){
    if(h>=15){
      digitalWrite(RELAY_PIN,LOW); 
    }else{
        digitalWrite(RELAY_PIN,HIGH);
    }
  }
}
void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  webSocket.begin(ip_host, port);
  webSocket.onEvent(webSocketEvent);
  pinMode(CAMBIEN_PIN,INPUT);
  pinMode(RELAY_PIN,OUTPUT);
}

void loop() {
  webSocket.loop();
  sendSensor();
  handle();
}
