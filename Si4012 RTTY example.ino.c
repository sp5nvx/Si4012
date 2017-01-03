/****************************************************************************
 *  Si4012 tools by SP5NVX 
 *  v. 1.0 (2014)
 *  sp5nvx@wp.pl
 ****************************************************************************/

#include <stdarg.h>
#include <Wire.h>

/****************************************************************************
 *  Global Macros & Definitions
 ****************************************************************************/

/////// Macro tools for bits (set and clear) 
#define BIT(x) (1 << (x)) 
#define SETBITS(x,y) ((x) |= (y)) 
#define CLEARBITS(x,y) ((x) &= (~(y))) 
#define SETBIT(x,y) SETBITS((x), (BIT((y)))) 
#define CLEARBIT(x,y) CLEARBITS((x), (BIT((y)))) 
#define BITSET(x,y) ((x) & (BIT(y))) 
#define BITCLEAR(x,y) !BITSET((x), (y)) 
#define BITSSET(x,y) (((x) & (y)) == (y)) 
#define BITSCLEAR(x,y) (((x) & (y)) == 0) 
#define BITVAL(x,y) (((x)>>(y)) & 1) 

/////// ***************************
/////// COMMAND: CHANGE_STATE    
#define MODE2 4, 0x60            
/////// ***************************
#define Idle1 0x00
#define Shutdown1 0x01
// --------------------------------
#define Standby2 0x00
#define Sensor2 0x01
#define Tune2 0x02

/////// ***************************
/////// PROPERTY: TUNE_INTERVAL
#define TUNE1INT 5, 0x11, 0x21
/////// ***************************
#define Tune1Sec2 0x00, 0x2
#define Tune1Sec10 0x00, 0x0A
#define Tune1Sec60 0x00, 0x3C

/////// ***************************
/////// PROPERTY: MODULATION_FSKDEV
#define MODUL2DEV 5, 0x11, 0x20
/////// ***************************
#define Ook1 0x00
#define Fsk1 0x01
// --------------------------------  
#define Fsk2Dev1ppm 0x01 
#define Fsk2Dev2ppm 0x02												   // FSK dev [ppm] http://www.jittertime.com/resources/ppmcalc.shtml 

/////// ***************************
/////// PROPERTY: CHIP_CONFIG
#define CHIP1CONFIG 4, 0x11, 0x10
/////// ***************************
#define UseXo1LsbFirst 0x0C
#define NoXo1LsbFirst 0x04

/////// ***************************
/////// PROPERTY: TX_FREQ
#define TX1FREQ 7, 0x11, 0x40
/* uint32_t to 4 bytes data tools macro */ 
 #define _U4BYTE1(w) ((unsigned char)(((uint32_t)(w) >> 24) & 0xFF)) 
 #define _U4BYTE2(w) ((unsigned char)(((uint32_t)(w) >> 16) & 0xFF))
 #define _U4BYTE3(w) ((unsigned char)(((uint32_t)(w) >>  8) & 0xFF))
 #define _U4BYTE4(w) ((unsigned char)(((uint32_t)(w) >>  0) & 0xFF))
#define Tx1Freq(hz)  _U4BYTE1(hz), _U4BYTE2(hz), _U4BYTE3(hz), _U4BYTE4(hz)

/////// ***************************
/////// PROPERTY: BITRATE_CONFIG 
#define BITRATE2CONFIG 6, 0x11, 0x31
/////// ***************************
/* uint16_t to 2 bytes data tools macro */
 #define _U2BYTE1(u) ((unsigned char)(((uint16_t)(u) >>  8) & 0xFF))
 #define _U2BYTE2(u) ((unsigned char)(((uint16_t)(u) >>  0) & 0xFF))
#define Data1Rate(i)  _U2BYTE1(i), _U2BYTE2(i)                             // In units of 100 bps, 1..1000 FSK 1..500 OOK
// -------------------------------- 
#define Ramp2Rate1us 0x01                                                  // Ramp rate in µs. 1, 2, 4, or 8 is supported, default 2
#define Ramp2Rate2us 0x02
#define Ramp2Rate4us 0x04
#define Ramp2Rate8us 0x08

/////// ***************************
/////// PROPERTY: XO_CONFIG
#define XO2CONFIG 8, 0x11, 0x50
/////// ***************************
#define Xo1Freq(hz)  _U4BYTE1(hz), _U4BYTE2(hz), _U4BYTE3(hz), _U4BYTE4(hz) // 10-13MHz
// --------------------------------
#define Xo2LowCapMore14pF 0x00                   // Cload > 14 pF
#define Xo2LowCapLess14pF 0x01

