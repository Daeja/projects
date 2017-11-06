///////////////////////////////////////////////////////
//
// 2017. 11. 03. release
//
///////////////////////////////////////////////////////

#include <SoftwareSerial.h>

#define DISPLAY_SERIAL

SoftwareSerial mySerial(2, 3);

#define MASTER_VOLUME 127
#define INSTRUMENT 3

#define W_NUMBER_OF_SHIFT_CHIPS  4
#define W_DATA_WIDTH   W_NUMBER_OF_SHIFT_CHIPS * 8
#define W_PULSE_WIDTH_USEC   5
#define W_POLL_DELAY_MSEC   1

#define B_NUMBER_OF_SHIFT_CHIPS  3  
#define B_DATA_WIDTH   B_NUMBER_OF_SHIFT_CHIPS * 8
#define B_PULSE_WIDTH_USEC   5
#define B_POLL_DELAY_MSEC   1

#define BYTES_VAL_T unsigned long int

#define BUF 22

const byte note_syllable[7] = {
  0b00000001, //c
  0b00000010, //d
  0b00000100, //e
  0b00001000, //f
  0b00010000, //g
  0b00100000, //a
  0b01000000, //b
};

//WHITE
int W_ploadPin        = 8;  // Connects to Parallel load pin the 165
int W_clockEnablePin  = 9;  // Connects to Clock Enable pin the 165
int W_dataPin         = 11; // Connects to the Q7 pin the 165
int W_clockPin        = 12; // Connects to the Clock pin the 165

//BLACK 
int B_ploadPin        = 4;  // Connects to Parallel load pin the 165
int B_clockEnablePin  = 5;  // Connects to Clock Enable pin the 165
int B_dataPin         = 6;  // Connects to the Q7 pin the 165
int B_clockPin        = 7;  // Connects to the Clock pin the 165

//WHITE_HELPER
int WH_dataPin        = 29;
int WH_clockPin       = 30;
int WH_latchPin       = 31;

//BLACK HELPER
int BL_dataPin        = 40;
int BL_clockPin       = 42;
int BL_latchPin       = 41;

//PEDAL
int A_Pedal           = 24; // for releasing keys

//Midi Instrument
byte note = 0; //The MIDI note value to be played
byte resetMIDI = 4; //Tied to VS1053 Reset line

// MUSIC_NOTE_COUNT
int m_count = 0;
int matching_count = 0;
bool flag =false;

BYTES_VAL_T W_Compare[32];
BYTES_VAL_T W_pinValues;
BYTES_VAL_T W_oldPinValues;
BYTES_VAL_T B_Compare[22];
BYTES_VAL_T B_pinValues;
BYTES_VAL_T B_oldPinValues;

boolean W_chk[32]={false,};
boolean B_chk[22]={false,};

char buf[BUF];
int bufPosition;

// WHITE_HELPER DATA
byte leds[] = { 0, 0, 0, 0 };
byte tleds[] = { 0, 0, 0, 0 };
//Send a MIDI note-on message.  Like pressing a piano key
//channel ranges from 0-15
void noteOn(byte channel, byte note, byte attack_velocity) {
	talkMIDI( (0x90 | channel), note, attack_velocity);
}

//Send a MIDI note-off message.  Like releasing a piano key
void noteOff(byte channel, byte note, byte release_velocity) {
	talkMIDI( (0x80 | channel), note, release_velocity);
}

//Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void talkMIDI(byte cmd, byte data1, byte data2) {
	mySerial.write(cmd);
	mySerial.write(data1);

	//Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes 
	//(sort of: http://253.ccarh.org/handout/midiprotocol/)
	if( (cmd & 0xF0) <= 0xB0)
		mySerial.write(data2);
}

BYTES_VAL_T W_read_shift_regs()
{
	long bitVal;
	BYTES_VAL_T bytesVal = 0;

	digitalWrite(W_clockEnablePin, HIGH);
	digitalWrite(W_ploadPin, LOW);
	delayMicroseconds(W_PULSE_WIDTH_USEC);
	digitalWrite(W_ploadPin, HIGH);
	digitalWrite(W_clockEnablePin, LOW);

	for(int i = 0; i < W_DATA_WIDTH; i++)
	{
		bitVal = digitalRead(W_dataPin);
		bytesVal |= (bitVal << ((W_DATA_WIDTH-1) - i));
		digitalWrite(W_clockPin, HIGH);
		delayMicroseconds(W_PULSE_WIDTH_USEC);
		digitalWrite(W_clockPin, LOW);
	}
	return(bytesVal);
}

BYTES_VAL_T B_read_shift_regs()
{
	long bitVal;
	BYTES_VAL_T bytesVal = 0;

	digitalWrite(B_clockEnablePin, HIGH);
	digitalWrite(B_ploadPin, LOW);
	delayMicroseconds(B_PULSE_WIDTH_USEC);
	digitalWrite(B_ploadPin, HIGH);
	digitalWrite(B_clockEnablePin, LOW);

	for(int i = 0; i < B_DATA_WIDTH; i++)
	{
		bitVal = digitalRead(B_dataPin);
		bytesVal |= (bitVal << ((B_DATA_WIDTH-1) - i));
		digitalWrite(B_clockPin, HIGH);
		delayMicroseconds(B_PULSE_WIDTH_USEC);
		digitalWrite(B_clockPin, LOW);
	}
	return(bytesVal);
}

boolean PedalStatus(void) {
	boolean stat = digitalRead(A_Pedal);
	if(stat) return true;
	else return false;
}

