#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

SoftwareSerial mySerial(2, 3);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int chipSelect = 4; //SD카드 모듈에서 cs핀 부분

File myFile;
String fileName = "filetest.txt";
char AMRid = '3';  // todo: 각 슬레이브에 아이디 지정해주기

long Cumulative_Power;

int Cum = 0;
int cur = 0;
String Cum_Power = "CumP:";
String cur_power = "CurP:";

int analogPin = 0;
int buttonApin = 7;

int sensor = 0;


void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  pinMode(buttonApin, INPUT_PULLUP);

  lcd.begin();
  lcd.clear();

  InitializeSDcard();

  Cumulative_Power = readData().toInt();

}


void loop() {

    // todo: 전류 측정하기대신 가변저항 값으로 변환
    sensor = analogRead(analogPin);
    delay(1000);
    
    //버튼으로 누적전력 초기화
    if (digitalRead(buttonApin) == HIGH) {
      Cumulative_Power += sensor;
    } else if (digitalRead(buttonApin) == LOW) {
      Cumulative_Power = 0;
    }
    
  // todo: 측정값 sd카드에 저장하기
  writeData(String(Cumulative_Power));

  delay(10);

  Cum = readData().toInt();
  cur = sensor;

  RequestIdFind(AMRid, Cum, cur); // todo: master의 요청이 들어오면 값 전송하기

  // todo: 저장되어 있는 값 lcd로 출력하기
  lcd.clear();
  lcdView(0, Cum_Power, Cum);
  lcdView(1, cur_power, cur);

}// end of loop

//SD카드 연결체크
void InitializeSDcard() {
  lcd.setCursor(0, 0);
  lcd.print("Opening SD Card..");
  delay(500);
  if (SD.begin(chipSelect))
  {
    lcd.setCursor(0, 1);
    lcd.print("Card ready to use");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Failed to open Card");
    return;
  }
}

//LCD화면출력
void lcdView(int Cursor, String view, int value) {
  lcd.setCursor(0, Cursor);
  lcd.print(view);
  lcd.print(value);
}


//데이터 저장
void writeData(String IntotalData) {
  myFile = SD.open(fileName, O_READ | O_WRITE | O_CREAT | O_READ);

  if (myFile) {
    myFile.println(IntotalData);
    myFile.close();
  } else {
    Serial.println("error opening test.txt");
  }

}

//저장된 데이터 읽기
String readData() {
  long data = 0;
  String total = "";

  myFile = SD.open(fileName);
  if (myFile) {
    while (myFile.available()) {
      data = myFile.read();
      total += (char)data;
    }
    myFile.close();
  } else {
    Serial.println("error opening datalog.txt");
  }
  return total;
}


//master의 요청이 들어오면 값 전송하기
void RequestIdFind(char amrId, int ADvalue, int NDvalue) {
  if (mySerial.available()) {
    Serial.write(mySerial.read());
    Serial.println();
    if (mySerial.find("req")) {
      if ((char)mySerial.read() == amrId) {
        Serial.println("ok");
        mySerial.print("resp");
        mySerial.print(ADvalue);
        mySerial.print("/");
        mySerial.println(NDvalue);
      }else{
         mySerial.flush();
      }
    }
  }
}
