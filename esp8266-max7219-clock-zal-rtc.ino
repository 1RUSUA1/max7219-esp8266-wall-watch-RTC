#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <ESP8266SSDP.h>
#include <ESP8266mDNS.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include <Ticker.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>
#include <pgmspace.h>

Max72xxPanel matrix = Max72xxPanel(12,4,1);//(PinCS,numberOfHorizontalDisplays,numberOfVerticalDisplays).Attach CS to this pin, DIN to MOSI(GPIO13) and CLK to SCK(GPIO14)
ThreeWire myWire(4,5,2); // DAT, CLK, RST
RtcDS1302<ThreeWire> Rtc(myWire);
WiFiClient client;
WiFiUDP udp;
Ticker Timer, Timer2;//timer for regular update via NTP and some other function and timer2 for every 30 sec scrolltext for errors and wifi check
NTPClient ntpClient(udp, "192.168.10.1", ((3*60*60)),6000000); // IST = GMT + 3, update interval 10 min.
ESP8266WebServer server(80);
char *ssid = "";
char *password = "";
uint8_t debug_en = 2;//global variable for debug mode
uint8_t New = 1;//global variable for the first launch of the system
uint8_t is_errors = 0;//global variable for the error existing sighn on LED. If there are any serious problems - the LED will display "|" not ":"
uint8_t wifi_conn = 1;//global variable for Wifi connection status.
uint8_t autoBri = 0;//global variable for auto brightntess function.Stored in EEPROM. Turning on or off by HTTP request.
const char *tmpdata_header = "wall-clock-zal.lan ";//this whould be added to the start of any log string, sent to Rsyslog server.
const char *OtaHostName = "Wall-Clock-Zal";
const char *SsdpName = "Wi-Fi Wall Clock in Zal";
char uptime[30];//for show uptime function
int TimerCounter = 0;//those counters will be incremented every 1 sec.
uint8_t TimerCounter2 = 0;//counter2 for timer2 for Wifi check and scrolltext with errors any 30 sec.
uint16_t TimerCounter3 = 0;//counter3 for timer2 to make getDayNight() checks
uint8_t h = 0,m = 0,s = 0;//variables for hours, minutes and seconds.
long localEpoc = 0;//local copy of millis() to store current time
long localMillisAtUpdate = 0;
uint8_t brightness_day = 0, brightness_night = 0, current_brightness = 0;//variables for brightness control
uint16_t sensorValue = 0;//for ADC in automatic brightness regulation
#define MAX_LENGTH_SSID_PASS 16
int lock_ap = 0;

void udpsend(const char* tmpdata);
void ntpTimer();
void correctRTC();
String getDate();
void updateTime();
void scrollText(String tape);
void checkDayNight();
void printDateTime(const RtcDateTime& dt);
void regularChecks();
void checkWifi();
void autoBrightness();
void timerFunc();
void setupEEPROM();
void setupOTA();
void setupSSDP();
void handleNotFound();
void handleHelp();
void handleUptime();
void handleDebug();
void handleDebugOn();
void handleDebugOff();
void handleRTC();
void setupWebserver();
void Init_EEPROM_Wifi();
void Read_EEPROM_Wifi();
void startAP();
void setupWifi();
void handleWifi();
void handleSave();
void handleEEPROM_Mem();

//sends a string ot text to Rsyslog server, set in beginPacket() function.
void udpsend(const char* tmpdata)
{
    char data[250];
    strcpy(data, tmpdata_header);
    strcat(data, tmpdata);
    udp.beginPacket("192.168.10.1",514);
    udp.write(data);
    udp.endPacket();
    delay(200);
}

//updates time every 24 hours by ntp
void ntpTimer() {
  char tmp[100];
  if (TimerCounter >= 86400) 
  {
     if(ntpClient.forceUpdate())
     {
        String source_time = ntpClient.getFormattedTime();
        h = source_time.substring(0,2).toInt();
        m = source_time.substring(3,6).toInt();
        s = source_time.substring(6,8).toInt();
        localMillisAtUpdate = millis();
        localEpoc = (h * 60 * 60 + m * 60 + s); 
        TimerCounter = 0;
        correctRTC();
        if (debug_en == 1)
        {
          long curEpoch = localEpoc + ((millis() - localMillisAtUpdate) / 1000);
          long epoch = round(curEpoch + 86400L);
          snprintf_P(tmp,sizeof(tmp),PSTR("system,debug ntpTimer: Time updated successfullyby NTP. H:%02u:M%02u:S%02u LocalEpoch:%l\n"),h,m,s,localEpoc);
          udpsend(tmp);
        }
     }
     else
     {
         TimerCounter = 0;
     }
  }
  TimerCounter++;
}

