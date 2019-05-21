#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

SoftwareSerial mySerial(13, 15);
void Split(int, String, char, String *, String *);

String slave_list[] = {"1", "2", "3"};
int slave_list_size = sizeof(slave_list) / sizeof(slave_list[0]);

const char *ssid = "zzeororo";  //ENTER YOUR WIFI SETTINGS
const char *password = "cjfghq1a1f!@";

//Web/Server address to read/write from 
const char *host = "192.168.1.2";   


void setup() {
  delay(1000);
  Serial.begin(115200);
  mySerial.begin(9600);

  WiFi.mode(WIFI_OFF);            //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);            //This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}


  char cut = '/';
  
void loop() {
  
  String sTemp, sCopy;
  String AMRdata = request();
  String json = "[";

  //각 slave의 누적전력값을 서버로 보내주기
  for (int i = 1; i <= slave_list_size; i++) {
    Split(i, AMRdata, cut, &sTemp, &sCopy);

    json += handleRoot(String(i), sTemp);
    
   if(i < slave_list_size) {
      json += ", ";
    } else {
      json += "]";
    }
    AMRdata = sCopy;
  }
  
  Serial.println(json);
  sendToServer(json);

  //delay(60000);
  delay(1000);
}

void sendToServer(String data) {
  HTTPClient http;    //Declare object of class HTTPClient

  //Post Data
  String postData = "data=" + data;
  
  http.begin("http://192.168.1.2:3000/arduino");                          //Specify request destination
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header

  int httpCode = http.POST(postData);   //Send the request
  String payload = http.getString();    //Get the response payload

 // Serial.println(httpCode);   //Print HTTP return code
 // Serial.println(payload);    //Print request response payload

  http.end();  //Close connection
}

// todo: 특정 slave 한테서 데이터 요구
String request() {
  String outAMRdata = "";
  String nulldata = "-9";

  for (int i = 0; i < slave_list_size; i++) {
    String temp = "req";
    temp += slave_list[i];

    // 슬레이브에 값 요청 보냄
    mySerial.print(temp);
    long start = millis();
    long timeout = 3000;
    String tempStr;
    while (1) {
      long current = millis();
      if (current - start < timeout) {

        if (mySerial.find("resp")) {
          tempStr = mySerial.readStringUntil('\n');
          
          outAMRdata += tempStr + '/';

          break;
        }
      } else {
        outAMRdata += nulldata+ '/' + nulldata + '/';
        break;
      }
    }
  }
  return outAMRdata;
}

//데이터 잘라내기
void Split(int id, String sData, char cSeparator, String *sTemp, String *sCopy) {
  int nGetIndex = 0 ;
  int nGetIndex2 = 0 ;

  *sTemp = "";
  String  Copy = sData;

  nGetIndex = Copy.indexOf(cSeparator);
  nGetIndex2 = Copy.indexOf(cSeparator, nGetIndex + 1);

  if (-1 != nGetIndex) {
    if (id == 1) {
      *sTemp = Copy.substring(0, nGetIndex);
      Copy = Copy.substring(nGetIndex + 1);
    } else {
      *sTemp = Copy.substring(nGetIndex + 1, nGetIndex2);
      Copy = Copy.substring(nGetIndex2 + 1);
    }

    *sCopy = Copy;
  }

}

//JSON Parsing하기
String handleRoot(String id, String Cum_Power) {
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["id"] = id;
  root["Cumulative_Power"] = Cum_Power;

  //Serial.println();
  //root.printTo(Serial);
  String result;
  root.prettyPrintTo(result);
  return result;
}


// todo: 블록체인 통신을 위한 서버 연결

// todo: sd카드 내부에 data(slave 들의 전력 사용량) 부분과 blockchain(data 해쉬값, 이전 블록, 다음 블록의 해쉬값) 부분 나누기
