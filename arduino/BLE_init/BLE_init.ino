// For arduino mega 2560
void setup() 
{
  Serial.begin(9600);   //시리얼모니터
  Serial2.begin(57600); //블루투스 시리얼
}
void loop()
{
  if (Serial2.available()) {       
    Serial.write(Serial2.read());  //블루투스측 내용을 시리얼모니터에 출력
  }
  if (Serial.available()) {         
    Serial2.write(Serial.read());  //시리얼 모니터 내용을 블루추스 측에 WRITE
  }
}
