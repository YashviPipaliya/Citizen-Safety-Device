#define USE_ARDUINO_INTERRUPTS true // Set-up low-level interrupts for most
acurate BPM math.
#include <LiquidCrystal.h>
#include <PulseSensorPlayground.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
//defining RX and TX for ESP8266
#define RX 13
#define TX 6
//user's friends' numbers
long numbers[] = {9426684641, 9428492817, 9909414845, 6359506664,
8000940593};
//setting up variables for GPS
int state = 0;
const int pin = 9;
float gpslat, gpslon;
TinyGPS gps;
SoftwareSerial sgps(4, 5);
SoftwareSerial sgsm(2, 3);
//setting up variables for the ESP8266 module and creating an object for
the same
String AP = "One plus 8T"; // AP NAME
String PASS = "hijjok"; // AP PASSWORD
String API = "IKVU9ZNVRD26QDK8"; // Write API KEY
String HOST = "api.thingspeak.com";
String PORT = "80";
String field = "field1";
int countTrueCommand;
int countTimeCommand;
boolean found = false;
int dangerBit = 0;
SoftwareSerial esp8266(RX,TX);
//initialising object for LCD display
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
// constants won't change. They're used here to set pin numbers:
const int buttonPin = 8; // the number of the pushbutton pin
//const int ledPin = 13; // the number of the LED pin
int buzzerPin = 7;
int counter = 0;
signed short minutes, secondes;
char timeline[16];
const int PulseWire = 0; // PulseSensor PURPLE WIRE connected to ANALOG PIN
0
const int LED13 = 13; // The on-board Arduino LED, close to PIN 13.
// variables will change:
int buttonState = 0; // variable for reading the pushbutton status
int Threshold = 300; // Determine which Signal to "count as a beat" and
which to ignore.
PulseSensorPlayground pulseSensor; // Creates an instance of the
PulseSensorPlayground
void setup() {
// initialize the buzzer pin as output:
pinMode(buzzerPin, OUTPUT);
//pinMode(ledPin, OUTPUT);
// initialize the pushbutton pin as an input:
pinMode(buttonPin, INPUT);
// initialize the pushbutton pin as an input:
Serial.begin(9600); // For Serial Monitor
//initialise GSM and GPS objects
sgsm.begin(9600);
sgps.begin(9600);
//configuring the ESP8266 module by setting up to send values
esp8266.begin(115200);
sendCommand("AT",5,"OK");
sendCommand("AT+CWMODE=1",5,"OK");
sendCommand("AT+CWJAP=""+ AP +"",""+ PASS +""",20,"OK");
// Configure the PulseSensor object, by assigning our variables to it.
pulseSensor.analogInput(PulseWire);
pulseSensor.blinkOnPulse(LED13); //auto-magically blink Arduino's LED with
heartbeat.
pulseSensor.setThreshold(Threshold);
// Double-check the "pulseSensor" object was created and "began" seeing a
signal.
if (pulseSensor.begin()) {
Serial.println("We created a pulseSensor Object !"); //This prints one time
at Arduino power-up, or on Arduino reset.
}
//initialises LCD
lcd.begin(16, 2);
lcd.print("Safety Device:");
}
void loop() {
buttonState = digitalRead(buttonPin);
int myBPM = pulseSensor.getBeatsPerMinute(); // Calls function on our
pulseSensor object that returns BPM as an "int".
// "myBPM" hold this BPM value now.
//if (pulseSensor.sawStartOfBeat()) { // Constantly test to see if "a beat
happened".
//Serial.println("♥ A HeartBeat Happened ! "); // If test is "true", print
a message "a heartbeat happened".
//Serial.print("BPM: "); // Print phrase "BPM: "
//Serial.println(myBPM);} // Print the value inside of myBPM.
// check if user is unsafe
if ((buttonState == LOW)||(myBPM>200)||(bpmSpike)) {
digitalWrite (buzzerPin, LOW);
delay(1000);
counter ++;
printf( "count =" + counter);
if(counter > 4){
dangerBit = 1;//to update the ThingSpeak LED
sendNumbers(numbers);// to send SOS texts
lcd.setCursor(0, 1);
sprintf(timeline,"DANGER");// to update the danger status on LCD
lcd.print(timeline);
}
// turn LED on:
//digitalWrite(ledPin, HIGH);
}
//user is safe
if ((buttonState == HIGH)&&((myBPM<150)||(bpmSpike)) {
// turn LED off:
counter = 0;
digitalWrite (buzzerPin, HIGH);
lcd.clear();
lcd.print("Safety Device:");
delay(1000);
// digitalWrite(ledPin, LOW);
}
String getData = "GET /update?api_key="+ API +"&"+ "field1"
+"="+String(myBPM)+"&"+"field2"+"="+String(counter);
sendCommand("AT+CIPMUX=1",5,"OK");
sendCommand("AT+CIPSTART=0,"TCP",""+ HOST +"","+ PORT,15,"OK");
sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
esp8266.println(getData);
delay(500);countTrueCommand++; // delay for sending data, net delay needed
of 1.5 seconds so this is deducting the 1s from before.
sendCommand("AT+CIPCLOSE=0",5,"OK");
}
void beep(unsigned char delayms) { // Created a
function for beep
analogWrite(buzzerPin, 20); // This
will set pin 11 to high
delay(delayms);
// Giving a delay
analogWrite(buzzerPin ,0); // This
will set pin 11 to LOW
delay(delayms);
// Giving a delay
}
boolean bpmSpike(int myBPM1){ // function to check spike in heartbeat
int myBPM2 = pulseSensor.getBeatsPerMinute(); //to check the current
heartbeat against the previous one
boolean check = false;
if(myBPM2 - myBPM1 > 50) check = true;//if spike is greater than 50, its
possible the person has faced difficulty.
return check;
}
void sendCommand(String command, int maxTime, char readReplay[]) {//
function to send command through ESP8266
Serial.print(countTrueCommand);
Serial.print(". at command => ");
Serial.print(command);
Serial.print(" ");
while(countTimeCommand < (maxTime*1))
{
esp8266.println(command);//at+cipsend
if(esp8266.find(readReplay))//ok
{
found = true;
break;
}
countTimeCommand++;
}
if(found == true)
{
Serial.println("OYI");
countTrueCommand++;
countTimeCommand = 0;
}
if(found == false)
{
Serial.println("Fail");
countTrueCommand = 0;
countTimeCommand = 0;
}
found = false;
}
void sendLocation(long phoneNumber){// function to send SOS messages to a
number
sgps.listen();
while (sgps.available())
{
int c = sgps.read();
if (gps.encode(c))
{
gps.f_get_position(&gpslat, &gpslon);
}
}
if (digitalRead(pin) == HIGH && state == 0) {
sgsm.listen();
sgsm.print("\r");
delay(1000);
sgsm.print("AT+CMGF=1\r");
delay(1000);
sgsm.print("AT+CMGS=\"+91" + String(phoneNumber) + "\"\r");
delay(1000);
//The text of the message to be sent.
sgsm.print("Latitude :");
sgsm.println(gpslat, 6);
sgsm.print("Longitude:");
sgsm.println(gpslon, 6);
delay(1000);
sgsm.write(0x1A);
delay(1000);
state = 1;
}
if (digitalRead(pin) == LOW) {
state = 0;
}
}
void sendNumbers(long numbers[]){
for(int i = 0; i++; i< numbers.length()){
sendLocation(numbers[i]);}
}
