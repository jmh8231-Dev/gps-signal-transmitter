//GPS데이터 송신기

//21.12.12 업데이트 내용: 주변 밝기에 따라 LED 제어 구현

//DHT22를 위한 라이브러리
#include "DHT.h"

//RF모듈을 위한 라이브러리
#include <SPI.h>
#include "RF24.h"

//타이머 인터럽트
#include <MsTimer2.h>

//통신 모듈 설정
RF24 radio(9, 10);//9번핀을 CE, 10번핀을 CSN으로 SPI통신 설정
const byte addresses[6] = "132435";//파이프 주소를 설정

//온습도 센서 설정
DHT DHT(8, DHT22);

//시간 저장 변수
byte TimeData[6];

//RF모듈로 전송할 시간 데이터
char RFTimeData[32];

//주변 밝기를 전송할 변수(STM32)
char LEDflag = 'B';

//주변 밝기를 저장할 변수(아두이노)
bool ledflag = 0;

//STM32로 전송할 데이터
String Data;

//온습도 데이터
//STM32 DMA특성 때문에 더미 데이터 저장
float Hum = 12.34; 
float Temp = 56.78;

int Sean_flag = 0;

void setup() 
{
  //STM32와 UART통신 초기화
  Serial.begin(115200);
  
  //무선통신 모듈 설정
  radio.begin();//무선 통신 모듈 초기화
  radio.setPALevel(RF24_PA_MIN);//거리가 가까운 순으로 RF24_PA_MIN / RF24_PA_LOW / RF24_PA_HIGH / RF24_PA_MAX
  radio.openWritingPipe(addresses);//데이터를 보낼 주소 설정
  radio.stopListening();//송신 모드로 설정

  //센서 초기화
  DHT.begin();
  
  //타이머 인터럽트 설정
  MsTimer2::set(2500, Toggle);// 2500ms 단위로 Toggle() 함수 호출
  MsTimer2::start();//인터럽트 실행
}

//IO핀 레지스터 설정
void PinModeSetUP()
{
  pinMode(2, INPUT);//SW1
  pinMode(3, INPUT);//SW2
  pinMode(4, OUTPUT);//Debug LED
  pinMode(5, OUTPUT);//FET
  pinMode(7, INPUT);//RF모듈 송신 타이밍용
  pinMode(14, OUTPUT);//MIN LED
  pinMode(15, OUTPUT);//LOW LED
  pinMode(16, OUTPUT);//HIGH LED
  pinMode(17, OUTPUT);//MAX LED
}

void TransmitData(char TxData[32])
{
  //MIN모드
  if(digitalRead(2) == LOW && digitalRead(3) == LOW)
  {
    radio.setPALevel(RF24_PA_MIN);

    if(ledflag == 1)
    {
      digitalWrite(14, HIGH);
      digitalWrite(15, LOW);
      digitalWrite(16, LOW);
      digitalWrite(17, LOW);
    }
    else
    {
      digitalWrite(14, LOW);
      digitalWrite(15, LOW);
      digitalWrite(16, LOW);
      digitalWrite(17, LOW);
    }

    radio.write(&TxData, sizeof(TxData));//시간 데이터를 전송 
  }
  
  //LOW모드
  else if(digitalRead(2) == HIGH && digitalRead(3) == LOW)
  {
    radio.setPALevel(RF24_PA_LOW);

    if(ledflag == 1)
    {
      digitalWrite(14, LOW);
      digitalWrite(15, HIGH);
      digitalWrite(16, LOW);
      digitalWrite(17, LOW);
    }
    else
    {
      digitalWrite(14, LOW);
      digitalWrite(15, LOW);
      digitalWrite(16, LOW);
      digitalWrite(17, LOW);
    }

    radio.write(&TxData, sizeof(TxData));//시간 데이터를 전송 
  }

  //HIGH모드
  else if(digitalRead(2) == LOW && digitalRead(3) == HIGH)
  {
    radio.setPALevel(RF24_PA_HIGH);

    if(ledflag == 1)
    {
      digitalWrite(14, LOW);
      digitalWrite(15, LOW);
      digitalWrite(16, HIGH);
      digitalWrite(17, LOW);
    }
    else
    {
      digitalWrite(14, LOW);
      digitalWrite(15, LOW);
      digitalWrite(16, LOW);
      digitalWrite(17, LOW);
    }

    radio.write(&TxData, sizeof(TxData));//시간 데이터를 전송 
  }

  //MAX모드
  else if(digitalRead(2) == HIGH && digitalRead(3) == HIGH)
  {
    radio.setPALevel(RF24_PA_MAX);

    if(ledflag == 1)
    {
      digitalWrite(14, LOW);
      digitalWrite(15, LOW);
      digitalWrite(16, LOW);
      digitalWrite(17, HIGH);
    }
    else
    {
      digitalWrite(14, LOW);
      digitalWrite(15, LOW);
      digitalWrite(16, LOW);
      digitalWrite(17, LOW);
    }

    radio.write(&TxData, sizeof(TxData));//시간 데이터를 전송 
  }
}

//LCD백라이트 컨트롤 함수
void FET_Control()
{          
  //CDS셀 데이터
  int Brightness;

  Brightness = analogRead(A4);

  if(Brightness > 100)
  {
    digitalWrite(5, LOW);
    LEDflag = 'B';
    ledflag = 1;
  }
  else
  {
    digitalWrite(5, HIGH);
    LEDflag = 'C';
    ledflag = 0;
  }
}

void CheckFlag()
{
  if(Sean_flag == 1)
  {
    Sean_flag = 0;

    //습도값을 변수에 저장
    Hum = DHT.readHumidity();
    //온도값을 변수에 저장
    Temp = DHT.readTemperature();
  }
}

void loop()
{
  //리셋을 방지하기 위해 WDT 전송
  Data = String(Hum) + String(Temp) + 'A' + String(LEDflag); 
  Serial.print(Data);
  
  //핀 모드를 정해주는 함수(오류로인해 핀 모드 레지스터가 바뀌어도 바로 수정할수 있다)
  PinModeSetUP(); 
  
  //만약 시간 데이터를 수신하면
  if(Serial.available() > 0)
  {
    //시간데이터를 읽어와 배열에 저장
    for(int i = 0; i < 6; i++)
    {
      TimeData[i] = Serial.read();//시간 데이터 저장
    }
  }
  
  //만약 STM32가 전송을 끝냈으면
  if(digitalRead(7) == LOW)
  {
    //바이트형 배열을 문자열로 변환
    String Time = String(TimeData[0]) + "/" + String(TimeData[1]) + "/" + String(TimeData[2]) + "/" + String(TimeData[3]) + "/" + String(TimeData[4]) + "/" + String(TimeData[5]);

    //문자열을 char형 배열로 변환
    Time.toCharArray(RFTimeData, Time.length() + 1);//수신기로 전송하기 위해 char형 변수로 바꾸어 저장
    
    TransmitData(RFTimeData);//시간정보 전송
  }
  
  //밝기를 센싱하여 LCD백라이트 제어   
  FET_Control();

  CheckFlag();//flag변수들을 확인하는 함수
}

void Toggle()//3초마다 실행되는 함수
{
  if(ledflag == 1)
  {
    //Debug LED 토글
    static boolean output = HIGH;
    digitalWrite(4, output);
    output =! output;
  }
  else
  {
    digitalWrite(4, LOW);
  }
  
    Sean_flag = 1;
}
