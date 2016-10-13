#include <Servo.h>                  //서보모터 제어시 필요한 라이브러리
#define TANK_HEIGHT 7.0              // 물탱크 높이 정의
Servo motor;                        //서보모터 관련 오브젝트 생성
int Switch=4;                        //tact스위치(pin : 4)
int trig=8, echo=9;                //초음파센서(pin : 8,9)
int green=2, blue=12, red=13;        //RGB LED(pin : 2,12,13)
int speaker=7;                       //피에조부저(pin : 7)
int clockPin = 3, latchPin = 5, dataPin = 6; //7segment(pin : 3, 5, 6)
byte dec_digits[] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111100, 0b00000111};    //7segment에 출력할 한자리 숫자들을 배열변수에 저장
float avg_waterheight;               //물의 평균 높이 저장 변수

void setup() { //셋팅문
  Serial.begin(9600); //9600레이트 셋팅
  
  motor.attach(10);  //서보모터 셋팅(pin : 10)
  pinMode(Switch,INPUT_PULLUP); //tact스위치 셋팅
  pinMode(trig,OUTPUT); pinMode(echo,INPUT); //초음파 센서 셋팅
  pinMode(red,OUTPUT); pinMode(green,OUTPUT); pinMode(blue,OUTPUT); //RGB LED 셋팅
  pinMode(latchPin, OUTPUT); pinMode(clockPin, OUTPUT); pinMode(dataPin, OUTPUT); //7segment LED 셋팅
}

void loop() { //반복문
  avg_waterheight_write();                   //평균 물의 높이 계산 후 출력
  rgb_write();                              //rgb 불빛 출력(rgb 제어)
  dec_digits_write();                       //물높이 값 출력(7segment 출력)
  motor_write();                            //물 유입 밸브 제어(모터 제어)
  
  delay(1000);
}

void avg_waterheight_write(){ //평균 물 높이 구하기
  float duration,distance;
  avg_waterheight = 0; //다음 값을 받기 위한 평균 물높이 변수 초기화
  while(avg_waterheight <= 0){
   digitalWrite(trig, LOW); digitalWrite(echo, LOW);
   delayMicroseconds(2);
   digitalWrite(trig, HIGH);
   delayMicroseconds(10);
   digitalWrite(trig, LOW);// 초음파센서 거리 측정
   duration = pulseIn(echo, HIGH); // 초음파센서 거리 측정
   distance =  duration/29.387/2.0; // 초음파센서 거리 측정
   avg_waterheight = 7.0-distance; //물의 높이를 계산
  }
  
  if (avg_waterheight > TANK_HEIGHT) avg_waterheight = TANK_HEIGHT; //평균 물 높이 변수에 물탱크 높이 대입
  Serial.print("A");
  Serial.print(avg_waterheight); //평균 물의 높이 출력
  Serial.println("Z");
}

void dec_digits_write(){ //7segment LED 제어
  digitalWrite(latchPin, LOW);  //shift out the bits
  
  if(avg_waterheight>0) shiftOut(dataPin, clockPin, MSBFIRST, dec_digits[int(avg_waterheight)]); //7segment LED에 물높이를 한자리 수로 출력
  else shiftOut(dataPin, clockPin, MSBFIRST, dec_digits[0]); //물탱크 총 높이 7을 넘으면 7segment LED에에 0을 출력
}

void motor_write(){ //물 유입 밸브 제어
  if(avg_waterheight > 5) motor.write(180);      //물 유입 밸브 잠그기
  else if(avg_waterheight <= 3){                 //물 유입 밸브 열기
    if(digitalRead(Switch)==LOW) motor.write(0);   //tact스위치가 눌리면 물 유입 진행
  }
  digitalWrite(latchPin, HIGH); //다음 값을 받을 때까지 대기(7segment LED)
}

void speaker_write(){tone(speaker,800,100);} //피에조부저 제어

void rgb_write(){ //rgb 제어
  if(avg_waterheight > 5){                       //물높이가 5을 초과이면 초록색 출력
   digitalWrite(green,HIGH); digitalWrite(red,LOW); digitalWrite(blue,LOW); //green만 HIGH
  }
  else if(avg_waterheight > 3){                 //물높이가 3을 초과이면 파란색 출력
   digitalWrite(blue,HIGH); digitalWrite(red,LOW); digitalWrite(green,LOW); //blue만 HIGH
  }
 else{                                      //나머지의 경우 빨간색 출력
  digitalWrite(red,HIGH); digitalWrite(blue,LOW); digitalWrite(green,LOW); //red만 HIGH
  speaker_write();                           //경고 부저 울리기(피에조부저 출력)
 }
}

