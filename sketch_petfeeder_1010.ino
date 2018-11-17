#include <Time.h>
#include <DS1302.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
SoftwareSerial BTSerial(2,3); //2번수신(RX), 3번 송신(TX)
#include <Servo.h>
#include "HX711.h"
#define calibration_factor 16789.0  //저울을 0으로 셋팅하기 위한 값
#define DOUT A1
#define CLK A0
LiquidCrystal lcd(12,11,6,7,8,9); //초기화
DS1302 rtc(10,5,4); //DS1302모듈의 초기화

HX711 scale(DOUT, CLK);
Servo SV; 
int pos[]={0,90}; //현재 각도값을 저장할 변수 선언

byte buffer[1024]; //데이터 수신 버퍼
int bufferPosition; //버퍼에 기록할 위치

char feed[20];  //스마트폰에서 전송한 사료양 읽어들임
char bufferIndex = 0;

Time t;

String strCopy = "";
String strTime = "";
String strFeed = "";
int mhour;
int mbun;

void setup() {
 SV.attach(13);  //서보모터 13번핀에 꽂음
 rtc.halt(false); //시간을 run-mode로 설정
 rtc.writeProtect(false); //시간설정을 자유롭게 하기 위해
//rtc.setDOW(FRIDAY); //화요일로 요일 설정
//rtc.setTime(11,12,05); //11시12분5초로 시간 설정
//rtc.setDate(28,9,2018); //2018년10월10일 날짜 설정
 
  Serial.begin(9600);
  BTSerial.begin(9600); //블루투스 모듈 초기화
  scale.set_scale(calibration_factor);  //무게센서 초기화
  scale.tare();

  lcd.begin(16,2);
  lcd.print("HELLO, MY PET!");
  bufferPosition=0;
  long weight=scale.get_units(5)*45.3592; // 무게단위 파운드를 g으로 변환
  sendData(weight);
  SV.write(pos[0]);
}

void loop() {
  while(BTSerial.available()){  //스마트폰에서 아두이노로 예약시간과 사료무게를 보낼때
    String strdata = BTSerial.readString(); 
   // Serial.print(strdata);
    strCopy = strdata;  //예약시간과 사료무게를 strCopy에 담음
    strTime = strCopy.substring(strCopy.length()-5, strCopy.length()); //예약시간만 추출
   // Serial.println(strTime);
    mhour = strTime.substring(0,2).toInt();  //예약시간의 시
   // Serial.println(mhour);
    mbun = strTime.substring(2,4).toInt();  //예약시간의 분
   // Serial.println(mbun);
    strFeed = strCopy.substring(0,strCopy.length()-strTime.length());  //사료무게
   // Serial.println(strFeed);
 }
   while(Serial.available()){  //아두이노에서 스마트폰으로 현재 무게 보낼때 
     byte data = BTSerial.read();
    buffer[bufferIndex++]=data;
   if(data=="\0"){
      bufferIndex = 0; //feed값을 불러옴
    }
   }

t= rtc.getTime();
  
long weight=(scale.get_units(5)*45.3592)-55; //밥그릇무게 55g
 sendData(weight); //현재 무게를 측정하여 스마트폰으로 보냄
 int ifeed = strFeed.toInt(); //줄 사료양을 읽어서 int로 변환

 int ihour =t.hour;   // 현재 시를 int로 변환
 int ibun = t.min;    // 현재 분을 int로 변환
 
if(ihour==mhour && ibun == mbun || mbun==0 && mhour==0){  //스마트폰에서 설정한 예약시간과 현재 시간 비교

 if(ifeed>0){
  SV.write(pos[1]);
 
} 
if(ifeed <= weight ){
  SV.write(pos[0]);
   ifeed=0;    
}
}

 scale.power_down();              // put the ADC in sleep mode ->무게센서 잠들기모드
 delay(5);
 scale.power_up();                // 무게센서 다시 깨우기
 
lcd.clear();
lcd.setCursor(0,0);  //커서를 0행0열에 둠
lcd.print("Bowl:");
lcd.print(weight);
lcd.print("g ");
lcd.setCursor(0,1);   //커서를 1행0열에 둠
lcd.print(t.hour, DEC);  //현재 시 표시
lcd.print(":");
lcd.print(t.min, DEC);   //현재 분 표시
}
 
void sendData(int value){         //스마트폰으로 현재 무게 보내는 메소드
  String message = String(value)+'\n';
  BTSerial.print(message);
  delay(3000);
}