/////// ***************************
/////// PROPERTY: PA_CONFIG
#define PA5CONFIG 9, 0x11, 0x60                  // Read Si4012 calculator spreadsheet AN564                                                            
/////// ***************************#define Pa1MaxDrv0 0x00#define Pa1MaxDrv1 0x01                          // Default// --------------------------------#define Pa2Level0 0x20            #define Pa2Level1 0x40           #define Pa2Level2 0x60#define Pa2Level3 0x7F                           // Max// --------------------------------#define Pa3Cap(c) _U2BYTE1(c), _U2BYTE2(c)       // Default 128   max 511// --------------------------------
#define f4AlphaSteps(s) _U2BYTE2(s)              // Default 125   (0..250) // --------------------------------#define f5BetaSteps(s) _U2BYTE2(s)               // Default 127   (0..254)
/////// ***************************
/////// PROPERTY: FIFO_THRESHOLD
#define FIFO3THRESHOLD 6, 0x11, 0x30
/////// ***************************
#define Almost1Full(b) _U2BYTE2(b)               // Almost Full Threshold in bytes, default 0xF0
// ---------------------------
#define Almost2Empty(b) _U2BYTE2(b)              // Almost Empty Threshold in bytes, default 0x10
// ---------------------------
#define Thres3Control(b) _U2BYTE2(b)             // Threshold, when to start auto transmit, default 0x20

/////// ***************************
/////// COMMAND: SET_INT FIFO_INT
#define FIFO1INTERRUPTS 3, 0x63/////// ***************************              //B11111110#define Enable1int(bin) _U2BYTE2(bin)            //Underflow,Full,Empty,Overflow,PacketSent,LowBatt,1(tune),reserved/////// ***************************//#define SI4012_MODE_SELECTION    MODE, 1Idle, 2Standby
//#define SI4012_TUNE_INTERVAL     0x11, 0x21, 0x00, 0x0A                           // 10 sec
//#define SI4012_MODULATION_FSKDEV 0x11, 0x20, 0x01, 0x5D
//#define SI4012_CHIP_CONFIG       0x11, 0x10, 0x00
//#define SI4012_TX_FREQ           0x11, 0x40, 0x36, 0xAA, 0x24, 0x00
//#define SI4012_BITRATE_CONFIG    0x11, 0x31, 0x00, 0x60, 0x02
//#define SI4012_XO_CONFIG         0x11, 0x50, 0x00, 0x98, 0x96, 0x80, 0x00
//#define SI4012_PA_CONFIG         0x11, 0x60, 0x01, 0x4D, 0x00, 0x80, 0x7D, 0xCC
//#define SI4012_FIFO_THRESHOLD    0x11, 0x30, 0xF0, 0x37, 0x20
//#define SI4012_LED_INTENSITY     0x11, 0x11, 0x03
//#define SI4012_LED_CONTROL       0x13, 0x00
//#define SI4012_FIFO_INTERRUPTS   0x63, 0x08
//#define SI4012_LBD_CONFIG        0x11, 0x41, 0x09, 0xC4, 0x00, 0x3C 

#define MSG_LEN 100 
                     //$$SP5NVX,42,103345,52.506660,21.392179,159,12,9,5,1.29,1.40,0.31*0BD6
                   
char msg[MSG_LEN] = "\n$$SP5NVX,42,103345,52.506660,21.392179,159,12,9,5,1.29,1.40,0.31*0BD6\n.     ... ...";
/////// The boolean bit i from value v
#define BTB(v,i) ((v) & (1u << (i))) ? true : false
volatile bool INT_STATUS_CHANGE = false;
int INT_STATUS;

int SetRadio(int nargs, ...)
{	
	bool flag1=false;
    bool flag2=false;
	bool flag3=false;
	bool flag4=false;
	int b = 0; 
	va_list arglist;
	va_start(arglist, nargs); 
	Wire.beginTransmission(B1110000); // transmit to device 1110000
	for(int n = 1; n < nargs; n++) {
        b = va_arg(arglist, int);		  
		Wire.write(b); 
		///if(nargs==4 && n==1 && b==0x60) SETBIT(PORTC,1);
        if(n==1 && b == 0x11) flag1=true; 
		if(flag1 && n==2 && b == 0x50) flag2=true;  
		//if(flag2 &&	n==3 )  SETBIT(PORTC,1); flag3=true;
		//if(flag3 && n==4 && b==0x7F )	flag4=true;
		//if(flag4 && n==5 && b==510) SETBIT(PORTC,1); 
	}	
	Wire.endTransmission();           // stop transmitting
    va_end(arglist);
	delayMicroseconds(2);
	
	Wire.requestFrom(B1110000, 1);    // request 1 byte
    while(Wire.available())           // slave may send less than requested
    {   
       byte st = Wire.read();         // read status byte 19 err codes
	   if(st!=128){ 
		   
		   return -1;
	   }
	   else return nargs;             // ok
	}
	return -1;                        // no response	    
}