void PlayingWithKeys(void) {

	// 여기서 getMessage() 호출하고
	// 블루투스 시리얼로 패킷 받아서 불을 먼저 킴. 이후 버튼정보 배열값을 0->1로 바꿔준 다음에
	// 아래 KEYS BEGIN 부분에서 버튼을 누르면 다시 배열 값이 1인지 확인하고 0으로 바꿔줌
	// 이후 LED부분 제어를 맡고있는 시프트레지스터를 조작해서 불을 꺼주면 됨.

	/// KEYS BEGIN ////////////////////////////////////////////////////////////////////////////////////  

  //2017.10.19. - 대현
  /////////////////////////////////////////////////////////////////////////////////////////
  // 같은 leds내의 2개 이상의 불이 들어올 경우 - 키 값 비교를 통한 LED 제어를 해야함.
  /////////////////////////////////////////////////////////////////////////////////////////
	//PLAYING WITH WHITE KEYS
	if(!(W_pinValues & 0x00000001) && !W_chk[0]) {
		noteOn(0, 36, 60);
		W_Compare[0] = (!(W_pinValues & 0x00000001));
		W_chk[0] = true;
		// 여기서 불을 꺼주면 됨.
    if((leds[0] & 1) == 1)
      matching_count++;
	} else { 
		if( (W_read_shift_regs() & 0x1) == W_Compare[0] ) {
			W_chk[0] = false;
		}
		if(!PedalStatus()) noteOff(0, 36, 0);
	}    
	if(!(W_pinValues & 0x00000002) && !W_chk[1]) { 
		noteOn(0, 38, 60); 
		W_Compare[1] = (!(W_pinValues & 0x00000002));
		W_chk[1] = true;
    if((leds[0] & 2) == 2)
      matching_count++;
	} else { 
		if( (W_read_shift_regs() >> 1 & 0x1) == W_Compare[1] ) {
			W_chk[1] = false;
		}
		if(!PedalStatus()) noteOff(0, 38, 0);
	}
	if(!(W_pinValues & 0x00000004) && !W_chk[2]) {
		noteOn(0, 40, 60);
		W_Compare[2] = (!(W_pinValues & 0x00000004));
		W_chk[2] = true;
    if((leds[0] & 4) == 4)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 2 & 0x1) == W_Compare[2] ) {
			W_chk[2] = false;  
		}
		if(!PedalStatus()) noteOff(0, 40, 0); 
	}
	if(!(W_pinValues & 0x00000008) && !W_chk[3]) {
		noteOn(0, 41, 60);
		W_Compare[3] = (!(W_pinValues & 0x00000008));
		W_chk[3] = true;
    if((leds[0] & 8) == 8)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 3 & 0x1) == W_Compare[3] ) {
			W_chk[3] = false;  
		}
		if(!PedalStatus()) noteOff(0, 41, 0);
	}
	if(!(W_pinValues & 0x00000010) && !W_chk[4]) {
		noteOn(0, 43, 60);
		W_Compare[4] = (!(W_pinValues & 0x00000010));
		W_chk[4] = true;
    if((leds[0] & 16) == 16)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 4 & 0x1) == W_Compare[4] ) {
			W_chk[4] = false;  
		}
		if(!PedalStatus()) noteOff(0, 43, 0);
	}
	if(!(W_pinValues & 0x00000020) && !W_chk[5]) {
		noteOn(0, 45, 60);
		W_Compare[5] = (!(W_pinValues & 0x00000020));
		W_chk[5] = true;
    if((leds[0] & 32) == 32)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 5 & 0x1) == W_Compare[5] ) {
			W_chk[5] = false;  
		}
		if(!PedalStatus()) noteOff(0, 45, 0);
	}
	if(!(W_pinValues & 0x00000040) && !W_chk[6]) {
		noteOn(0, 47, 60);
		W_Compare[6] = (!(W_pinValues & 0x00000040));
		W_chk[6] = true;
    if((leds[0] & 64) == 64)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 6 & 0x1) == W_Compare[6] ) {
			W_chk[6] = false;  
		}
		if(!PedalStatus()) noteOff(0, 47, 0);
	}
	if(!(W_pinValues & 0x00000080) && !W_chk[7]) {
		noteOn(0, 48, 60);
		W_Compare[7] = (!(W_pinValues & 0x00000080));
		W_chk[7] = true;
    if((leds[0] & 128) == 128)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 7 & 0x1) == W_Compare[7] ) {
			W_chk[7] = false;  
		}
		if(!PedalStatus()) noteOff(0, 48, 0);
	}
	if(!(W_pinValues & 0x00000100) && !W_chk[8]) {
		noteOn(0, 50, 60);
		W_Compare[8] = (!(W_pinValues & 0x00000100));
		W_chk[8] = true;
    //if(leds[1] == 1 || leds[1] == 3 || leds[1] == 5 || leds[1] == 9 || leds[1] == 17 || leds[1] == 33 || leds[1] == 65 || leds[1] == 129)
    if((leds[1] & 1) == 1)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 8 & 0x1) == W_Compare[8] ) {
			W_chk[8] = false;  
		}
		if(!PedalStatus()) noteOff(0, 50, 0);
	}
	if(!(W_pinValues & 0x00000200) && !W_chk[9]) {
		noteOn(0, 52, 60);
		W_Compare[9] = (!(W_pinValues & 0x00000200));
		W_chk[9] = true;
    //if(leds[1] == 2 || leds[1] == 6 || leds[1] == 10 || leds[1] == 18 || leds[1] == 34 || leds[1] == 66 || leds[1] == 130)
    if((leds[1] & 2) == 2)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 9 & 0x1) == W_Compare[9] ) {
			W_chk[9] = false;  
		}
		if(!PedalStatus()) noteOff(0, 52, 0);
	}
	if(!(W_pinValues & 0x00000400) && !W_chk[10]) {
		noteOn(0, 53, 60);
		W_Compare[10] = (!(W_pinValues & 0x00000400));
		W_chk[10] = true;
   //if(leds[1] == 4 || leds[1] == 12 || leds[1] == 20 || leds[1] == 36 || leds[1] == 68 || leds[1] == 132)
   if((leds[1] & 4) == 4)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 10 & 0x1) == W_Compare[10] ) {
			W_chk[10] = false;  
		}
		if(!PedalStatus()) noteOff(0, 53, 0);
	}
	if(!(W_pinValues & 0x00000800) && !W_chk[11]) {
		noteOn(0, 55, 60);
		W_Compare[11] = (!(W_pinValues & 0x00000800));
		W_chk[11] = true;
   //if(leds[1] == 8 || leds[1] == 24 || leds[1] == 40 || leds[1] == 72 || leds[1] == 136)
   if((leds[1] & 8) == 8)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 11 & 0x1) == W_Compare[11] ) {
			W_chk[11] = false;  
		}
		if(!PedalStatus()) noteOff(0, 55, 0);
	}
	if(!(W_pinValues & 0x00001000) && !W_chk[12]) {
		noteOn(0, 57, 60);
		W_Compare[12] = (!(W_pinValues & 0x00001000));
		W_chk[12] = true;
   //if(leds[1] == 16 || leds[1] == 48 || leds[1] == 80 || leds[1] == 144)
   if((leds[1] & 16) == 16)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 12 & 0x1) == W_Compare[12] ) {
			W_chk[12] = false;  
		}
		if(!PedalStatus()) noteOff(0, 57, 0);
	}
	if(!(W_pinValues & 0x00002000) && !W_chk[13]) {
		noteOn(0, 59, 60);
		W_Compare[13] = (!(W_pinValues & 0x00002000));
		W_chk[13] = true;
   //if(leds[1] == 32 || leds[1] == 96 || leds[1] == 160)
   if((leds[1] & 32) == 32)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 13 & 0x1) == W_Compare[13] ) {
			W_chk[13] = false;  
		}
		if(!PedalStatus()) noteOff(0, 59, 0);
	}
	if(!(W_pinValues & 0x00004000) && !W_chk[14]) {
		noteOn(0, 60, 60);
		W_Compare[14] = (!(W_pinValues & 0x00004000));
		W_chk[14] = true;
   //if(leds[1] == 64 || leds[1] == 192)
   if((leds[1] & 64) == 64)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 14 & 0x1) == W_Compare[14] ) {
			W_chk[14] = false;  
		}
		if(!PedalStatus()) noteOff(0, 60, 0);
	}
	if(!(W_pinValues & 0x00008000) && !W_chk[15]) {
		noteOn(0, 62, 60);
		W_Compare[15] = (!(W_pinValues & 0x00008000));
		W_chk[15] = true;
   //if(leds[1] == 128)
   if((leds[1] & 128) == 128)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 15 & 0x1) == W_Compare[15] ) {
			W_chk[15] = false;  
		}
		if(!PedalStatus()) noteOff(0, 62, 0);
	}
	if(!(W_pinValues & 0x00010000) && !W_chk[16]) {
		noteOn(0, 64, 60);
		W_Compare[16] = (!(W_pinValues & 0x00010000));
		W_chk[16] = true;
   if((leds[2] & 1) == 1)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 16 & 0x1) == W_Compare[16] ) {
			W_chk[16] = false;  
		}
		if(!PedalStatus()) noteOff(0, 64, 0);
	}
	if(!(W_pinValues & 0x00020000) && !W_chk[17]) {
		noteOn(0, 65, 60);
		W_Compare[17] = (!(W_pinValues & 0x00020000));
		W_chk[17] = true;
   if((leds[2] & 2) == 2)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 17 & 0x1) == W_Compare[17] ) {
			W_chk[17] = false;  
		}
		if(!PedalStatus()) noteOff(0, 65, 0);
	}
	if(!(W_pinValues & 0x00040000) && !W_chk[18]) {
		noteOn(0, 67, 60);
		W_Compare[18] = (!(W_pinValues & 0x00040000));
		W_chk[18] = true;
   if((leds[2] & 4) == 4)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 18 & 0x1) == W_Compare[18] ) {
			W_chk[18] = false;  
		}
		if(!PedalStatus()) noteOff(0, 67, 0); 
	}
	if(!(W_pinValues & 0x00080000) && !W_chk[19]) {
		noteOn(0, 69, 60);
		W_Compare[19] = (!(W_pinValues & 0x00080000));
		W_chk[19] = true;
   if((leds[2] & 8) == 8)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 19 & 0x1) == W_Compare[19] ) {
			W_chk[19] = false;  
		}
		if(!PedalStatus()) noteOff(0, 69, 0);
	}
	if(!(W_pinValues & 0x00100000) && !W_chk[20]) {
		noteOn(0, 71, 60);
		W_Compare[20] = (!(W_pinValues & 0x00100000));
		W_chk[20] = true;
   if((leds[2] & 16) == 16)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 20 & 0x1) == W_Compare[20] ) {
			W_chk[20] = false;  
		}
		if(!PedalStatus()) noteOff(0, 71, 0);
	}
	if(!(W_pinValues & 0x00200000) && !W_chk[21]) {
		noteOn(0, 72, 60);
		W_Compare[21] = (!(W_pinValues & 0x00200000));
		W_chk[21] = true;
   if((leds[2] & 32) == 32)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 21 & 0x1) == W_Compare[21] ) {
			W_chk[21] = false;  
		}
		if(!PedalStatus()) noteOff(0, 72, 0);
	}
	if(!(W_pinValues & 0x00400000) && !W_chk[22]) {
		noteOn(0, 74, 60);
		W_Compare[22] = (!(W_pinValues & 0x00400000));
		W_chk[22] = true;
   if((leds[2] & 64) == 64)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 22 & 0x1) == W_Compare[22] ) {
			W_chk[22] = false;  
		}
		if(!PedalStatus())  noteOff(0, 74, 0);
	}
	if(!(W_pinValues & 0x00800000) && !W_chk[23]) {
		noteOn(0, 76, 60);
		W_Compare[23] = (!(W_pinValues & 0x00800000));
		W_chk[23] = true;
   if((leds[2] & 128) == 128)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 23 & 0x1) == W_Compare[23] ) {
			W_chk[23] = false;  
		}
		if(!PedalStatus()) noteOff(0, 76, 0);
	}
	if(!(W_pinValues & 0x01000000) && !W_chk[24]) {
		noteOn(0, 77, 60);
		W_Compare[24] = (!(W_pinValues & 0x01000000));
		W_chk[24] = true;
   if((leds[3] & 1) == 1)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 24 & 0x1) == W_Compare[24] ) {
			W_chk[24] = false;  
		}
		if(!PedalStatus()) noteOff(0, 77, 0);
	}
	if(!(W_pinValues & 0x02000000) && !W_chk[25]) {
		noteOn(0, 79, 60);
		W_Compare[25] = (!(W_pinValues & 0x02000000));
		W_chk[25] = true;
   if((leds[3] & 2) == 2)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 25 & 0x1) == W_Compare[25] ) {
			W_chk[25] = false;  
		}
		if(!PedalStatus()) noteOff(0, 79, 0);
	}
	if(!(W_pinValues & 0x04000000) && !W_chk[26]) {
		noteOn(0, 81, 60);
		W_Compare[26] = (!(W_pinValues & 0x04000000)); // Bug fixed
		W_chk[26] = true;
   if((leds[3] & 4) == 4)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 26 & 0x1) == W_Compare[26] ) {
			W_chk[26] = false;  
		}
		if(!PedalStatus()) noteOff(0, 81, 0);
	}
	if(!(W_pinValues & 0x08000000) && !W_chk[27]) {
		noteOn(0, 83, 60);
		W_Compare[27] = (!(W_pinValues & 0x08000000));
		W_chk[27] = true;
   if((leds[3] & 8) == 8)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 27 & 0x1) == W_Compare[27] ) {
			W_chk[27] = false;  
		}
		if(!PedalStatus()) noteOff(0, 83, 0);
	}
	if(!(W_pinValues & 0x10000000) && !W_chk[28]) {
		noteOn(0, 84, 60); 
		W_Compare[28] = (!(W_pinValues & 0x10000000));
		W_chk[28] = true;
   if((leds[3] & 16) == 16)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 28 & 0x1) == W_Compare[28] ) {
			W_chk[28] = false;  
		}
		if(!PedalStatus()) noteOff(0, 84, 0);
	}
	if(!(W_pinValues & 0x20000000) && !W_chk[29]) {
		noteOn(0, 86, 60);
		W_Compare[29] = (!(W_pinValues & 0x20000000));
		W_chk[29] = true;
   if((leds[3] & 32) == 32)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 29 & 0x1) == W_Compare[29] ) {
			W_chk[29] = false;  
		}
		if(!PedalStatus()) noteOff(0, 86, 0);
	}
	if(!(W_pinValues & 0x40000000) && !W_chk[30]) {
		noteOn(0, 88, 60);
		W_Compare[30] = (!(W_pinValues & 0x40000000));
		W_chk[30] = true;
   if((leds[3] & 64) == 64)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 30 & 0x1) == W_Compare[30] ) {
			W_chk[30] = false;  
		}
		if(!PedalStatus()) noteOff(0, 88, 0);
	}
	if(!(W_pinValues & 0x80000000) && !W_chk[31]) {
		noteOn(0, 89, 60);
		W_Compare[31] = (!(W_pinValues & 0x80000000));
		W_chk[31] = true;
   if((leds[3] & 128) == 128)
      matching_count++;
	} else {
		if( (W_read_shift_regs() >> 31 & 0x1) == W_Compare[31] ) {
			W_chk[31] = false;  
		}
		if(!PedalStatus()) noteOff(0, 89, 0); 
	}


	//PLYING WITH BLACK KEYS
    if(!(B_pinValues & 0x000001) && !B_chk[0]){
      noteOn(0, 37, 60);
      B_Compare[0] = (!(B_pinValues & 0x000001));
      B_chk[0] = true;
      if((tleds[0] & 1) == 1){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() & 0x1) == B_Compare[0] ){
        B_chk[0] = false;
      }
      if(!PedalStatus()) noteOff(0, 37, 0);
    }   
    if(!(B_pinValues & 0x000002) && !B_chk[1]){
      noteOn(0, 39, 60);
      B_Compare[1] = (!(B_pinValues & 0x000002));
      B_chk[1] = true;
      if((tleds[0] & 2) == 2){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 1 & 0x1) == B_Compare[1] ){
        B_chk[1] = false;
      }
      if(!PedalStatus()) noteOff(0, 39, 0);
    }
    if(!(B_pinValues & 0x000004) && !B_chk[2]){
      noteOn(0, 42, 60);
      B_Compare[2] = (!(B_pinValues & 0x000004));
      B_chk[2] = true;
      if((tleds[0] & 4) == 4){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 2 & 0x1) == B_Compare[2] ){
        B_chk[2] = false;
      }
      if(!PedalStatus()) noteOff(0, 42, 0);
    }
    if(!(B_pinValues & 0x000008) && !B_chk[3]){
      noteOn(0, 44, 60);
      B_Compare[3] = (!(B_pinValues & 0x000008));
      B_chk[3] = true;
      if((tleds[0] & 8) == 8){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 3 & 0x1) == B_Compare[3] ){
        B_chk[3] = false;
      }
      if(!PedalStatus()) noteOff(0, 44, 0);
    }
    if(!(B_pinValues & 0x000010) && !B_chk[4]){
      noteOn(0, 46, 60);
      B_Compare[4] = (!(B_pinValues & 0x000010));
      B_chk[4] = true;
      if((tleds[0] & 16) == 16){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 4 & 0x1) == B_Compare[4] ){
        B_chk[4] = false;
      }
      if(!PedalStatus()) noteOff(0, 46, 0);
    }
    if(!(B_pinValues & 0x000020) && !B_chk[5]){
      noteOn(0, 49, 60);
      B_Compare[5] = (!(B_pinValues & 0x000020));
      B_chk[5] = true;
      if((tleds[0] & 32) == 32){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 5 & 0x1) == B_Compare[5] ){
        B_chk[5] = false;
      }
      if(!PedalStatus()) noteOff(0, 49, 0);
    }
    if(!(B_pinValues & 0x000040) && !B_chk[6]){
      noteOn(0, 51, 60);
      B_Compare[6] = (!(B_pinValues & 0x000040));
      B_chk[6] = true;
      if((tleds[0] & 64) == 64){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 6 & 0x1) == B_Compare[6] ){
        B_chk[6] = false;
      }
      if(!PedalStatus()) noteOff(0, 51, 0);
    }
    if(!(B_pinValues & 0x000080) && !B_chk[7]){
      noteOn(0, 54, 60);
      B_Compare[7] = (!(B_pinValues & 0x000080));
      B_chk[7] = true;
      if((tleds[0] & 128) == 128){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 7 & 0x1) == B_Compare[7] ){
        B_chk[7] = false;
      }
      if(!PedalStatus()) noteOff(0, 54, 0);
    }
    if(!(B_pinValues & 0x000100) && !B_chk[8]){
      noteOn(0, 56, 60);
      B_Compare[8] = (!(B_pinValues & 0x000100));
      B_chk[8] = true;
      if((tleds[1] & 1) == 1){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 8 & 0x1) == B_Compare[8] ){
        B_chk[8] = false;
      }
      if(!PedalStatus()) noteOff(0, 56, 0);
    }
    if(!(B_pinValues & 0x000200) && !B_chk[9]){
      noteOn(0, 58, 60);
      B_Compare[9] = (!(B_pinValues & 0x000200));
      B_chk[9] = true;
      if((tleds[1] & 2) == 2){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 9 & 0x1) == B_Compare[9] ){
        B_chk[9] = false;
      }
      if(!PedalStatus()) noteOff(0, 58, 0);
    }
    if(!(B_pinValues & 0x000400) && !B_chk[10]){
      noteOn(0, 61, 60);
      B_Compare[10] = (!(B_pinValues & 0x000400));
      B_chk[10] = true;
      if((tleds[1] & 4) == 4){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 10 & 0x1) == B_Compare[10] ){
        B_chk[10] = false;
      }
      if(!PedalStatus()) noteOff(0, 61, 0);
    }
    if(!(B_pinValues & 0x000800) && !B_chk[11]){
      noteOn(0, 63, 60);
      B_Compare[11] = (!(B_pinValues & 0x000800));
      B_chk[11] = true;
      if((tleds[1] & 8) == 8){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 11 & 0x1) == B_Compare[11] ){
        B_chk[11] = false;
      }
      if(!PedalStatus()) noteOff(0, 63, 0);
    }
    if(!(B_pinValues & 0x001000) && !B_chk[12]){
      noteOn(0, 66, 60);
      B_Compare[12] = (!(B_pinValues & 0x001000));
      B_chk[12] = true;
      if((tleds[1] & 16) == 16){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 12 & 0x1) == B_Compare[12] ){
        B_chk[12] = false;
      }
      if(!PedalStatus()) noteOff(0, 66, 0);
    }
    if(!(B_pinValues & 0x002000) && !B_chk[13]){
      noteOn(0, 68, 60);
      B_Compare[13] = (!(B_pinValues & 0x002000));
      B_chk[13] = true;
      if((tleds[1] & 32) == 32){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 13 & 0x1) == B_Compare[13] ){
        B_chk[13] = false;
      }
      if(!PedalStatus()) noteOff(0, 68, 0);
    }
    if(!(B_pinValues & 0x004000) && !B_chk[14]){
      noteOn(0, 70, 60);
      B_Compare[14] = (!(B_pinValues & 0x004000));
      B_chk[14] = true;
      if((tleds[1] & 64) == 64){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 14 & 0x1) == B_Compare[14] ){
        B_chk[14] = false;
      }
      if(!PedalStatus()) noteOff(0, 70, 0);
    }
    if(!(B_pinValues & 0x008000) && !B_chk[15]){
      noteOn(0, 73, 60);
      B_Compare[15] = (!(B_pinValues & 0x008000));
      B_chk[15] = true;
      if((tleds[1] & 128) == 128){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 15 & 0x1) == B_Compare[15] ){
        B_chk[15] = false;
      }
      if(!PedalStatus()) noteOff(0, 73, 0);
    }
    if(!(B_pinValues & 0x010000) && !B_chk[16]){
      noteOn(0, 75, 60);
      B_Compare[16] = (!(B_pinValues & 0x010000));
      B_chk[16] = true;
      if((tleds[2] & 1) == 1){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 16 & 0x1) == B_Compare[16] ){
        B_chk[16] = false;
      }
      if(!PedalStatus()) noteOff(0, 75, 0);
    }
    if(!(B_pinValues & 0x020000) && !B_chk[17]){
      noteOn(0, 78, 60);
      B_Compare[17] = (!(B_pinValues & 0x020000));
      B_chk[17] = true;
      if((tleds[2] & 2) == 2){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 17 & 0x1) == B_Compare[17] ){
        B_chk[17] = false;
      }
      if(!PedalStatus()) noteOff(0, 78, 0);
    }
    if(!(B_pinValues & 0x040000) && !B_chk[18]){
      noteOn(0, 80, 60);
      B_Compare[18] = (!(B_pinValues & 0x040000));
      B_chk[18] = true;
      if((tleds[2] & 4) == 4){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 18 & 0x1) == B_Compare[18] ){
        B_chk[18] = false;
      }
      if(!PedalStatus()) noteOff(0, 80, 0);
    }
    if(!(B_pinValues & 0x080000) && !B_chk[19]){
      noteOn(0, 82, 60);
      B_Compare[19] = (!(B_pinValues & 0x080000));
      B_chk[19] = true;
      if((tleds[2] & 8) == 8){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 19 & 0x1) == B_Compare[19] ){
        B_chk[19] = false;
      }
      if(!PedalStatus()) noteOff(0, 82, 0);
    }
    if(!(B_pinValues & 0x100000) && !B_chk[20]){
      noteOn(0, 85, 60);
      B_Compare[20] = (!(B_pinValues & 0x100000));
      B_chk[20] = true;
      if((tleds[2] & 16) == 16){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 20 & 0x1) == B_Compare[20] ){
        B_chk[20] = false;
      }
      if(!PedalStatus()) noteOff(0, 85, 0);
    }
    if(!(B_pinValues & 0x200000) && !B_chk[21]){
      noteOn(0, 87, 60);
      B_Compare[21] = (!(B_pinValues & 0x200000));
      B_chk[21] = true;
      if((tleds[2] & 32) == 32){
        matching_count++;
      }
    } else { 
      if( (B_read_shift_regs() >> 21 & 0x1) == B_Compare[21] ){
        B_chk[21] = false;
      }
      if(!PedalStatus()) noteOff(0, 87, 0);
    }

  //////////////////////////////////////////
  // 기본연주 PACKET 전송 코드 자리
  //////////////////////////////////////////
  if(m_count == matching_count && flag==true) {
    int cnt;
    
    flag = false;
    
    m_count = 0;

    for(cnt = 0; cnt < 4; cnt++) leds[cnt] = 0;
    for(cnt = 0; cnt < 4; cnt++) tleds[cnt] = 0;

    Serial2.write("BASIC");
    
    updateHelperRegister();
  }
  
  matching_count=0;

	/// KEYS END ////////////////////////////////////////////////////////////////////////////////////////

	if(W_pinValues != W_oldPinValues)
	{  
#ifdef DISPLAY_SERIAL
		Serial.print("*Pin value change detected*\r\n");
		Serial.print("Pin States: ");
		Serial.println(W_pinValues);    
#endif
		W_oldPinValues = W_pinValues;
	}

	if(B_pinValues != B_oldPinValues)
	{
#ifdef DISPLAY_SERIAL
		Serial.print("*Pin value change detected*\r\n");
		Serial.print("Pin States: ");
		Serial.println(B_pinValues);    
#endif
		B_oldPinValues = B_pinValues;
	}

	//delay(W_POLL_DELAY_MSEC);
}

