#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <WebSocketClient.h>


char path[] = "/";
char host[] = "soylatte.kr";

SoftwareSerial HM10(D6, D5); // RX, TX
char buf[700];
String input;
String sending = "";
boolean isCallDISC = true;

WebSocketClient webSocketClient;
WiFiClient client;

void setup() {
  Serial.begin(115200);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Connect to the websocket server
  if (client.connect("soylatte.kr", 3000)) {
    Serial.println("Connected");
  } else {
    Serial.println("Connection failed.");
    while(1) {
      // Hang on failure
    }
  }

  // Handshake with the server
  webSocketClient.path = path;
  webSocketClient.host = host;
  if (webSocketClient.handshake(client)) {
    Serial.println("Handshake successful");
  } else {
    Serial.println("Handshake failed.");
    while(1) {
      // Hang on failure
    }  
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  Serial.begin(9600);
  HM10.begin(9600);
}

void loop() { // run over and over
  if(isCallDISC) {
    delay(10);
    HM10.print("AT+DISC?"); 
    isCallDISC = false;
  }
  if (HM10.available()) {
    input += HM10.readString();
    int idx = input.indexOf("OK+DISCE");
    if(idx > -1) {
      String nowInput = input.substring(0, idx + 8);
      input = input.substring(idx + 9, input.length());
      int newLineIdx = 0;
      while(newLineIdx > -1) {
        newLineIdx = nowInput.indexOf("\n");
        String lineInput = nowInput.substring(0, newLineIdx);

        int oknameIdx = lineInput.indexOf("OK+NAME");
        if(oknameIdx > -1) {
          String sepInput = lineInput.substring(oknameIdx, lineInput.length());
          String value = sepInput.substring(sepInput.indexOf(":") + 1, sepInput.length());
          value.trim();
          if(value.indexOf("ZZIPPU") > -1) {
            sending +=  "," + value;
          }
        }
        
        nowInput = nowInput.substring(newLineIdx + 1, nowInput.length());
      }
      sending = sending.substring(1, sending.length());
      Serial.print("send: ");
      Serial.println(sending);

      
//      WiFiClient client;
//      const int port = 3000;
//      if (client.connect("soylatte.kr", port)) {
//        client.print(sending);
//      } else {
//        Serial.println("connection failed");
//      }

      if (client.connected()) {
        webSocketClient.sendData(sending);
      } 
  
      sending = "";

      isCallDISC = true;
    }
  }
}
