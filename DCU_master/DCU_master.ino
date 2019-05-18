#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

SoftwareSerial mySerial(13, 15);

String Split(int, String, char, String *, String *);

String slave_list[] = {"1", "2", "3"};
int slave_list_size = sizeof(slave_list) / sizeof(slave_list[0]);


void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);
}


void loop() {
  char cut = '/';
  String sTemp, sCopy;
  String AMRdata = request();

  //각 slave의 누적전력값을 서버로 보내주기
  for (int i = 1; i <= slave_list_size; i++) {
    Split(i, AMRdata, cut, &sTemp, &sCopy);

    int Cum_Power = sTemp.toInt();

    handleRoot(i, Cum_Power);
    AMRdata = sCopy;
  }

}


// todo: 특정 slave 한테서 데이터 요구
String request() {
  String outAMRdata = "";

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
        outAMRdata += -1;
        break;
      }
    }
  }
  return outAMRdata;
}

//데이터 잘라내기
String Split(int id, String sData, char cSeparator, String *sTemp, String *sCopy) {
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
void handleRoot(int id, int Cum_Power) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["id"] = id;
  root["Cumulative_Power"] = Cum_Power;

  Serial.println();
  root.printTo(Serial);
}


// todo: 블록체인 통신을 위한 서버 연결

// todo: sd카드 내부에 data(slave 들의 전력 사용량) 부분과 blockchain(data 해쉬값, 이전 블록, 다음 블록의 해쉬값) 부분 나누기