void getMessage() { // 미구현  
  //수신 대기 - Packet가 없을시 대기

  
  /*
  while(!Serial2.available()) {
    talkMIDI(0xB0, 0, 0x00); //Default bank GM1
    talkMIDI(0xC0, INSTRUMENT, 0);

    W_pinValues = W_read_shift_regs();
    B_pinValues = B_read_shift_regs();

    PlayingWithKeys();

    //delay(1);
  }
  
	// 아두이노 수신부.
	while(Serial2.available()) {
		char data = Serial2.read();
		//Serial.write(data);
		buf[bufPosition++] = data;

    // 수신 데이터 크기 조절
    //if(bufPosition >= BUF) bufPosition = 0;
    if(data == '\0') break;
    delay(1);
	}

 delay(1);
  */
  // 정지 버튼 누를시 LED 초기화
 if(!strcmp(buf, "STOP")) {
  int cnt;
  
  for(cnt = 0; cnt < 4; cnt++) leds[cnt] = 0;
  for(cnt = 0; cnt < 4; cnt++) tleds[cnt] = 0;

  flag = false;
  
  matching_count = 0;
  m_count = 0;

  bufPosition = 0;
  
  updateHelperRegister();
  }
  // 자동 연주 모드
 else if(buf[0] == '1') {
  int note_octave = 0;
  int note_syllable_arrnum = 0;
  int cnt;
  
  Serial2.write("AUTO");
  
  for(cnt = 0; cnt < 4; cnt++) leds[cnt] = 0;
  for(cnt = 0; cnt < 4; cnt++) tleds[cnt] = 0;
  
  light_leds();
  updateHelperRegister();
  
  sound();

  note_syllable_arrnum = 0;
 }
 // 기본 연습 모드
 else if( buf[0] == '0') {
  flag=true;

  if(buf[2] != '0') m_count++;
  if(buf[7] != '0') m_count++;

  light_leds();
  updateHelperRegister();
 }

  Serial.print("buf = "); Serial.print(buf); Serial.print(" ");
  Serial.print("mcount = "); Serial.print(m_count); Serial.print(", matching_count = "); Serial.print(matching_count); Serial.print(" ");
  Serial.print("leds => "); Serial.print(leds[0]); Serial.print(" "); Serial.print(leds[1]); Serial.print(" "); Serial.print(leds[2]); Serial.print(" "); Serial.print(leds[3]); Serial.print(" ");
  Serial.print("tleds => "); Serial.print(tleds[0]); Serial.print(" "); Serial.print(tleds[1]); Serial.print(" "); Serial.println(tleds[2]);
  
  // 버퍼 초기화
  //buf[bufPosition] = '\0';
  for(int i=0; i<BUF; i++) buf[i] = 0;
  bufPosition = 0;
}

