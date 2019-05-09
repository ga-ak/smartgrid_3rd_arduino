/*
 * AMR_slave
 * 
 * 
 */
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
char AMRid = '1';  // todo: 각 슬레이브에 아이디 지정해주기

String data = "";
int PowerData = 0;
int totalData = data.toInt();

String ReadData = "";
String cut = "//";

int AD = 0;
int ND = 0;
String AccData = "AccData:";
String NowData = "NowData:";


void setup() {
  Serial.begin(9600);
  lcd.begin();
  lcd.clear();
  while (!Serial) {
    ;
  }
  InitializeSDcard();
}

void loop() {
  
  // todo: 전류 측정하기 (테스트용으로 가변저항값이 저장되게 해놓음)
  data = String(analogRead(A0));
  delay(1000);

  // todo: 측정값 sd카드에 저장하기
  PowerData = data.toInt();
  totalData += PowerData;
  
  myFile = SD.open(fileName, O_READ | O_WRITE | O_CREAT | O_READ);
  writeData(String(totalData), String(data));

  // todo: 저장되어 있는 값 lcd로 출력하기
  delay(1000);
  
  myFile = SD.open(fileName);
  ReadData = readData();

  AD = totalSumSplit(ReadData,cut);
  ND = NowSplit(ReadData,cut);

  RequestIdFind(AMRid, AD, ND); // todo: master의 요청이 들어오면 값 전송하기
  lcd.clear();
  lcdView(0,AccData,AD);
  lcdView(1,NowData,ND);

}


//SD카드 연결체크
void InitializeSDcard(){
  Serial.println("Opening SD Card . . .");
  delay(500);
  if(SD.begin(chipSelect))
  {
    Serial.println("SD Card ready to use");
  }else{
    Serial.println("Failed to open SD Card");
    return;
  }
}

//LCD화면출력
void lcdView(int Cursor, String view, int value){
  lcd.setCursor(0, Cursor); //커서 위치 지정
  lcd.print(view); 
  lcd.print(value); // 전력값 
}


//데이터 저장
void writeData(String intotalData, String indata) {
  
  if (myFile) {
    myFile.println(intotalData);
    myFile.println(indata);
    myFile.close(); 

  } else {
    Serial.println("error opening test.txt");
  }
}

//저장된 데이터 읽기
String readData() { 
  int data = 0;
  String total = "";

  if (myFile) {
    while (myFile.available()) {
      data = myFile.read();

      if (data != 10 && data != 13) {
        total += (char)data;
      } else {
        total += '/';
      }
    }
    myFile.close();
  } else {
    Serial.println("error opening datalog.txt");
  }
  return total;
}

//누적데이터 잘라오기
int totalSumSplit(String sData, String sSeparator) {
  int nCount = 0;
  int nGetIndex = 0 ;

  String totalSum = "";
  String sCopy = sData;

    nGetIndex = sCopy.indexOf(sSeparator);
    
    if (-1 != nGetIndex){
      totalSum = sCopy.substring(0, nGetIndex);    
    }  
  return totalSum.toInt();
}

//현재데이터 잘라오기
int NowSplit(String sData, String sSeparator) {
  int nCount = 0;
  int nGetIndex = 0 ;
  int nGetIndex2 = 0 ;
  
  String now = "";
  String sCopy = sData;
  
    nGetIndex = sCopy.indexOf(sSeparator);
    nGetIndex2 = sCopy.indexOf(sSeparator,nGetIndex+2);
  
    if (-1 != nGetIndex){
      now = sCopy.substring(nGetIndex+2,nGetIndex2);
    }  
  return now.toInt();
}

//master의 요청이 들어오면 값 전송하기
void RequestIdFind(char amrId, int ADvalue, int NDvalue) {
  if(mySerial.available()) {
    if(mySerial.find("req")){
      if((char)mySerial.read() == amrId) {
        mySerial.print("resp");
      mySerial.print(ADvalue);
      mySerial.print("/");
      mySerial.println(NDvalue);
      }
    }
  }
}
