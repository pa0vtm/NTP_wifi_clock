#include <FS.h>          // this needs to be first, or it all crashes and burns...
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <time.h>


int EncA = D1;
int EncB = D2;
int EncS = D3;

int pinCS = D8; 
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays   = 1;
char time_value[20];

int wait = 70; // In milliseconds
int spacer = 1;
int width  = 5 + spacer; // The font width is 5 pixels
int brightness = 7;
String mytime;
char APname[16]= "NTP_CLOCK";
char APPass[16]= "password";
long lastsync;
long currentms;
long intervalms = 300000;
long lastrefresh;
long screenrefresh = 500;
long screentimeout = 1000;
long longtimeout=10000;
int currentscreen,currentmenunumber, prevscreen;

volatile bool EncAState, EncBState, EncSState;
volatile int EncPos;
volatile bool rotChanged = false ;


String localTZ = "GMT-1BST";
bool LTZ = true;

// LED Matrix Pin -> ESP8266 Pin
// Vcc            -> 3v  (3V on NodeMCU 3V3 on WEMOS)
// Gnd            -> Gnd (G on NodeMCU)
// DIN            -> D7  (Same Pin for WEMOS)
// CS             -> D8  (Same Pin for WEMOS)
// CLK            -> D5  (Same Pin for WEMOS)

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
  void ICACHE_RAM_ATTR ISRoutineA ();
  void ICACHE_RAM_ATTR ISRoutineB (); 
  void ICACHE_RAM_ATTR ISRoutineS ();

  
void setup() {
  Serial.begin(115200);
  Serial.println();

  pinMode(EncA, INPUT);
  pinMode(EncB, INPUT);
  pinMode(EncS, INPUT_PULLUP);

  void ICACHE_RAM_ATTR ISRoutineA ();
  void ICACHE_RAM_ATTR ISRoutineB (); 
  void ICACHE_RAM_ATTR ISRoutineS ();

attachInterrupt(digitalPinToInterrupt(EncA), ISRoutineA,FALLING);
attachInterrupt(digitalPinToInterrupt(EncB), ISRoutineB,FALLING);
attachInterrupt(digitalPinToInterrupt(EncS), ISRoutineS,FALLING);
  
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);

  //reset settings - for testing
  //wifiManager.resetSettings();


  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "NTP_Clock" with password "password"
  //and goes into a blocking loop awaiting configuration
  
  
    matrix.setIntensity(brightness); // Use a value between 0 and 15 for brightness
    matrix.setRotation(0, 1);    // The first display is position upside down
    matrix.setRotation(1, 1);    // The first display is position upside down
    matrix.setRotation(2, 1);    // The first display is position upside down
    matrix.setRotation(3, 1);    // The first display is position upside down
    matrix.fillScreen(LOW);
    matrix.write();
    
    screen_manager(1);
//    screen_manager(3);
//    screen_manager(1);
  if (!wifiManager.autoConnect(APname, APPass)) {  //wait until wifi configured
      Serial.println("failed to connect, we should reset as see if it connects");
      delay(3000);
      screen_manager(4);
      ESP.restart();
      delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected... :)");
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  delay(10);
    Serial.print("Current time: ");
    Serial.print(mytime);
    Serial.print("      NTP time: ");
    configTime(0 * 3600, 0, "nl.pool.ntp.org", "time.nist.gov");
    lastsync = currentms;
    time_t now = time(nullptr);
    String time = String(ctime(&now));
    mytime = time;
    Serial.print(mytime);
    Serial.print("   next sync in: ");
    Serial.print(intervalms);
    Serial.println(" ms.");
  setenv("TZ", "GMT-1BST",1);
screen_manager(0);
}