void light_leds() {
  upper_leds();
  lower_leds();
}

void upper_leds() {
  if(buf[2] == '2') {
    if(buf[4] == 'c') { if(buf[5] == '#') tleds[0] += 1; else leds[0] += 1; }
    else if(buf[4] == 'd') { if(buf[5] == '#') tleds[0] += 2; else if(buf[5] == '$') tleds[0] += 1; else leds[0] += 2; }
    else if(buf[4] == 'e') { if(buf[5] == '$') tleds[0] += 2; else leds[0] += 4; }
    else if(buf[4] == 'f') { if(buf[5] == '#') tleds[0] += 4; else leds[0] += 8; }
    else if(buf[4] == 'g') { if(buf[5] == '#') tleds[0] += 8; else if(buf[5] == '$') tleds[0] += 4; else leds[0] += 16; }
    else if(buf[4] == 'a') { if(buf[5] == '#') tleds[0] += 16; else if(buf[5] == '$') tleds[0] += 8; else leds[0] += 32; }
    else if(buf[4] == 'b') { if(buf[5] == '$') tleds[0] += 16; else leds[0] += 64; }
  }
  else if(buf[2] == '3') {
    if(buf[4] == 'c') { if(buf[5] == '#') tleds[0] += 32; else leds[0] += 128; }
    else if(buf[4] == 'd') { if(buf[5] == '#') tleds[0] += 64; else if(buf[5] == '$') tleds[0] += 32; else leds[1] += 1; }
    else if(buf[4] == 'e') { if(buf[5] == '$') tleds[0] += 64; else leds[1] += 2; }
    else if(buf[4] == 'f') { if(buf[5] == '#') tleds[0] += 128; else leds[1] += 4; }
    else if(buf[4] == 'g') { if(buf[5] == '#') tleds[0] += 256; else if(buf[5] == '$') tleds[0] += 128; else leds[1] += 8; }
    else if(buf[4] == 'a') { if(buf[5] == '#') tleds[0] += 512; else if(buf[5] == '$') tleds[0] += 256; else leds[1] += 16; }
    else if(buf[4] == 'b') { if(buf[5] == '$') tleds[0] += 512; else leds[1] += 32; }
  }
  else if(buf[2] == '4') {
    if(buf[4] == 'c') { if(buf[5] == '#') tleds[0] += 1024; else leds[1] += 64; }
    else if(buf[4] == 'd') { if(buf[5] == '#') tleds[0] += 2048; else if(buf[5] == '$') tleds[0] += 1024; else leds[1] += 128; }
    else if(buf[4] == 'e') { if(buf[5] == '$') tleds[0] += 2048; else leds[2] += 1; }
    else if(buf[4] == 'f') { if(buf[5] == '#') tleds[0] += 4096; else leds[2] += 2; }
    else if(buf[4] == 'g') { if(buf[5] == '#') tleds[0] += 8192; else if(buf[5] == '$') tleds[0] += 4096; else leds[2] += 4; }
    else if(buf[4] == 'a') { if(buf[5] == '#') tleds[0] += 16384; else if(buf[5] == '$') tleds[0] += 8192; else leds[2] += 8; }
    else if(buf[4] == 'b') { if(buf[5] == '$') tleds[0] += 16384; else leds[2] += 16; }
  }
  else if(buf[2] == '5') {
    if(buf[4] == 'c') { if(buf[5] == '#') tleds[0] += 32768; else leds[2] += 32; }
    else if(buf[4] == 'd') { if(buf[5] == '#') tleds[0] += 65536; else if(buf[5] == '$') tleds[0] += 32768; else leds[2] += 64; }
    else if(buf[4] == 'e') { if(buf[5] == '$') tleds[0] += 65536; else leds[2] += 128; }
    else if(buf[4] == 'f') { if(buf[5] == '#') tleds[0] += 131072; else leds[3] += 1; }
    else if(buf[4] == 'g') { if(buf[5] == '#') tleds[0] += 262144; else if(buf[5] == '$') tleds[0] += 131072; else leds[3] += 2; }
    else if(buf[4] == 'a') { if(buf[5] == '#') tleds[0] += 524288; else if(buf[5] == '$') tleds[0] += 262144; else leds[3] += 4; }
    else if(buf[4] == 'b') { if(buf[5] == '$') tleds[0] += 524288; else leds[3] += 8; }
  }
  else if(buf[2] == '6') {
    if(buf[4] == 'c') { if(buf[5] == '#') tleds[0] += 1048576; else leds[3] += 16; }
    else if(buf[4] == 'd') { if(buf[5] == '#') tleds[0] += 2097152; else if(buf[5] == '$') tleds[0] += 1048576; else leds[3] += 32; }
    else if(buf[4] == 'e') { if(buf[5] == '$') tleds[0] += 2097152; else leds[3] += 64; }
    else if(buf[4] == 'f') leds[3] += 128;
  }
  else {
    int cnt;
    for(cnt = 0; cnt < 4; cnt++) leds[cnt] += 0;
    for(cnt = 0; cnt < 4; cnt++) tleds[cnt] += 0;
  }
}

