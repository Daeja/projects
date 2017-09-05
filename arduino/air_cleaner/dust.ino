#include<SoftwareSerial.h>

#define BLETX 5
#define BLERX 6

#define RELAY 7

SoftwareSerial bleSerial(BLETX, BLERX);
String str = "";

void setup() {
	Serial.begin(9600);
	bleSerial.begin(9600);

	pinMode(RELAY, OUTPUT);
	digitalWrite(RELAY, LOW);

	//선풍기용
	//digitalWrite(RELAY, HIGH);
}

void loop() {
	//Serial.available()
	while(bleSerial.available())  //mySerial에 전송된 값이 있으면
	{
		char myChar = (char)bleSerial.read();  //mySerial int 값을 char 형식으로 변환
		str+=myChar;   //수신되는 문자를 myString에 모두 붙임 (1바이트씩 전송되는 것을 연결)
		delay(5);           //수신 문자열 끊김 방지
	}


	if(str.equals("on")) {
		Serial.println("Turn on the pan...");
		digitalWrite(RELAY, HIGH);
	} else if(str.equals("off")) {
		Serial.println("Turn off the pan...");
		digitalWrite(RELAY, LOW);
	}

	str = "";  //버퍼 초기화
}