void correctRTC()
{
  String source_time = ntpClient.getFormattedTime();
  h = source_time.substring(0,2).toInt();
  m = source_time.substring(3,6).toInt();
  s = source_time.substring(6,8).toInt();
  String formattedDate = ntpClient.getFormattedDate();
  int splitT = formattedDate.indexOf("T");
  String dayStamp = formattedDate.substring(0, splitT);
  int Y = dayStamp.substring(0,4).toInt();
  int M = dayStamp.substring(5,7).toInt();
  int D = dayStamp.substring(8,10).toInt();
  RtcDateTime currentTime = RtcDateTime(Y,M,D,h,m,s); //define date and time object
  Rtc.SetDateTime(currentTime);
  RtcDateTime now1 = Rtc.GetDateTime();
  initializeTime(now1);
  if (debug_en == 1)
  {
    udpsend("debug,info RTC corrected successfully!");
  }
}

//shows up current date
String getDate() {
   String formattedDate = ntpClient.getFormattedDate();
   int splitT = formattedDate.indexOf("T");
   String dayStamp = formattedDate.substring(0, splitT);
   return dayStamp;
}

//update current time and set variables of H,M,S to show on the screen
void updateTime()
{
  long curEpoch = localEpoc + ((millis() - localMillisAtUpdate) / 1000);
  long epoch = round(curEpoch + 86400L);
  h = ((epoch  % 86400L) / 3600) % 24;
  m = (epoch % 3600) / 60;
  s = epoch % 60;
}

//just for fun function. Can show up scrolling text, taken by HTTP request.
void scrollText(String tape)
{
    for ( int i = 0 ; i < 6 * tape.length() + matrix.width() - 1 - 1; i++ ) {
    matrix.fillScreen(LOW);
    int letter = i / 6;
    int x = (matrix.width() - 1) - i % 6;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + 6 - 1 >= 0 && letter >= 0 ) {
      if ( letter < tape.length() ) {
        matrix.drawChar(x, y, tape[letter], HIGH, LOW, 1);
      }
      letter--;
      x -= 6;
    }
    matrix.write();
    delay(40);
  }
}

//change brightness of LED screen at night.Levels of brightness are between 0 and 15.The are stored in EEPROM.Could be set via HTTP request.
void checkDayNight()
{
  char tmp[42];
  if (h == 22 || h == 23 || h == 00 || h == 01 || h == 02 || h == 03 || h == 04 || h == 05 || h == 06)
  {
    if (current_brightness != brightness_night)
    {
      matrix.setIntensity(brightness_night);
      current_brightness = brightness_night;
      if (debug_en == 1)
      {
        snprintf_P(tmp,sizeof(tmp),PSTR("system,debug Brightness changed to Night"));
        udpsend(tmp);
      }
    }
  }
  else
  {
    if (current_brightness != brightness_day)
    {
      matrix.setIntensity(brightness_day);
      current_brightness = brightness_day;
      if (debug_en == 1)
      {
        snprintf_P(tmp,sizeof(tmp),PSTR("system,debug Brightness changed to Day"));
        udpsend(tmp);
      }
    }
  }
}

void printDateTime(const RtcDateTime& dt)
{
    char datestring[23];
    #define countof(a) (sizeof(a) / sizeof(a[0]))
    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u\n"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second());
    server.send(200, "text/html", datestring);
}

void initializeTime(const RtcDateTime& dt)
{
  h = dt.Hour();
  m = dt.Minute();
  s = dt.Second();
  char tmp[80];
  localMillisAtUpdate = millis();
  localEpoc = (h * 60 * 60 + m * 60 + s);
  if (debug_en == 1)
  {
    snprintf_P(tmp,countof(tmp),PSTR("system,debug System time initialized by RTC module. Current time: %02u:%02u:%02u\n"),h,m,s);
    udpsend(tmp);
  }
}

//regular check every 30 sec. by Timer2
void regularChecks()
{
  if (TimerCounter2 >= 30) // check wifi connection state every 30 sec.
  {
    checkWifi();
    TimerCounter2 = 0;
  }
  if (TimerCounter3 >= 600) // check DayNight brightness every 10 minutes.
  {
    if (autoBri == 0)//if autoBrightness by light sensor is disabled, using manual correction by time
    {
      checkDayNight();
    }
    TimerCounter3 = 0;
  }
  TimerCounter2++;
  TimerCounter3++;
}

