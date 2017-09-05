#include <EtherCard.h>

/////////////////////////////////////////
//  dust.ino 업그레이드 버전
/////////////////////////////////////////

#define RELAY 3
#define LED 2

#define DELAYTIME 280
#define DELAYTIME2 40
#define OFFTIME 9680

float dustVal=0;
float dustDensity = 0;

// Sensor Status on WebPage
char * DUST_statusLabel;
char * DUST_data;

// Status Characters
char* on = "ON";
char* off = "OFF";
char* RED = "#FF7676";
char* YEL = "#FFFF67";
char* GRN = "#9AFF9E";

// Status Buffer
char dustData[16]="";

int pos = 0;

// ethernet interface mac address, must be unique on the LAN
byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x63 };

byte Ethernet::buffer[700];

void chk_Env(){
	// Dust check.
	if(dustDensity > 0.49)
		DUST_statusLabel = RED;
	else if(dustDensity > 0.24)
		DUST_statusLabel = YEL;
	else 
		DUST_statusLabel = GRN;
}

void setup () {
	// Serial Begin.. (# Ethernet Module works on 57600 speed)
	Serial.begin(57600);

	pinMode(LED,OUTPUT);
	pinMode(RELAY, OUTPUT);
	digitalWrite(RELAY, HIGH);

	if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
		Serial.println(F("Failed to access Ethernet controller"));
	if (!ether.dhcpSetup())
		Serial.println(F("DHCP failed"));

	//Web connection ip
	ether.printIp("IP:  ", ether.myip);
	ether.printIp("GW:  ", ether.gwip);
}

void loop () {
	digitalWrite(LED,LOW);
	delayMicroseconds(DELAYTIME);

	dustVal=analogRead(A0);
	delayMicroseconds(DELAYTIME2);

	digitalWrite(LED,HIGH);
	delayMicroseconds(OFFTIME);

	dustDensity = 0.17 * (dustVal * 0.0049) - 0.01;
	Serial.print("Dust density(mg/m3) = ");
	Serial.println(dustDensity);

	if(dustDensity >= 0.25) digitalWrite(RELAY, LOW);
	else digitalWrite(RELAY, HIGH);

	memset(dustData, 0 , sizeof(dustData));

	dtostrf(dustDensity, 7, 2, dustData);

	//웹 페이지 받을 준비
	word len = ether.packetReceive();
	word pos = ether.packetLoop(len);

	if(pos) {
		BufferFiller bfill = ether.tcpOffset();
		bfill.emit_p(PSTR("HTTP/1.0 200 OK\r\n"
					"Content-Type:text/html\r\nPragma:no-cache\r\n\r\n"
					"<html>"
					"<head>"
					"<meta http-equiv=\"refresh\" content=\"2\" charset=\"utf-8\">"
					"<title>Dust</title>"
					"</head>"
					"<body>"
					"<br>Environment:<br>"
					"<table>"
					"<tr><th>DUST</th><th bgcolor=$S>$S</th></tr>"
					"</table>"
					"</body>"
					"</html>"
					), DUST_statusLabel, dustData );

		ether.httpServerReply(bfill.position());  
	}

	chk_Env();
	delay(100);
}