void lower_leds() {
  if(buf[7] == '2') {
    if(buf[9] == 'c') { if(buf[10] == '#') tleds[0] += 1; else leds[0] += 1; }
    else if(buf[9] == 'd') { if(buf[10] == '#') tleds[0] += 2; else if(buf[10] == '$') tleds[0] += 1; else leds[0] += 2; }
    else if(buf[9] == 'e') { if(buf[10] == '$') tleds[0] += 2; else leds[0] += 4; }
    else if(buf[9] == 'f') { if(buf[10] == '#') tleds[0] += 4; else leds[0] += 8; }
    else if(buf[9] == 'g') { if(buf[10] == '#') tleds[0] += 8; else if(buf[10] == '$') tleds[0] += 4; else leds[0] += 16; }
    else if(buf[9] == 'a') { if(buf[10] == '#') tleds[0] += 16; else if(buf[10] == '$') tleds[0] += 8; else leds[0] += 32; }
    else if(buf[9] == 'b') { if(buf[10] == '$') tleds[0] += 16; else leds[0] += 64; }
  }
  else if(buf[7] == '3') {
    if(buf[9] == 'c') { if(buf[10] == '#') tleds[0] += 32; else leds[0] += 128; }
    else if(buf[9] == 'd') { if(buf[10] == '#') tleds[0] += 64; else if(buf[10] == '$') tleds[0] += 32; else leds[1] += 1; }
    else if(buf[9] == 'e') { if(buf[10] == '$') tleds[0] += 64; else leds[1] += 2; }
    else if(buf[9] == 'f') { if(buf[10] == '#') tleds[0] += 128; else leds[1] += 4; }
    else if(buf[9] == 'g') { if(buf[10] == '#') tleds[0] += 256; else if(buf[10] == '$') tleds[0] += 128; else leds[1] += 8; }
    else if(buf[9] == 'a') { if(buf[10] == '#') tleds[0] += 512; else if(buf[10] == '$') tleds[0] += 256; else leds[1] += 16; }
    else if(buf[9] == 'b') { if(buf[10] == '$') tleds[0] += 512; else leds[1] += 32; }
  }
  else if(buf[7] == '4') {
    if(buf[9] == 'c') { if(buf[10] == '#') tleds[0] += 1024; else leds[1] += 64; }
    else if(buf[9] == 'd') { if(buf[10] == '#') tleds[0] += 2048; else if(buf[10] == '$') tleds[0] += 1024; else leds[1] += 128; }
    else if(buf[9] == 'e') { if(buf[10] == '$') tleds[0] += 2048; else leds[2] += 1; }
    else if(buf[9] == 'f') { if(buf[10] == '#') tleds[0] += 4096; else leds[2] += 2; }
    else if(buf[9] == 'g') { if(buf[10] == '#') tleds[0] += 8192; else if(buf[10] == '$') tleds[0] += 4096; else leds[2] += 4; }
    else if(buf[9] == 'a') { if(buf[10] == '#') tleds[0] += 16384; else if(buf[10] == '$') tleds[0] += 8192; else leds[2] += 8; }
    else if(buf[9] == 'b') { if(buf[10] == '$') tleds[0] += 16384; else leds[2] += 16; }
  }
  else if(buf[7] == '5') {
    if(buf[9] == 'c') { if(buf[10] == '#') tleds[0] += 32768; else leds[2] += 32; }
    else if(buf[9] == 'd') { if(buf[10] == '#') tleds[0] += 65536; else if(buf[10] == '$') tleds[0] += 32768; else leds[2] += 64; }
    else if(buf[9] == 'e') { if(buf[10] == '$') tleds[0] += 65536; else leds[2] += 128; }
    else if(buf[9] == 'f') { if(buf[10] == '#') tleds[0] += 131072; else leds[3] += 1; }
    else if(buf[9] == 'g') { if(buf[10] == '#') tleds[0] += 262144; else if(buf[10] == '$') tleds[0] += 131072; else leds[3] += 2; }
    else if(buf[9] == 'a') { if(buf[10] == '#') tleds[0] += 524288; else if(buf[10] == '$') tleds[0] += 262144; else leds[3] += 4; }
    else if(buf[9] == 'b') { if(buf[10] == '$') tleds[0] += 524288; else leds[3] += 8; }
  }
  else if(buf[7] == '6') {
    if(buf[9] == 'c') { if(buf[10] == '#') tleds[0] += 1048576; else leds[3] += 16; }
    else if(buf[9] == 'd') { if(buf[10] == '#') tleds[0] += 2097152; else if(buf[10] == '$') tleds[0] += 1048576; else leds[3] += 32; }
    else if(buf[9] == 'e') { if(buf[10] == '$') tleds[0] += 2097152; else leds[3] += 64; }
    else if(buf[9] == 'f') leds[3] += 128;
  }
  else {
    int cnt;
    for(cnt = 0; cnt < 4; cnt++) leds[cnt] += 0;
    for(cnt = 0; cnt < 4; cnt++) tleds[cnt] += 0;
  }
}