void setup()
{
	attachInterrupt(INT0, INT_RADIO, CHANGE);   // Set the radio Interrupts
	
	DDRD = DDRD | B10000000;		            // Set bit 8 D7 as output SDN radio	        
	PORTD = PORTD | B10000000;	                // Radio OFF (SHUTDOWN, Reset default values)
	delay(1);
	PORTD = PORTD & B01111111;                  // Radio ON (reset default settings) low bit 8 pin D7 on
	delay(50);
	int ret = 0;

	ret=SetRadio(MODE2, Idle1, Standby2);       // Idle1 | Shutdown1, Standby2 | Sensor2 | Tune2
	if(ret!=4) SETBIT(PORTC,1);

	ret=SetRadio(TUNE1INT, Tune1Sec2);          // Tune1Sec2 | Tune1Sec10 | Tune1Sec60
	if(ret!=5) SETBIT(PORTC,1);

	ret=SetRadio(MODUL2DEV, Fsk1, Fsk2Dev2ppm); // Ook1 | Fsk1, Fsk1Dev1ppm | Fsk1Dev2ppm | ...
	if(ret!=5) SETBIT(PORTC,1);  
	 
	ret=SetRadio(CHIP1CONFIG, UseXo1LsbFirst);  // UseXo1LsbFirst | NoXo1LsbFirst | ... 
	if(ret!=4) SETBIT(PORTC,1);  
	 
	ret=SetRadio(TX1FREQ, Tx1Freq(70170001));   // Tx1Freq([Hz]) 27..960 Hz (use balanced antenna matching)
	if(ret!=7) SETBIT(PORTC,1);  

	ret=SetRadio(BITRATE2CONFIG, Data1Rate(3), Ramp2Rate1us); // In units of 100 bps, 1..1000   1..500 OOK
	if(ret!=6) SETBIT(PORTC,1);  

	ret=SetRadio(XO2CONFIG, Xo1Freq(9999915), Xo2LowCapMore14pF);
	if(ret!=8) SETBIT(PORTC,1);  

	ret=SetRadio(PA5CONFIG, Pa1MaxDrv1, Pa2Level3, 400, f4AlphaSteps(0), 0); // AlphaSteps default 125 (0..250) 
	if(ret!=9) SETBIT(PORTC,1);  

	ret=SetRadio(FIFO3THRESHOLD, Almost1Full(125), Almost2Empty(20),  Thres3Control(5));
	if(ret!=6) SETBIT(PORTC,1);  

	ret=SetRadio(FIFO1INTERRUPTS, Enable1int(B11111110)); // Underf,Full,Empty,Overfl,Packet,Batt,1(tune),res
	if(ret!=3) SETBIT(PORTC,1);  

	SetFIFO(0,0,0);
}
void INT_RADIO()
{ 	 
	INT_STATUS_CHANGE = true;
}

long last_tx_time = 0L;
char testcount=49;
void loop()
{
	delay(100);
	if(INT_STATUS_CHANGE) 
		GetIntStatus();

	if((millis()*4)>2000 && (millis()*4)<2400){
		Data();		
		TX_Start(90*8,true);
		last_tx_time = millis()*4;
		delay(400);
	}

	if(((millis()*4)-last_tx_time) > 58000/3) // /3 datarate 3
	{
		testcount=49;
		Data();		
		TX_Start(90*8,true);
		last_tx_time = millis()*4;
		delay(400);
	}
}
void Blink(int n)
{
	for(int i=0;i<n;i++)
	{ SETBIT(PORTC,1); delay(50); CLEARBIT(PORTC,1); delay(50); }
}


void Data()
{      	
    bool clr = false;
	if(testcount==49) clr=true;
	msg[10]=char(testcount++);
	int 
	w = SetFIFO(0,23,clr);            // loading data 1/3 1/3 1/3  
	w+= SetFIFO(24,47,false);         /// TODO: czemu ³aduje tylko po 23 znaki???? <<<<<<<<<<<<<<<
	w+= SetFIFO(48,71,false);
				
	if(w==90) CLEARBIT(PORTC,1);
}