void loop() {         //TODO: optimize bij using sleepmode
  if(rotChanged){
    delay(25);
    Serial.print("rotary: ");
    Serial.println(EncPos);
    Serial.print("currentscreen: ");
    Serial.println(currentscreen);
    Serial.print("currentmenunumber: ");
    Serial.println(currentmenunumber);
    switch(currentscreen){
      case 0: currentmenunumber=0; menu_manager(); break;
      case 11: brightness = brightness + EncPos; brightness = max(brightness,0); brightness = min(brightness,15);screen_manager(11); break;
      case 100: currentmenunumber=currentmenunumber+EncPos; menu_manager();break;
      default: break;
    }
    if(EncSState){
        switch(currentscreen){
            case 0: break;//open  menu
            case 11: screen_manager(prevscreen); break;
           // case 100: button_press(); break;
            default: break;          
        } 
      if(currentscreen==100){
          button_press();
          }
      }
    }
    
     
    EncSState=false;
    rotChanged = false;
  
  
  currentms = millis();
  if (currentms > lastrefresh + screenrefresh){ //refresh display only when screen0 (time) is shown
    if(currentscreen==0){
      screen_manager(0);
    }
  }

    if (currentms > lastrefresh + longtimeout){ //refresh display tiem time after longtimeout

      screen_manager(0);
    
  }



  if(currentms > lastsync + intervalms){      // resync NTP
    Serial.print("Current time: ");
    Serial.print(mytime);
    Serial.print("      NTP time: ");
    configTime(0 * 3600, 0, "nl.pool.ntp.org", "time.nist.gov");
    lastsync = currentms;
    time_t now = time(nullptr);
    String time = String(ctime(&now));
    mytime = time;
    Serial.print(mytime);
    Serial.print("   next sync in: ");
    Serial.print(intervalms);
    Serial.println(" ms.");
  }

}

void chars5(String message){    //print 5 chars
    matrix.setIntensity(brightness); // Use a value between 0 and 15 for brightness
    matrix.drawChar(2,0, message[0], HIGH,LOW,1); // 1
    matrix.drawChar(8,0, message[1], HIGH,LOW,1); // 2  
    matrix.drawChar(14,0,message[2], HIGH,LOW,1); // 3
    matrix.drawChar(20,0,message[3], HIGH,LOW,1); // 4
    matrix.drawChar(26,0,message[4], HIGH,LOW,1); // 5
    matrix.write(); // Send bitmap to display
    
  
}

void display_message(String message){   //print scrolling text
   for ( int i = 0 ; i < width * message.length() + matrix.width() - spacer; i++ ) {
    //matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < message.length() ) {
        matrix.drawChar(x, y, message[letter], HIGH, LOW, 1); // HIGH LOW means foreground ON, background off, reverse to invert the image
      }
      letter--;
      x -= width;
    }
    matrix.write(); // Send bitmap to display
    delay(wait/2);
  }
}

String IpAddress2String(const IPAddress& ipAddress){ //convert up adress to string
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}


//schermen:
//0:  klok
//1:  WI-FI
//2:  FAIL
//3:
//4:
//10:  instellingen
//20:  netwerk
//11: helderheid
//12: sync interval
//22: toon IP adres
//23: toon ntp server
//29: reset wifi

void menu_manager(){
  currentscreen=100;
  switch(currentmenunumber){
    case -1: currentmenunumber = 0; break;  
    case 0: chars5(LTZ?"UTC":"LTZ"); break;
    case 1: chars5("Light"); break;
    case 2: chars5("IP");break;
    case 3: chars5("Exit"); break;
    default : currentmenunumber=0; break;
  }
  EncPos=0;
}


void button_press(){
  if(currentscreen==100){
    prevscreen = currentscreen;
    switch(currentmenunumber){
      case 0: setenv("TZ", LTZ?"UTC":"GMT-1BST",1);LTZ=!LTZ; screen_manager(prevscreen);break;  //exit
      case 1: screen_manager(11);break;   //brightness 
      case 2: screen_manager(22);break;   //network
      case 3: screen_manager(0); break;  //exit
      default: screen_manager(0);break;
    }
  }
}

//screenmanager displays the info on the screen, based on the requested screennumber 

void screen_manager(int screennumber){
  lastrefresh = currentms;
  switch (screennumber) {  
      case 0: screen_0(); break;
      case 1: screen_1(); break;
      case 2: screen_2(); break;
      case 3: screen_3(); break;
      case 4: screen_4(); break;
      case 11: screen_11(); break;
      case 22: screen_22(); break;
      case 23: screen_23(); break;
      case 24: screen_24(); break;
      case 25: screen_25(); break;
      case 100: menu_manager(); break;
      default: screen_0(); break;
  }
 
}

//every screen is a separete function, called from screen_manager