void sound() {
  up_sound();
  down_sound();
}

void up_sound() {
  int s;
  if(buf[2] != '0') {
    s = ((octave(buf[2]) * 12) + 12) + syllable(buf[4]);
    noteOn(0, s, 60);  // MIDI Interface sound routin

    /*
    Serial.print(s);
    Serial.println();
    */
  }
}

void down_sound() {
  int s;
  if(buf[9] != '0') {
    s = ((octave(buf[7]) * 12) + 12) + syllable(buf[9]);
    noteOn(0, s, 60);  // MIDI Interface sound routin

    /*
    Serial.write(s);
    Serial.println();
    */
  }
}

int octave(char c) {
  int s;
  switch(c) {
    case '2' :
    s = 2;
    break;

    case '3' :
    s = 3;
    break;

    case '4' :
    s = 4;
    break;

    case '5' :
    s = 5;
    break;

    case '6' :
    s = 6;
    break;
  }

  return s;
}

int syllable(char c) {
  int s;
  
  switch(c) {
    case 'c':
    s = 0;
    break;
    
    case 'd' :
    s = 2;
    break;

    case 'e' :
    s = 4;
    break;

    case 'f' :
    s = 5;
    break;

    case 'g' :
    s = 7;
    break;

    case 'a' :
    s = 9;
    break;

    case 'b' :
    s = 11;
    break;
    }

    return s;
}

  // 건반 LED 조절
