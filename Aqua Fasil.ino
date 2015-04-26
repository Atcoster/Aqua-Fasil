#include <OneWire.h>
#include <DallasTemperature.h>
#include <MenuBackend.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <Wire.h>
#include <Stepper.h>
#include <FastIO.h>
#include <I2CIO.h>
#include <LCD.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>


//LCD defined vars
#define I2C_ADDR    0x27 // <<----- Add your address here.  Find it from I2C Scanner
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

// The temp sensor Data wire is plugged into port 9
#define ONE_WIRE_BUS 9

// all buttons list keyes ir
#define buttonPinUP 5316027  
#define buttonPinRight   553536955   
#define buttonPinDown    2747854299
#define buttonPinLeft    1386468383
#define buttonPinOK      3622325019

/*
#define buttonPinZero    465573243
#define buttonPinOne     3238126971
#define buttonPinTwo     2538093563
#define buttonPinThree   4039382595
#define buttonPinFour    2534850111
#define buttonPinFive    1033561079
#define buttonPinSix     1635910171
#define buttonPinSeven   2351064443
#define buttonPinEight   1217346747
#define buttonPinNine    71952287
#define buttonPinStar    851901943
#define buttonPinHashtag 1053031451
*/

// ir Universal Remote 2.0 app Android buttons
#define btnPinUP 3261853764  
#define btnPinRight   1400905448   
#define btnPinDown    3305092678
#define btnPinLeft    1972149634
#define btnPinOK      2331063592
#define btnPinSmart  1479585163

/*
    	This is the structure of the modelled menu	
    	Main menu
          > Info
          > Date
          > Clock
          > Temperature
          > Settings
          	> Rotate Rounds
          > Food
          > Timers
          > Help
          
          I've kept it simple for now, but will upgrade it later
*/

//this controls the menu backend and the event generation
MenuBackend menu = MenuBackend(menuUseEvent,menuChangeEvent);

	//beneath is the list of menu items needed to build the menu
	MenuItem mi_main = MenuItem("Main");
            MenuItem mi_info = MenuItem("Info");
            MenuItem mi_datetime = MenuItem("DateTime");
            MenuItem mi_temp = MenuItem("Temperature");
  	    MenuItem mi_settings = MenuItem("Settings");
                MenuItem mi_rotateRounds = MenuItem("RotateRounds");
            MenuItem mi_food = MenuItem("Food");
            MenuItem mi_timer = MenuItem("Timer");
    	    MenuItem mi_help = MenuItem("Help");
            MenuItem mi_credits = MenuItem("Credits");
        

// mac adress for the arduino ethernet shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// NTP Servers:
IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 103); // time-c.timefreq.bldrdoc.gov

        
//IR receiver vars
#define RECV_PIN 8
IRrecv irrecv(RECV_PIN);
decode_results results;


//used the Central European Time
const int timeZone = 2;     // Central European Time
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)


// current menu
const char* currentMenu = "";

//lcd
const int col = 16; //lcd total colloms(character)
const int row = 2; // lcd rows (lines)

// Aqua fasil animation speed:
const int s = 20;
int feededCounts = 0;

// steps per revolution (stepper motor)
const int stepsPerRevolution = 200;
bool isChangeRotation = false;
bool rotating;
int rotateSpeed = 60;
int rounds = 1;

// timer used for refreshing data
unsigned long previousMillis = 0; // last time update
long interval = 5000; // interval at which to do something (milliseconds)

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// initialize the stepper library on pins 34 through 40(digital on arduino mega):
Stepper myStepper(stepsPerRevolution, 34, 36, 38, 40);

LiquidCrystal_I2C lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);

EthernetUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
time_t prevDisplay = 0; // when the digital clock was displayed

/**************************
**some custom characters:**
**************************/
byte heart[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};

byte smiley[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b00000,
  0b10001,
  0b01110,
  0b00000
};

//fish for animation parts

byte fish1[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b10000,
  0b01000,
  0b10000,
  0b00000,
  0b00000
};

byte fish2[8] = {
  0b00000,
  0b00000,
  0b11000,
  0b11100,
  0b11010,
  0b11100,
  0b11000,
  0b00000
};

byte fish3[8] = {
  0b00000,
  0b00000,
  0b01110,
  0b11111,
  0b11110,
  0b11111,
  0b01110,
  0b00000
};