//checks out Wifi status. If its disconnected, trying to restore connection every 30 sec.
void checkWifi()
{
  if (WiFi.status() != WL_CONNECTED) 
  {
    if (wifi_conn == 1)
    {
      wifi_conn = 0;
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
  else
  {
    if (wifi_conn == 0)
    {
      wifi_conn = 1;
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
}

void autoBrightness()
{
  uint16_t sensorValue, value;
  sensorValue = analogRead(A0);
  if (sensorValue <= 10) { value = 0; }
  if (sensorValue > 10 and sensorValue < 100) { value = 5; }
  if (sensorValue > 100 and sensorValue < 250) { value = 10; }
  if (sensorValue >= 250) { value = 15; }
  if (current_brightness != value)
  {
    matrix.setIntensity(value);
    current_brightness = value;
  }  
}

void timerFunc()
{
  ntpTimer();
  regularChecks();
  if (autoBri == 1)
  {
    autoBrightness();
  }
  checkWifi();
}

void setup() {
  char tmp[70];
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0,INPUT_PULLUP);
  digitalWrite(LED_BUILTIN, LOW);
  matrix.setIntensity(15);
  matrix.setRotation(0, 1);
  matrix.setRotation(3, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(1, 1);
  setupWifi();
  udpsend("system,info System just has been started!");
  setupWebserver();
  setupSSDP();
  setupEEPROM();  
  setupOTA();
  if (MDNS.begin(OtaHostName))
  {
    snprintf_P(tmp,sizeof(tmp),PSTR("system,info MDNS responder started. Hostname: %c"),OtaHostName);
    udpsend(tmp);
  }
  udp.begin(123);
  Timer.attach(1,timerFunc);
  snprintf_P(tmp,sizeof(tmp),PSTR("system,info NTP update timer activated!"));
  udpsend(tmp);
  ntpClient.begin();
  Rtc.Begin();
  if (!Rtc.IsDateTimeValid()) 
  {
    snprintf_P(tmp,sizeof(tmp),PSTR("system,error RTC lost confidence in the DateTime!"));
    udpsend(tmp);
    is_errors = 1;
  }
  if (Rtc.GetIsWriteProtected())
  {
    snprintf_P(tmp,sizeof(tmp),PSTR("system,error RTC was write protected, enabling writing now"));
    udpsend(tmp);
    Rtc.SetIsWriteProtected(false);
  }
  if (!Rtc.GetIsRunning())
  {
    snprintf_P(tmp,sizeof(tmp),PSTR("system,error RTC was not actively running, starting now"));
    udpsend(tmp);
    Rtc.SetIsRunning(true);
  }
  if (autoBri == 1)
  {
    snprintf_P(tmp,sizeof(tmp),PSTR("system,info Automatic brightness sensor activated!"));
    udpsend(tmp);
  }
    snprintf_P(tmp,sizeof(tmp),PSTR("system,info System started successfully!"));
    udpsend(tmp);
}

void loop() {
  if (New == 1)//important function of the first launch - shows up a text "----" and syncronizing time.
  {
    if (is_errors == 0)
    {
      RtcDateTime now1 = Rtc.GetDateTime();
      initializeTime(now1);   
    }
    if (is_errors == 1)
    {
      TimerCounter = 86400;
      ntpTimer();
      if (debug_en == 1)
      {
        udpsend("system,error First launch - RTC module error. Time initialized by NTP!");
      }
    }
    if (autoBri == 0)
    {
      checkDayNight();
    }
    New = 0;
    if (debug_en == 1)
    {
      udpsend("system,info First launch - time successfully syncronized.");
    }
    int y = (matrix.height() - 8) / 2;
    matrix.drawChar(2, y, '-', HIGH, LOW, 1);
    matrix.drawChar(8, y, '-', HIGH, LOW, 1);
    matrix.drawChar(19, y, '-', HIGH, LOW, 1);
    matrix.drawChar(25, y, '-', HIGH, LOW, 1);
    matrix.write();
    delay(500);
  }
  server.handleClient();
  MDNS.update();
  ArduinoOTA.handle();
  updateTime();
  matrix.fillScreen(LOW);
  int y = (matrix.height() - 8) / 2; // Centering text by vertical
  if(s & 1)   //printing : every odd second to make doubledots blinking
  {
    if ((is_errors == 1) || (wifi_conn == 0))
    {
      matrix.drawChar(14, y, (String("!"))[0], HIGH, LOW, 1);
    }
    if (lock_ap != 0)
    {
      matrix.drawChar(14, y, (String("+"))[0], HIGH, LOW, 1);
    }
    if ((is_errors != 1) && (wifi_conn == 1) && (lock_ap == 0))
    {
      matrix.drawChar(14, y, (String(":"))[0], HIGH, LOW, 1);
    }
  }
  String hour1 = String (h/10);
  String hour2 = String (h%10);
  String min1 = String (m/10);
  String min2 = String (m%10);
  matrix.drawChar(2, y, hour1[0], HIGH, LOW, 1);
  matrix.drawChar(8, y, hour2[0], HIGH, LOW, 1);
  matrix.drawChar(19, y, min1[0], HIGH, LOW, 1);
  matrix.drawChar(25, y, min2[0], HIGH, LOW, 1);
  matrix.write();
  if (digitalRead(0) == LOW)
  {
    if (lock_ap == 0)
    {
        startAP();
    }
  }
}