bool GetIntStatus()
{		
	INT_STATUS_CHANGE = false;
	// COMMAND: GET_INT_STATUS
    Wire.beginTransmission(B1110000); // transmit to device 1110000
    Wire.write(0x64);                 // cmmand GET_INT_STATUS
	Wire.endTransmission();           // stop transmitting
    delayMicroseconds(2);
	Wire.requestFrom(B1110000, 2);    // request 2 bytes
    while(Wire.available())           // slave may send less than requested
    {                                 // read status byte
	   if(Wire.read()==128){
          INT_STATUS = Wire.read();           
	   }else return false;
	}
	
	if(BTB(INT_STATUS,7)){ }
	if(BTB(INT_STATUS,6)){ Blink(2); }   // Almost Full
	if(BTB(INT_STATUS,5)){ SETBIT(PORTC,1); Data(); }  // Almost Empty
	if(BTB(INT_STATUS,4)){ Blink(3); }   // FIFO Overflow
	if(BTB(INT_STATUS,3)){ }
	if(BTB(INT_STATUS,2)){ }
	if(BTB(INT_STATUS,1)){ }
	if(BTB(INT_STATUS,0)){ }
    
	/* 7 iffunder—FIFO Underflow
       6 itxffafull—TX FIFO Almost Full
       5 itxffaem—TX FIFO Almost Empty    
       4 iffover—FIFO Overflow              (po za³adowaniu ponad 255 bytes)           
       3 ipksent—Packet Sent
       2 ilbd—Low Battery Detect
       1 itune—tune complete                (po ramce zapala siê z lekkim opóŸnieniem)
       0 ipor—Power On Reset
	*/	
	
	return true;
}


int SetFIFO(int from, int to, bool clear)
{	
    int Count=0;
	int w=0;
	if(clear){
	 delayMicroseconds(2);
	 Wire.beginTransmission(B1110000); // transmit to device 1110000
     Wire.write(0x65);                 // cmmand FiFo Clear
     Wire.endTransmission();       
     delayMicroseconds(2);
     Wire.requestFrom(B1110000, 1);    // request 1 byte
     while(Wire.available())           // slave may send less than requested
     {                                 // read status byte
	   if(Wire.read()!=128)
		   return -1;
	 }
	}
    if(from==0 && to==0) return 0;
    Wire.beginTransmission(B1110000); // transmit to device 1110000
    Wire.write(0x66);                 // cmmand FiFo Set
	byte c = 0;                       // 7N1 
	int b = 0;                        // bits 0..7 of c (out byte) 
	int i = from;                     // i chars 0..len-1
	int n = 0;                        // bits 0..6 of msg[] data
	do{    
                                      // set nth bit n 0..6
		if(n==0){                     // before msg[i] char
		   c &= ~(1u << b);           // set start bit 0
		   b++; 
		}		
		if(b>7) {b=0;w+=Wire.write(c);c=0;Count++;}
        
		if(BTB(msg[i],n)) c |= (1u << b);
		            else  c &= ~(1u << b);
	 	b++;                          // next bit of out   
		if(b>7) {b=0;w+=Wire.write(c);c=0;Count++;} 
		n++;		                  // next bit of msg[i]
		if(n>6)                       // finish msg[i] char
		{
			c |= (1u << b);           // stop bit 1
			b++;
			
			if(b>7) {b=0;w+=Wire.write(c);c=0;Count++;}
            c |= (1u << b);           // stop bit 2
			b++;
            
			n=0;
			i++;                      // next msg[i] char
		}                 
	    if(b>7) {b=0;w+=Wire.write(c);c=0;Count++;} 
		delayMicroseconds(2);
	}while(i<to+1);	

	
    Wire.endTransmission();       
    delayMicroseconds(2);
    Wire.requestFrom(B1110000, 1);    // request 1 byte
    while(Wire.available())           // slave may send less than requested
    { 		
       int st = Wire.read();          // read status byte
	   if(st!=128)
		   return -1;
	}    	
	return w;
}
int TX_Start(int32_t size, bool autotx)
{   
    int a = 0;
	if(autotx) a=4;
	unsigned char bytes[2];
    bytes[0] = (size >>  8) & 0xFF;
    bytes[1] =  size        & 0xFF;

    Wire.beginTransmission(B1110000); // transmit to device 1110000
    Wire.write(0x62);                 // cmmand TX
	Wire.write(bytes[0]);             // packet size 
    Wire.write(bytes[1]);             // packet size            
	Wire.write(a);                    // AutoTX State    4 - AutoTX and idle state after
    Wire.write(0);                    // idle mode 0 or sensor 1
    Wire.write(0);                    // DTMod: FIFO 0 CW 1 Test: 2 i 3 <------- mode !!! ///
	Wire.endTransmission();           // stop transmitting
    delayMicroseconds(2);
	Wire.requestFrom(B1110000, 2);    // request 2 bytes
    while(Wire.available())           // slave may send less than requested
    { 
       int st = Wire.read();          // read status byte
	   int s  = Wire.read();          // not sent packet size
	   if(st!=128)
		   return -1;
	   else return s;                 // ok
	}	
	return -1;
}
