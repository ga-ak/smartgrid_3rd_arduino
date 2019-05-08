#include <SoftwareSerial.h> 

SoftwareSerial mySerial(13, 15);
String slave_list[] = {
  "1", "2", "3"
};

int slave_list_size = sizeof(slave_list) / sizeof(slave_list[0]);

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
}
int i = 0;
void loop() {
  Serial.println("connected!");
  
  request();
  delay(1000);
}

// todo: 특정 slave 한테서 데이터 요구

void request() {
  for(int i=0; i<slave_list_size; i++) {
    String temp = "req";
    temp += slave_list[i];

    // 슬레이브에 값 요청 보냄
    mySerial.print(temp);
    long start = millis();
    long timeout = 3000;
    String tempStr;
    while(1) {
      if(start - millis() < timeout) {
        if(mySerial.find("resp")) {
          tempStr = mySerial.readStringUntil('\n');
          Serial.println(tempStr);
          break;                          
        }
      } else {
        Serial.print(slave_list[i]);
        Serial.println(" time out!");
        break;
      }
    }
  }
}

// todo: 블록체인 통신을 위한 서버 연결

// todo: sd카드 내부에 data(slave 들의 전력 사용량) 부분과 blockchain(data 해쉬값, 이전 블록, 다음 블록의 해쉬값) 부분 나누기
