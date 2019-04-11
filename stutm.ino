#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#define RX 7
#define TX 6
#define PN532_SCK  (13)
#define PN532_MOSI (11)
#define PN532_SS   (10)
#define PN532_MISO (12)

Adafruit_PN532 nfc(PN532_SS);
#if defined(ARDUINO_ARCH_SAMD)
 #define Serial SerialUSB
#endif

int countTrueCommand;
int countTimeCommand; 
String ReadLine;
String ReadLine2;
String AP     = "ESSID";
String PASS   = "WIFIPASWORD";
String HOST   = "192.168.1.69";
String PORT   = "80";
boolean found = false;

SoftwareSerial esp8266(RX,TX);

void setup() {
	#ifndef ESP8266
		while (!Serial);
	#endif
	Serial.begin(115200);
	#ifndef ESP8266
		while (!Serial);
	#endif
	esp8266.begin(115200);
	sendCommand("AT",5,"OK");
	sendCommand("AT+RST",5,"OK");
	sendCommand("AT+CWMODE=1",5,"OK");
	sendCommand("AT+CWJAP_CUR=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
	Serial.begin(115200);
	Serial.println("Hello!");
	nfc.begin();
	uint32_t versiondata = nfc.getFirmwareVersion();
	if (! versiondata){
		Serial.print("Didn't find PN53x board");
		while (1);
	}
	Serial.print("Found chip PN5");
	Serial.println((versiondata>>24) & 0xFF, HEX); 
	Serial.print("Firmware ver. ");
	Serial.print((versiondata>>16) & 0xFF, DEC); 
	Serial.print('.');
	Serial.println((versiondata>>8) & 0xFF, DEC);
	nfc.SAMConfig();
	Serial.println("Waiting for an ISO14443A Card ...");
}

void loop() {
	uint8_t success;
	uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
	uint8_t uidLength;
	success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
	if (success) {
		Serial.print(uidLength, DEC);
		Serial.println(" bytes");
		Serial.print("  UID Value: ");
		nfc.PrintHex(uid, uidLength);
		if (uidLength == 7) {      
			uint8_t data[32];
		  	success = nfc.mifareultralight_ReadPage (4, data);
		  	if(success = "39 32 2E 31 36 38 2E 31 2E 39 35 2F 68 75 62 2E"){
		    	String getData = "GET /hub.php";
		    	sendCommand("AT+CIPMUX=1",5,"OK");
		    	sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
		    	sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
		    	esp8266.println(getData);
		    	delay(1500);
		    	countTrueCommand++;
		    	sendCommand("AT+CIPCLOSE=0",5,"OK");
		   	} else {
				Serial.print("NO");
		   	}
		}
	}
}

void sendCommand(String command, int maxTime, char readReplay[]) {
	Serial.print(command);
  	Serial.print("\r\n");
  	while(countTimeCommand < (maxTime*1)) {
  		esp8266.println(command);
  		if(esp8266.find(readReplay)) {
			found = true;
			break;
		}
		countTimeCommand++;
	}
	
	if(found == true) {
		countTrueCommand++;
		countTimeCommand = 0;
	}
  
	if(found == false){
		Serial.println("Fail");
		countTrueCommand = 0;
		countTimeCommand = 0;
	}
  
	found = false; 
}
