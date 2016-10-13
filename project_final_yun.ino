#include <Process.h> //Process 라이브러리 : Yun Shield의 Linux 명령어 호출
#include <Console.h> //Console 라이브러리 : 콘솔 시리얼 통신, Arduino IDE에 디버깅 정보 표시
#include <SoftwareSerial.h>  // 소프트웨어 시리얼 라이브러리 : 아두이노 간의 시리얼 통신

SoftwareSerial mySerial(10, 11); // 소프트웨어 시리얼 정의(RX : 10번, TX : 11번)

// 전역변수 정의
const unsigned long postingInterval = 20000; // 데이터 전송 시간간격 : 20초 (최소 15초)
unsigned long lastRequest = 0;               // 가장 최근에 데이터를 전송한 시간
String dataString = "";                      // HTTP 전송할 데이터 문자열(예 : 4.58)
char data;                                   // 소프트웨어 시리얼 수신데이터(1byte)
int done;                                    // 소프트웨어 시리얼 수신 완료여부 검사용

void setup() {
  Bridge.begin();    // Uno와 Yun Shield 간의 통신 시작
  Console.begin();
  mySerial.begin(9600);  // 소프트웨어 시리얼 시작
  while (!Console);     // 콘솔 시리얼 대기(wait for Network Serial to open)
  Console.println("Ready...");    // 데이터 전송 준비
  lastRequest = millis();
  sendData();                      // 최초 데이터 전송
}
void loop() {
  long now = millis();             // 현재 시간(timestamp) 계산
  if (now - lastRequest >= postingInterval) {       // 일정 시간(20초) 간격으로 데이터 전송
    sendData();                                     // 데이터 전송 함수 호출
    lastRequest = now;                              // 데이터를 전송한 시간
    }  
  }


// 데이터 전송함수 정의
// 1. 소프트웨어 시리얼로 센서 측정값 수신
// 2. 웹 서버와의 HTTP 연결

void sendData() {
  // 1. 소프트웨어 시리얼로 센서 측정값 수신(쓰레기값을 제외하기 위해 측정값은 "A5.11Z"와 같은 형식으로 이루어져 수신됨)
  if(mySerial.available()>0) {           
    while(done==0) {
      if(mySerial.peek()=='A') {         // 시작문자(A) 제거
        mySerial.read();
        while(mySerial.peek()!='Z') {    // 종료문자(Z) 제거
          data = mySerial.read();
          if(data != 'A') {
              dataString += data;    // 시리얼 데이터(1byte)를 문자열로 조합
          }
        } done = 1;                  // 수신 완료
        
        } else data = mySerial.read();  // 손실된 데이터 버리기
      }
    }
  Console.print(dataString);   // 콘솔 시리얼 출력
  mySerial.flush();             // 수신 시리얼 버퍼 초기화
  while(mySerial.available())
    mySerial.read();
    
  // 2. 웹 서버와의 HTTP 연결
  // 접속 URL 정보, (API Key : WFH1R32NMOU6XQD1&field1)
  String url = "http://api.thingspeak.com/update?api_key=WFH1R32NMOU6XQD1&field1=";
  url += dataString;
  
  // URL 접속 및 파라미터 전송 프로세스
  Process xively;
  Console.print("\n\nSending data... ");  // 프로세스 시작
  xively.begin("curl");
  xively.addParameter("-k");
  xively.addParameter("--request");
  xively.addParameter("GET");        // HTTP GET 요청 전송
  xively.addParameter(url);          // URL 접속
  xively.run();                      // HTTP 통신(run)
  Console.println("done!");         // 프로세스 완료
  dataString = "";                   // 데이터 초기화
  done = 0;
}
