/*
   AMR_slave


   todo: 1. loop안의 delay()가 2초있기 때문에 master가 slave의 답신을 기다리는 3초를 넘어가버림 delay() 쓰지않는 방법 탐색
            >> 방법1. 시간 측정법 https://geronimob.tistory.com/18 | 방법2. 쓰레드 이용방법 https://kocoafab.cc/tutorial/view/609


*/

#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_ADS1015.h>
#include <MsTimer2.h>

SoftwareSerial mySerial(2, 3);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_ADS1015 ads;

const int chipSelect = 4; //SD카드 모듈에서 cs핀 부분

File myFile;
String fileName = "filetest.txt";
char AMRid = '1';  // todo: 각 슬레이브에 아이디 지정해주기

const float FACTOR = 100;
const float multiplier = 0.0625F;

float currentRMS = 0;
float power = 0;
int Cumulative_Power;

int Cum = 0;
int cur = 0;
String Cum_Power = "CumPower:";
String cur_power = "CurPower:";


void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  ads.setGain(GAIN_TWO);  //±2.048V 1bit =0.0625mV
  ads.begin();

  lcd.begin();
  lcd.clear();

  InitializeSDcard();

  Cumulative_Power = readData().toInt();
}


long startTime = 0 ;

void loop() {

  long loopTime = millis();

  if (loopTime - startTime <= 1000) {

    // todo: 전류 측정하기
    currentRMS = getCorriente(); //전류측정
    power = 220.0 * currentRMS;  //전력계산

    if (Cumulative_Power >= 0) {
      Cumulative_Power += int(power);
    } else {
      Cumulative_Power = 0;
    }

    // todo: 측정값 sd카드에 저장하기
    writeData(String(Cumulative_Power));

  }
  startTime = loopTime;

  delay(10);

  Cum = readData().toInt();
  cur = int(power);

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
  int data = 0;
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
    if (mySerial.find("req")) {
      if ((char)mySerial.read() == amrId) {
        mySerial.print("resp");
        mySerial.print(ADvalue);
        mySerial.print("/");
        mySerial.println(NDvalue);
      }
    }
  }
}


//전력측정
float getCorriente() {
  float voltage;
  float corriente;
  float sum = 0;
  long tiempo = millis();
  int counter = 0;

  while (millis() - tiempo < 1000) {
    voltage = ads.readADC_Differential_0_1() * multiplier;
    corriente = voltage * FACTOR;
    corriente /= 1000.0;

    sum += sq(corriente);
    counter = counter + 1;
  }

  corriente = sqrt(sum / counter);
  return (corriente);
}