void screen_0 (void){ //print time
   matrix.setIntensity(brightness);
    matrix.fillScreen(LOW);
    time_t now = time(nullptr);
    String time = String(ctime(&now));
    mytime = time;
    time.trim();
    time.substring(11,19).toCharArray(time_value, 10); 
    matrix.drawChar(2,0, time_value[0], HIGH,LOW,1); // H
    matrix.drawChar(8,0, time_value[1], HIGH,LOW,1); // HH  
    matrix.drawChar(14,0,time_value[2], HIGH,LOW,1); // HH:
    matrix.drawChar(20,0,time_value[3], HIGH,LOW,1); // HH:M
    matrix.drawChar(26,0,time_value[4], HIGH,LOW,1); // HH:MM
    matrix.write(); // Send bitmap to display
    lastrefresh = currentms;
    currentscreen = 0;
}

void screen_1(void){  //print WI-FI
    matrix.setIntensity(brightness); // Use a value between 0 and 15 for brightness
    matrix.drawChar(2,0, 'W', HIGH,LOW,1); // H
    matrix.drawChar(8,0, 'I', HIGH,LOW,1); // HH  
    matrix.drawChar(14,0,'-', HIGH,LOW,1); // HH:
    matrix.drawChar(20,0,'F', HIGH,LOW,1); // HH:M
    matrix.drawChar(26,0,'I', HIGH,LOW,1); // HH:MM
    matrix.write(); // Send bitmap to display
    currentscreen =1;
  
}

void screen_2(void){  //print FAIL
    matrix.setIntensity(brightness); // Use a value between 0 and 15 for brightness
    matrix.drawChar(2,0, 'F', HIGH,LOW,1); // H
    matrix.drawChar(8,0, 'A', HIGH,LOW,1); // HH  
    matrix.drawChar(14,0,'I', HIGH,LOW,1); // HH:
    matrix.drawChar(20,0,'L', HIGH,LOW,1); // HH:M
    matrix.drawChar(26,0,'!', HIGH,LOW,1); // HH:MM
    matrix.write(); // Send bitmap to display
    currentscreen =2;
  
}

void screen_3(void){  //print local AP info
   matrix.setIntensity(brightness); // Use a value between 0 and 15 for brightness
    display_message("Connect to:");
    display_message(APname);
    display_message("use password:");
    display_message(APPass);
    currentscreen=3;
}

void screen_4(void){  //print reboot
    matrix.setIntensity(brightness); // Use a value between 0 and 15 for brightness
    display_message("_REBOOT_");
    currentscreen=4;
 }

void screen_11(void){ //set brightness
    matrix.setIntensity(brightness); // Use a value between 0 and 15 for brightness
    chars5(String(brightness));
    currentscreen=11;
 }


 
void screen_22(void){ //print IP
    matrix.setIntensity(brightness); // Use a value between 0 and 15 for brightness
  display_message(IpAddress2String(WiFi.localIP()));
  currentscreen=22;
  screen_manager(prevscreen);
}

void screen_23(void){ //print NTP **todo, dynamic NTP address or NTP address from webbrouwser
    matrix.setIntensity(brightness); // Use a value between 0 and 15 for brightness
  display_message("nl.pool.ntp.org");
  currentscreen=23;
}

void screen_24(void){ //print connected wifi network
   matrix.setIntensity(brightness); // Use a value between 0 and 15 for brightness
    display_message(APname);
    currentscreen=24;
}

void screen_25(void){ //print  APmode wifi password
  matrix.setIntensity(brightness); // Use a value between 0 and 15 for brightness
  display_message(APPass);
  currentscreen=25;
}



// Interrupt handlers for rotary-encoder + pushbutton

void ISRoutineA() { //encoderA
    if(!rotChanged){
    EncBState = digitalRead(EncB);
    EncBState?EncPos++:EncPos--;
    rotChanged = true;      
    }
}

void ISRoutineB() { //encoderA
    if(!rotChanged){
    EncAState = digitalRead(EncA);
    EncAState?EncPos--:EncPos++;
    rotChanged = true;      
    }
}

void ISRoutineS() { //encoderSwitch
    if(!rotChanged){
    EncSState = true;
    rotChanged = true;      
    }
}
