#include "SoftwareSerial.h"
#define WIFISSID "Charishma Patel" // WIFI Username
#define WIFIPASS "charishmapatel" // WIFI Password
#define SERVERIP "data.sparkfun.com"// Server to post the update.
#define POSTURL "POST
/input/RMxD26WKzYH6dlaA159A?private_key=lzE2M5kergt4Dn1EGKzE&amp;balance=&amp;
cardno="
#include &lt;SPI.h&gt;// Include SPI Bus
#include &lt;MFRC522.h&gt; //Include MRFC522 Library
#include &lt;Servo.h&gt; // Include Servo Control Library
Servo myservo;
int pos=0;
#define SS_PIN 10
#define RST_PIN 5
MFRC522::MIFARE_Key key;
MFRC522 mfrc522 (SS_PIN, RST_PIN);
int led_granted = 5;
int led_denied = 6;
char st [20];
String cards[3]={"D4GF1901","58826A10","C6FF7A25"};
byte cardsize=sizeof(cards);
SoftwareSerial Serial1(2,3);
void sendToESP8266AndWaitForResponse (const char *cmd, const char *resp, bool
waitForResponse, int duration) {
String bytes;
Serial.print ("CMD: "); Serial.println(cmd);
do {
Serial.print(".");
Serial1.println (cmd);
delay(duration);
bytes = Serial1.readString();
} while ( (waitForResponse) &amp;&amp; (bytes.indexOf(resp)&lt;0));
Serial.print ("RESPONSE: ");
Serial.print (bytes.c_str());
Serial.println("\n------------------------------");
}
void connectToWIFI() {
String cwjapCmd = "AT+CWJAP=\";
cwjapCmd+=WIFISSID; cwjapCmd+="\",\"; cwjapCmd+=WIFIPASS;
cwjapCmd+="\";

sendToESP8266AndWaitForResponse (cwjapCmd.c_str(), "OK", true, 50);
}
void connectToServer() {
String cipstartCmd = "AT+CIPSTART=\"TCP\",\";
cipstartCmd += SERVERIP; cipstartCmd += "\",80";
sendToESP8266AndWaitForResponse (cipstartCmd.c_str(), "Linked", true, 10);
}
// the setup function runs once when you press reset or power the board
void setup() {
Serial.begin(9600);
Serial1.begin(9600);
// Setup Wifi as STA and connect to AP
sendToESP8266AndWaitForResponse ("AT+CWMODE=1", ", false, 5);
connectToWIFI();
sendToESP8266AndWaitForResponse ("AT+CIPMUX=0", "OK", false, 50);
pinMode (led_granted, OUTPUT);
pinMode (led_denied, OUTPUT);
myservo.attach(9); // Set servo atach which is connected to digital pin 3
myservo.write(90); // Gate Close Move the servo to the home position
Serial.begin (9600) ; // Enable Serial at baud rate 9600
SPI.begin (); // Starts SPI bus Commnunication
mfrc522.PCD_Init(); // Starts MFRC522
for (byte i = 0; i &lt; 6; i++)
{
key.keyByte[i] = 0xFF;
}
dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
Serial.println();
Serial.println ("Initializing your card reader ...");
Serial.println ("TOLL GATE @ BMS : Show your Card ...");
Serial.println();
}

void check_compatibility( MFRC522::PICC_Type piccType)
{
// Check for compatibility

if ( piccType != MFRC522::PICC_TYPE_MIFARE_MINI
&amp;&amp; piccType != MFRC522::PICC_TYPE_MIFARE_1K
&amp;&amp; piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
Serial.println(F("This sample only works with MIFARE Classic cards."));
return;
}
}
// the loop function runs over and over again forever
void loop() {
int flag=0;
if (! (mfrc522.PICC_IsNewCardPresent ())) // approximation of the card
{
return;
}
// Select one of the cards
Serial.println("8");
if (! (mfrc522.PICC_ReadCardSerial ()))
{
return;
}
Serial.print ("UID Tag:"); // Display the UID serial
String cont = ";
byte letter;
for (byte i = 0; i &lt;mfrc522.uid.size; i++)
{
Serial.print (mfrc522.uid.uidByte [i] &lt;0x10? "0": ");
Serial.print (mfrc522.uid.uidByte [i], HEX);
cont.concat (String (mfrc522.uid.uidByte [i] &lt;0x10? "0": "));
cont.concat (String (mfrc522.uid.uidByte [i], HEX));
}
Serial.println(F("Card UID:"));
dump_byte_array1(mfrc522.uid.uidByte, mfrc522.uid.size);
Serial.println();
MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
check_compatibility(piccType);
byte sector = 1;
byte blockAddr = 4;

byte trailerBlock = 7;
MFRC522::StatusCode status;
byte buffer[18];
byte size = sizeof(buffer);
status = (MFRC522::StatusCode)
mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock,
&amp;key, &amp;(mfrc522.uid));
if (status != MFRC522::STATUS_OK)
{
Serial.print(F("PCD_Authenticate() failed: "));
Serial.println(mfrc522.GetStatusCodeName(status));
return;
}
Serial.println ();
status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &amp;size);
if (status != MFRC522::STATUS_OK) {
// Serial.print(F("MIFARE_Read() failed: "));
// Serial.println(mfrc522.GetStatusCodeName(status));
}
//Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
dump_byte_array(buffer, 16); Serial.println();
Serial.println();
Serial.print ("message");
cont.toUpperCase();
for(byte i=0;i&lt;cardsize;i++)
{
if (cont.substring(0) == cards[i]) // Test if the Card been read
{
Serial.println ("Card - Access Granted");
Serial.println ();
delay (100);
status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer,
&amp;size);
if (status != MFRC522::STATUS_OK)
{
}
String a= ";
a+= cards[i];
int c;
c=a.toInt();

/*dump_byte_array(buffer, 16); Serial.println();
Serial.println();*/
Serial.print(F("Balance: "));
dump_byte_array1(buffer, 1);
Serial.println();
byte readCard[1];
readCard[0] = buffer[0];
if(readCard[0]&gt;0)
{
Serial.println("Access Allowed");
// myservo.write(0);
readCard[0]=readCard[0]-0x10;
Serial.print("After deduction balance is :");
Serial.println(readCard[0], HEX);
int bal= readCard[0];
byte dataBlock1[] = {readCard[0], 0x02, 0x03, 0x04, // 1, 2, 3, 4,
0x05, 0x06, 0x07, 0x08, // 5, 6, 7, 8,
0x08, 0x09, 0xff, 0x0b, // 9, 10, 255, 12,
0x0c, 0x0d, 0x0e, 0x0f // 13, 14, 15, 16
};
status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer,
&amp;size);
if (status != MFRC522::STATUS_OK)
{
}
dump_byte_array(buffer, 16);
dump_byte_array(dataBlock1, 16);
status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock1,
16);
if (status != MFRC522::STATUS_OK)
{
}
status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &amp;size);
if (status != MFRC522::STATUS_OK)
{
}
dump_byte_array(buffer, 16);

Serial.println();
mfrc522.PICC_HaltA();
mfrc522.PCD_StopCrypto1();
myservo.write (180);
delay(2000);
myservo.write(90);
digitalWrite (led_granted, LOW);
flag=1;
// Serial.println( "AT+CIPSTART=\"TCP\",\"80", "OK, Linked, true"
break;
connectToServer();
char cmd[200],cipsend[100];
sprintf (cmd, "%s%d%d HTTP/1.0\r\n\r\n Host:
%s\r\n\r\n",POSTURL,12,24,SERVERIP);
sprintf (cipsend, "AT+CIPSEND=%d",strlen(cmd));
sendToESP8266AndWaitForResponse (cipsend, "&gt;", true, 10);
sendToESP8266AndWaitForResponse (cmd, ", false, 10);
delay(10000);
break;
}
else
{
Serial.print("BALANCE LOW! NO ACCESS!");
break;
}
}
}
if(flag==0)
{
Serial.println ("Card - Access Denied");
Serial.println ();
for (int i = 1; i &lt;5; i++) // Flashing red LED
{
digitalWrite (led_denied, HIGH);
delay (20);
digitalWrite (led_denied, LOW);
delay (20);
}

}
delay (50);
mfrc522.PCD_Init();
delay(5000);
Serial.println ("TOLL GATE @ BMS : Show your Card ...");
}
void dump_byte_array1(byte *buffer, byte bufferSize)
{
for (byte i = 0; i &lt; bufferSize; i++)
{
Serial.print( buffer[i] &lt; 0x10 ? " 0" : " ");
Serial.print(buffer[i], HEX);
}
}
void dump_byte_array(byte *buffer, byte bufferSize)
{
for (byte i = 0; i &lt; bufferSize; i++)
{
(buffer[i] &lt; 0x10 ? " 0" : " ");
(buffer[i], HEX);
}
}