void updateHelperRegister() {
	digitalWrite(WH_latchPin, LOW);
	shiftOut(WH_dataPin, WH_clockPin, LSBFIRST, leds[0]);
	shiftOut(WH_dataPin, WH_clockPin, LSBFIRST, leds[1]);
	shiftOut(WH_dataPin, WH_clockPin, LSBFIRST, leds[2]);
	shiftOut(WH_dataPin, WH_clockPin, LSBFIRST, leds[3]);
	digitalWrite(WH_latchPin, HIGH);

  digitalWrite(BL_latchPin, LOW);
  shiftOut(BL_dataPin, BL_clockPin, LSBFIRST, tleds[0]);
  shiftOut(BL_dataPin, BL_clockPin, LSBFIRST, tleds[1]);
  shiftOut(BL_dataPin, BL_clockPin, LSBFIRST, tleds[2]);
  digitalWrite(BL_latchPin, HIGH);
}

void setup()
{
	Serial.begin(57600);
	Serial2.begin(9600);
	mySerial.begin(31250);
	bufPosition = 0;

	pinMode(resetMIDI, OUTPUT);
	digitalWrite(resetMIDI, LOW);
	delay(100);
	digitalWrite(resetMIDI, HIGH);
	delay(100);
	talkMIDI(0xB0, 0x07, MASTER_VOLUME);

	pinMode(A_Pedal, INPUT);

	pinMode(W_ploadPin, OUTPUT);
	pinMode(W_clockEnablePin, OUTPUT);
	pinMode(W_clockPin, OUTPUT);
	pinMode(W_dataPin, INPUT);

	digitalWrite(W_clockPin, LOW);
	digitalWrite(W_ploadPin, HIGH);

	pinMode(B_ploadPin, OUTPUT);
	pinMode(B_clockEnablePin, OUTPUT);
	pinMode(B_clockPin, OUTPUT);
	pinMode(B_dataPin, INPUT);

	digitalWrite(B_clockPin, LOW);
	digitalWrite(B_ploadPin, HIGH);

	pinMode(WH_latchPin, OUTPUT);
	pinMode(WH_clockPin, OUTPUT);
	pinMode(WH_dataPin, OUTPUT);
  // 1030추가
  pinMode(BL_latchPin, OUTPUT);
  pinMode(BL_clockPin, OUTPUT);
  pinMode(WH_dataPin, OUTPUT);
  //
  
	W_pinValues = W_read_shift_regs();
	B_pinValues = B_read_shift_regs();

	W_oldPinValues = W_pinValues;
	B_oldPinValues = B_pinValues;
}

void loop()
{
  if(Serial2.available()){
    char data = Serial2.read();
    buf[bufPosition++] = data;

    
    // 수신 데이터 크기 조절
    if(bufPosition >= 11 || strcmp(buf,"STOP")==0) {
      bufPosition = 0;
      getMessage();
    }
    //if(data == '\0') break;
    delay(1);
  }else{
      talkMIDI(0xB0, 0, 0x00); //Default bank GM1
      talkMIDI(0xC0, INSTRUMENT, 0);
  
      W_pinValues = W_read_shift_regs();
      B_pinValues = B_read_shift_regs();
  
      PlayingWithKeys();
      
      //delay(1);
  }
	
	
	//getMessage();
}