byte fish4[8] = {
  0b00000,
  0b00000,
  0b10000,
  0b11101,
  0b01111,
  0b11101,
  0b10000,
  0b00000
};

byte fish5[8] = {
  0b00000,
  0b01100,
  0b00110,
  0b00111,
  0b00011,
  0b00111,
  0b00110,
  0b01100
};

byte fish6[8] = {
  0b00000,
  0b00010,
  0b00011,
  0b00011,
  0b00000,
  0b00011,
  0b00011,
  0b00010
};

byte fish7[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

byte arrowUp[8] = {
  0b00000,
  0b00100,
  0b01110,
  0b11011,
  0b10001,
  0b00000,
  0b00000,
  0b00000
};

byte arrowDown[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b10001,
  0b11011,
  0b01110,
  0b00100,
  0b00000
};


/********************************************/
/********Start setup And loop methods********/
/********************************************/

void setup() // run once 
{  
  Serial.begin(9600);
  
  // my lcd is a 16x2 (16 char and 2 rows)
  lcd.begin (col, row); //
  
  
  // Switch on the backlight
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.clear();
  
  if (Ethernet.begin(mac) == 0) {
    // no point in carrying on, so do nothing forevermore:
    while (1) {
      Serial.println("Failed to configure Ethernet using DHCP");
      delay(10000);
    }
  }  
  
  Serial.println(Ethernet.localIP());
  
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  while(timeStatus()== timeNotSet); // wait until the time is set by the sync provider
  
  // set motor rotating to false 
  rotating = false;
  
 // set the speed 20 - 60 rpm:
  myStepper.setSpeed(rotateSpeed); //60rpm
  
  //enable ir sensor
  irrecv.enableIRIn();
  
  showLogo();
   
  lcd.createChar(8, arrowUp);
  lcd.createChar(9, arrowDown);
  
  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  // sensors.setResolution(myThermometer, 9);
  
  sensors.begin(); // begin the temp sensor
    
  menuSetup();
  
  menu.moveDown();
  menu.moveDown();

}

void loop()
{   
  unsigned long currentMillis = millis();
  
  if (irrecv.decode(&results)) { //we have received an IR
      readButtons();
      irrecv.resume();
      
      if(isChangeRotation){
          changeRotation();
      }   
      
      Serial.print(results.value);
      Serial.print("\n");
  }
     
      if(currentMillis - previousMillis > interval) {
         previousMillis = currentMillis;
         if(currentMenu == "Temperature"){
           showTemp();
         }
      }
 
       if(currentMenu == "DateTime"){
         if (timeStatus() != timeNotSet) {
            if (now() != prevDisplay) { //update the display only if time has changed
              prevDisplay = now();
              showDateTime();
            }
          }   
       }
       
       if(rotating){
           feedFish();              
           rotating =  !rotating;
       }

      
}

/**********************************************/
/*******End setup And loop methods*************/
/**********************************************/

//this function builds the menu and connects the correct items together
void menuSetup()
{        
        lcd.clear();
	lcd.write("Loading");
        for(int i = 0; i <= 6; i++){
          delay(300);
          lcd.write(".");
        }
        
	//add the file menu to the menu root
	menu.getRoot().add(mi_main);        
        
        //Main menu
	mi_main.addRight(mi_help);
            //scroll up and down
            mi_help.addBefore(mi_info);
            mi_help.addAfter(mi_datetime);
            mi_datetime.addAfter(mi_temp);
            mi_temp.addAfter(mi_settings);
            mi_settings.addAfter(mi_food);
                mi_settings.addRight(mi_rotateRounds);
            mi_food.addAfter(mi_timer);
            mi_timer.addAfter(mi_credits);
            mi_credits.addAfter(mi_info);
            mi_info.addAfter(mi_help);
            
        //Back to Main menu
        mi_info.addLeft(mi_main);
        mi_datetime.addLeft(mi_main);
        mi_temp.addLeft(mi_main);
        mi_settings.addLeft(mi_main);
        mi_food.addLeft(mi_main);
        mi_timer.addLeft(mi_main);
        mi_help.addLeft(mi_main);
        mi_credits.addLeft(mi_main);
        
        // back to Settings
        mi_rotateRounds.addLeft(mi_settings);
}


//used for refreshing or showing more info of that menu item if it has
void menuUseEvent(MenuUseEvent used)
{
    const char* currentUsed = used.item.getName();
    
    if (used.item == mi_temp) //comparison agains a known item
    {
      showTemp();
    }
    if (used.item == mi_rotateRounds)
    { 
      isChangeRotation = true;
      showRotateRounds();
    }
}

//go to menu item
void menuChangeEvent(MenuChangeEvent changed)
{  
  currentMenu = changed.to.getName();
  
  Serial.print(currentMenu);
  
  if(currentMenu == "Main"){
      showMain();
  }else if(currentMenu == "Info"){
      showInfo();
  }else if(currentMenu == "DateTime"){
      showDateTime();
  }else if(currentMenu == "Food"){
      showFeed();
  }else if(currentMenu == "Temperature"){
      showTemp();
  }else if(currentMenu == "Timer"){
      showTimer();
  }else if(currentMenu == "Settings"){
      showSettings();
  }else if(currentMenu == "Help"){
      showHelp();
  }else if(currentMenu == "Credits"){
      showCredits();
  }else if(currentMenu == "RotateRounds"){
      showRotateRounds();
  }
}

void readButtons(){  
  int currentBtnValue = results.value;
  
    /* enable when using arduino keyes ir
    switch (currentBtnValue){
      case 5316027:
          menu.moveUp();
        break;
      case 3622325019:
          menu.use();
        break;
      case 1386468383:
         menu.moveLeft();
        break;
      case 553536955:
        menu.moveRight();
        break;
      case 2747854299:
        menu.moveDown();
        break;
      case 851901943: // asteric button
         rotating = true;
         break;
    }*/
    
    //enable when using IR Remote 2.0 app ir
     switch (currentBtnValue){
      case 3261853764:
          menu.moveUp();
        break;
      case 2331063592:
          menu.use();
        break;
      case 1972149634:
         menu.moveLeft();
        break;
      case 1400905448:
        menu.moveRight();
        break;
      case 3305092678:
        menu.moveDown();
        break;
      case 1479585163: // smart btn
         rotating = true;
         break;
    }
    
    delay(500);
}


// show the logo and animation
void showLogo() {
  lcd.setCursor(3, 0);
  lcd.print("Aqua Fasil");  
  delay(s);
  AnimateFish();
  delay(s);
}

// Fish Animation
void AnimateFish() {
  lcd.createChar(1, fish1);
  lcd.createChar(2, fish2);
  lcd.createChar(3, fish3);
  lcd.createChar(4, fish4);
  lcd.createChar(5, fish5);
  lcd.createChar(6, fish6);
  lcd.createChar(7, fish7);
  
  int fishByteTotal = 7;
  lcd.setCursor(0, 1);
  lcd.print("                ");
   
  for(int i = 0; i < col; i++){
    for(int k = 0; k <= fishByteTotal; k++){
      DrawFish(i, k); 
    }
  }
}

//Draw fish on each collom
void DrawFish(int x, int myByte){
  lcd.setCursor(x, 1);
  lcd.write(myByte);
  delay(s);
}

//write an empty char in given col and row
void Empty(int x, int y){
  lcd.setCursor(x, y);
  lcd.write(" ");
  delay(s);
}

// MENU METHODS
void showMain() {
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.write("Main Menu:");
    lcd.setCursor(7,1);
    lcd.write("Press > ");
}

void showInfo() {
    lcd.clear();
    lcd.setCursor(5,0);
    lcd.write("Info:");
    lcd.setCursor(0,1);
    lcd.write("  Aqua fasil V1 ");    
}

void showTemp(){
    lcd.clear();    
    
    sensors.requestTemperatures();
    
    lcd.setCursor(0,0);
    lcd.write("Temperature:");
    lcd.setCursor(0,1);
    lcd.write("C:");        
    lcd.print((int)sensors.getTempCByIndex(0));
    lcd.write(223); 
    lcd.write("    F:");        
    lcd.print((int)DallasTemperature::toFahrenheit(sensors.getTempCByIndex(0))); 
    
}

void showFeed() {      
    lcd.clear();
    lcd.setCursor(4,0);
    lcd.write("Feeded: ");
    lcd.setCursor(5,1);
    lcd.print(feededCounts);
    if(feededCounts <= 1){
      lcd.write(" Time");
    }else{
      lcd.write(" Times");
    }
}

void showTimer() {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.write("Timer 1: ");
    lcd.write("9:00");
    lcd.setCursor(0,1);    
    lcd.write("Timer 2: ");
    lcd.write("21:00");
}

void showHelp() {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.write("To navigate use:");
    lcd.setCursor(0,1);
    lcd.write(" ");
    lcd.write(8);
    lcd.write(" ");
    lcd.write(9);
    lcd.write(" < >  & OK");
}

void showCredits() {
    lcd.clear();
    lcd.setCursor(4,0);
    lcd.write("Created:");
    lcd.setCursor(0,1);
    lcd.write("  By A.T.Coster ");    
}

void feedFish(){
  //rotate motor to feed fish   
 
   // get the sensor value   
  int val = analogRead(0);

    // move a number of steps equal to the change in the
    // sensor reading
  for(int i = 0; i < (rounds * 2); i++){
    myStepper.step(val);
  }
    
  feededCounts+=1;
}

/*************************
**********Settings********
*************************/
void showSettings() {
    lcd.clear();
    lcd.setCursor(4,0);
    lcd.write("Settings:");
    lcd.setCursor(4,1);
    lcd.write("Press >");
  
    isChangeRotation = false;
}

void showRotateRounds(){
    lcd.createChar(8, arrowUp);
    lcd.createChar(9, arrowDown);
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.write("Feeder Rotation:");
    lcd.setCursor(6,1);
    lcd.write(9);
    lcd.write(" ");
    lcd.print(rounds);
    lcd.write(" ");
    lcd.write(8);
}

void changeRotation(){
    int currentBtnValue = results.value;
    
  
    /*
    // if using arduino keyes ir
    switch (currentBtnValue){
      case 5316027:
          rounds += 1;
        break;
      case 2747854299:
        if(rounds <=  0){
           rounds = 0;
        }else{
         rounds -= 1;
        }
      break;
    }
    */
    
     switch (currentBtnValue){
      case 3261853764:
          rounds += 1;
        break;
      case 3305092678:
        if(rounds <=  0){
           rounds = 0;
        }else{
         rounds -= 1;
        }
      break;
    } 
    showRotateRounds();
    delay(500);
}


/*************************
*****date and time fix***
*************************/
void showDateTime() {
    lcd.clear();
    lcd.setCursor(0,0);
    delay(10);
    //digital date
    printDay();
    printMonth();
    lcd.print(day());
    lcd.print(" ");
    lcd.print(year());
    
    lcd.setCursor(0,1);
    delay(10);
    lcd.print("Time ");
    // digital clock display of the time 
    lcd.print(hour());
    lcd.print(":");
    printDigits(minute());
    lcd.print(":");
    lcd.print(second());    
}

void printDigits(int digits){
  // utility for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
    lcd.print("0");
    lcd.print(digits);
}

void printDay(){
  int day;
  day = weekday();
  if(day == 1){lcd.print("Sun, ");}
  if(day == 2){lcd.print("Mon, ");}
  if(day == 3){lcd.print("Tue, ");}
  if(day == 4){lcd.print("Wed, ");}
  if(day == 5){lcd.print("Thu, ");}
  if(day == 6){lcd.print("Fri, ");}
  if(day == 7){lcd.print("Sat, ");}
}

void printMonth(){
  int mon;
  mon = month();
  if(mon == 1){lcd.print("Jan ");}
  if(mon == 2){lcd.print("Feb ");}
  if(mon == 3){lcd.print("Mar ");}
  if(mon == 4){lcd.print("Apr ");}
  if(mon == 5){lcd.print("May ");}
  if(mon == 6){lcd.print("Jun ");}
  if(mon == 7){lcd.print("Jul ");}
  if(mon == 8){lcd.print("Aug ");}
  if(mon == 9){lcd.print("Sep ");}
  if(mon == 10){lcd.print("Oct ");}
  if(mon == 11){lcd.print("Nov ");}
  if(mon == 12){lcd.print("Dec ");}
}
 
 
/*************************
*****end date and time fix***
*************************/
 
/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

