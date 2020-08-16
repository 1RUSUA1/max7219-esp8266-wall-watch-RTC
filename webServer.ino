void handleNotFound()
{
  char tmp[37];
	String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  if (debug_en == 1)
  {
    snprintf_P(tmp,sizeof(tmp),PSTR("debug,http Some page was not found."));
    udpsend(tmp);
  }
}

void handleHelp()
{
  char helppage[1700];
  char tmp[34];
  snprintf_P(helppage, sizeof(helppage), PSTR("<pre>General help:\n\
  /                       - root page. Shows current temperature\n\
  /adc                    - get current ADC value\n\
  /description.xml        - config file from SSDP protocol\n\
  /brightness             - shows current brightness values, stored in EEPROM and system variables\n\
  /brightness/auto        - use ?enable or ?disable parameter to set automatic brightness control.Stored in EEPROM and system variables\n\
  /brightness/day?set=    - set new value between 0 and 15. Store to EEPROM and system variables\n\
  /brightness/night?set=  - set new value between 0 and 15. Store to EEPROM and system variables\n\
  /correct                - correct RTC module time to actual time, got by NTP\n\
  /debug                  - shows the current status of debug mode\n\
  /debug/on               - permanently turns on debug to syslog\n\
  /debug/off              - permanently turns off debug to syslog\n\
  /help                   - shows this help page\n\
  /index.html             - the same as the root page above\n\
  /led/on                 - turn built-in LED on\n\
  /led/off                - turn built-in LED off\n\
  /memory                 - get stored Wifi data in EEPROM.\n\
  /ntp                    - view the current timestamp, received from NTP server\n\
  /reboot                 - restart the system\n\
  /rtc                    - get current time and date value from RTC module\n\
  /show                   - Showing up a text, given as GET parametr after /show?\n\
  /upd                    - force update of current time on the clock\n\
  /uptime                 - get current uptime value\n\
  /wifi                   - setup Wifi data page.\n</pre>\n\
  "));
  server.send(200, "text/html", helppage);
  if (debug_en == 1)
  {
    snprintf_P(tmp,sizeof(tmp),PSTR("debug,http Access to /help page."));
    udpsend(tmp);
  }
}

void handleUptime()//shows up current uptime when being called via HTTP request
{
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  char tmp[36];
  snprintf(uptime, 30, "Uptime: %02d:%02d:%02d\n", hr, min % 60, sec % 60);
  server.send(200, "text/html", uptime);
  if (debug_en == 1)
  {
    snprintf_P(tmp,sizeof(tmp),PSTR("debug,http Access to /uptime page."));
    udpsend(tmp);
  }
}

void handleDebug()//handle request via HTTP and shows up the current status of debug.
{
  char tmp[35];
  if (debug_en == 0)
  {
    snprintf_P(tmp,sizeof(tmp),PSTR("Debug is DISABLED\n"));
    server.send(200, "text/plain", tmp);
  }
  if (debug_en == 1)
  {
    snprintf_P(tmp,sizeof(tmp),PSTR("Debug is ENABLED\n"));
    server.send(200, "text/plain", tmp); 
  }
  if (debug_en == 1)
  {
    snprintf_P(tmp,sizeof(tmp),PSTR("debug,http Access to /debug page."));
    udpsend(tmp);
  }
}

void handleDebugOn()
{
  char tmp[62];
  if (debug_en == 0)
  {
    EEPROM.begin(512);
    EEPROM.write(0,1);
    EEPROM.commit();
    EEPROM.end();
    debug_en = 1;
    snprintf_P(tmp,sizeof(tmp),PSTR("Debug activated!\n"));
    server.send(200, "text/plain", tmp);
    if (debug_en == 1)
    {
      snprintf_P(tmp,sizeof(tmp),PSTR("debug,info Debug activated!"));
      udpsend(tmp);
    }
  }
  else
  {
    snprintf_P(tmp,sizeof(tmp),PSTR("Debug already ENABLED!\n"));
    server.send(200, "text/plain", tmp);
    snprintf_P(tmp,sizeof(tmp),PSTR("debug,info Wrong trying to turn on Debug! Already turned on!"));
    udpsend(tmp);
  }
}

void handleDebugOff()
{
  char tmp[31];
  if (debug_en == 1)
  {
    EEPROM.begin(512);
    EEPROM.write(0,0);
    EEPROM.commit();
    EEPROM.end();
    snprintf_P(tmp,sizeof(tmp),PSTR("Debug is deactivated!\n"));
    server.send(200, "text/plain", tmp);
    if (debug_en == 1)
    {
      snprintf_P(tmp,sizeof(tmp),PSTR("debug,info Debug deactivated!"));
      udpsend(tmp);
    }
    debug_en = 0;
  }
  else
  {
    snprintf_P(tmp,sizeof(tmp),PSTR("Debug is already deactivated!\n"));
    server.send(200, "text/plain",tmp);
  }
}

void handleRTC()
{
   RtcDateTime now = Rtc.GetDateTime();
   printDateTime(now);
}

void setupWebserver()
{
  char tmp[34];
  server.onNotFound(handleNotFound);
  server.on("/", handleHelp);
  server.on("/adc", []() 
  {
      char tmp[35];
      server.send(200, "text/plain", String(analogRead(A0)) + "\n");
      if (debug_en == 1)
      {
         snprintf_P(tmp,sizeof(tmp),PSTR("system,debug Access to /adc page."));
         udpsend(tmp);
      }
  });
  server.on("/brightness", []() 
  {
      char tmp[37];
      String data = "Brightness_day = " + String(brightness_day) + "\n";
      data += "Brightness_night = " + String(brightness_night) + "\n";
      if (autoBri == 1)
      {
        data += "Automatic brightness control is active!\n";
      }
      data += "Brightness_current = " + String(current_brightness) + "\n";
      server.send(200, "text/plain", data);
      if (debug_en == 1)
      {
         snprintf_P(tmp,sizeof(tmp),PSTR("system,debug Access to /brightness."));
         udpsend(tmp);
      }
  });
  server.on("/brightness/auto", []() 
  {
      char tmp[70];
      if (server.argName(0) == "enable")
      {
        EEPROM.begin(512);
        EEPROM.write(3,1);
        EEPROM.commit();
        autoBri = EEPROM.read(3);
        if (autoBri == 1)
        {
          snprintf_P(tmp,sizeof(tmp),PSTR("Automatic brightness control successfully activated!\n"));
          server.send(200, "text/plain", tmp);
          if (debug_en == 1)
          {
            snprintf_P(tmp,sizeof(tmp),PSTR("system,debug Automatic brightness control activated."));
            udpsend(tmp);
          }
        }
        else
        {
            snprintf_P(tmp,sizeof(tmp),PSTR("Error writing new value to EEPROM!\n"));
            server.send(200, "text/plain", tmp);
            if (debug_en == 1)
            {
              snprintf_P(tmp,sizeof(tmp),PSTR("system,debug Error writing new value of autoBrightness to EEPROM!"));
              udpsend(tmp);
            }
          }
        
      }
      else if (server.argName(0) == "disable")
      {
        EEPROM.begin(512);
        EEPROM.write(3,0);
        EEPROM.commit();
        autoBri = EEPROM.read(3);
        if (autoBri == 0)
        {
          snprintf_P(tmp,sizeof(tmp),PSTR("Automatic brightness control successfully deactivated!\n"));
          server.send(200, "text/plain", tmp);
          checkDayNight();
          if (debug_en == 1)
          {
              snprintf_P(tmp,sizeof(tmp),PSTR("system,debug Automatic brightness control deactivated."));
              udpsend(tmp);
          }
        }
        else
        {
            snprintf_P(tmp,sizeof(tmp),PSTR("Error writing new value to EEPROM!\n"));
            server.send(200, "text/plain", tmp);
            if (debug_en == 1)
            {
              snprintf_P(tmp,sizeof(tmp),PSTR("system,debug Error writing new value of autoBrightness to EEPROM!"));
              udpsend(tmp);
            }
          }
        
      }
      else
      {
           snprintf_P(tmp,sizeof(tmp),PSTR("Error! Unknown command. See /help.\n"));
           server.send(200, "text/plain", tmp);
           if (debug_en == 1)
           {
              snprintf_P(tmp,sizeof(tmp),PSTR("system,debug Error! Unknown command received for /brightness/auto"));
              udpsend(tmp);
          }
      }
  });
  server.on("/brightness/day", []() 
  {
      char tmp[65];
      if (server.argName(0) == "set")
      {
        if ((server.arg(0).toInt() <= 15) and (server.arg(0).toInt() >= 0))
        {
          EEPROM.begin(512);
          EEPROM.write(1,server.arg(0).toInt());
          EEPROM.commit();
          brightness_day = EEPROM.read(1);
          if (brightness_day == server.arg(0).toInt())
          {
            snprintf_P(tmp,sizeof(tmp),PSTR("New value of Brightness_day successfully set to %i\n"),brightness_day);
            server.send(200, "text/plain", tmp);
            checkDayNight();
            if (debug_en == 1)
            {
              snprintf_P(tmp,sizeof(tmp),PSTR("system,debug New value of Brightness_day successfully set to %i"),brightness_day);
              udpsend(tmp);
            }
          }
          else
          {
            snprintf_P(tmp,sizeof(tmp),PSTR("Error! New value of Brightness_day not set to %i\n"),brightness_day);
            server.send(200, "text/plain", tmp);
            if (debug_en == 1)
            {
              snprintf_P(tmp,sizeof(tmp),PSTR("system,debug Error! New value of Brightness_day not set to %i"),brightness_day);
              udpsend(tmp);
            }
          }
        }
      }
      else
      {
           snprintf_P(tmp,sizeof(tmp),PSTR("Error! Unknown command. See /help.\n"));
           server.send(200, "text/plain", tmp);
           if (debug_en == 1)
           {
             snprintf_P(tmp,sizeof(tmp),PSTR("system,debug Error! Unknown command received for /brightness/day"));
             udpsend(tmp);
          }
      }
  });
  server.on("/brightness/night", []() 
  {
      char tmp[70];
      char tmp2[3];
      if (server.argName(0) == "set")
      {
        if ((server.arg(0).toInt() <= 15) && (server.arg(0).toInt() >= 0))
        {
          EEPROM.begin(512);
          EEPROM.write(2,server.arg(0).toInt());
          EEPROM.commit();
          brightness_night = EEPROM.read(2);
          if (brightness_night == server.arg(0).toInt())
          {
            snprintf_P(tmp,sizeof(tmp),PSTR("New value of Brightness_night successfully set to %i\n"),brightness_night);
            server.send(200, "text/plain", tmp);
            if (debug_en == 1)
            {             
              snprintf_P(tmp,sizeof(tmp),PSTR("system,debug New value of Brightness_night successfully set to %i"),brightness_night);
              udpsend(tmp);
            }
          }
          else
          {
            snprintf_P(tmp,sizeof(tmp),PSTR("Error! New value of Brightness_night not set to %i\n"),tmp2);
            server.arg(0).toCharArray(tmp2,3);
            server.send(200, "text/plain", tmp);
            if (debug_en == 1)
            {
              snprintf_P(tmp,sizeof(tmp),PSTR("system,debug Error! New value of Brightness_night not set to %c"),tmp2);
              udpsend(tmp);
            }
          }
        }
      }
      else
      {
           snprintf_P(tmp,sizeof(tmp),PSTR("Error! Unknown command. See /help.\n"));
           server.send(200, "text/plain", tmp);
           if (debug_en == 1)
           {
             snprintf_P(tmp,sizeof(tmp),PSTR("system,debug Error! Unknown command received for /brightness/night"));
             udpsend(tmp);
          }
      }
  });
  server.on("/correct", []() 
  {
      char tmp[31];
      correctRTC();
      snprintf_P(tmp,sizeof(tmp),PSTR("RTC corrected successfully!\n"));
      server.send(200, "text/html", tmp);
  });
  server.on("/debug", handleDebug);
  server.on("/debug/on", handleDebugOn);
  server.on("/debug/off", handleDebugOff);
  server.on("/description.xml", []() 
  {
      char tmp[40];
      SSDP.schema(server.client());
      if (debug_en == 1)
      {
         snprintf_P(tmp,sizeof(tmp),PSTR("system,debug Access to description.xml"));
         udpsend(tmp);
      }
  });
  server.on("/help", handleHelp);
  server.on("/index.html", handleHelp);
  server.on("/led/on", []() {
    char tmp[18];
    digitalWrite(LED_BUILTIN, LOW);
    snprintf_P(tmp,sizeof(tmp),PSTR("Led turned ON\n"));
    server.send(200, "text/plain", tmp);
  });
  server.on("/led/off", []() {
    char tmp[18];
    digitalWrite(LED_BUILTIN, LOW);
    snprintf_P(tmp,sizeof(tmp),PSTR("Led turned OFF\n"));
    server.send(200, "text/plain", tmp);
  });
  server.on("/memory", handleEEPROM_Mem);
  server.on("/ntp", []() 
  {
      char tmp[34];
      ntpClient.update();
      String data = getDate() + " " + ntpClient.getFormattedTime() + "\n";
      server.send(200, "text/plain", data);
      if (debug_en == 1)
      {
         snprintf_P(tmp,sizeof(tmp),PSTR("system,debug Access to /ntp page"));
         udpsend(tmp);
      }
  });
  server.on("/reboot", []() 
  {
      char tmp[40];
      snprintf_P(tmp,sizeof(tmp),PSTR("System is going to reboot!\n"));
      server.send(200, "text/plain", tmp);
      snprintf_P(tmp,sizeof(tmp),PSTR("system,info System is going to reboot!"));
      udpsend(tmp);
      int y = (matrix.height() - 8) / 2;
      matrix.drawChar(2, y, '^', HIGH, LOW, 1);
      matrix.drawChar(8, y, '_', HIGH, LOW, 1);
      matrix.drawChar(19, y, '_', HIGH, LOW, 1);
      matrix.drawChar(25, y, '^', HIGH, LOW, 1);
      matrix.write();
      delay(50);
      ESP.restart();
  });
  server.on("/rtc", handleRTC);
  server.on("/save", handleSave);
  server.on("/show", []() {
    server.send(200, "text/plain", "Showing up your text: "+ String(server.argName(0)) + "!\n");
    scrollText(String(server.argName(0)));
  });
  server.on("/upd", []() 
  {
      char tmp[24];
      TimerCounter = 600;
      ntpTimer();
      snprintf_P(tmp,sizeof(tmp),PSTR("Force updating time!\n"));
      server.send(200, "text/plain", tmp);
      
  });
  server.on("/uptime", handleUptime);
  server.on("/wifi", handleWifi);
  server.begin();
  snprintf_P(tmp,sizeof(tmp),PSTR("system,info HTTP server started."));
  udpsend(tmp);
}

void handleEEPROM_Mem()
{
  int len_ssid = 0, len_pass = 0;
  int i = 0;
  String data;
  data += "Current stored data in EEPROM:\n";
  EEPROM.begin(512);
  len_ssid = EEPROM.read(10); // length of the wifi ap name in eeprom cell 10
  len_pass = EEPROM.read(11); // length of the wifi ap name in eeprom cell 11
  data += "Len_ssid: " + String(len_ssid) + "\n";
  data += "Len_pass: " + String(len_pass) + "\n";
  char* buf_ssid = new char[len_ssid+1];
  char* buf_pass = new char[len_pass+1];
  for(i = 0; i < len_ssid; i++) buf_ssid[i] = char(EEPROM.read(i+20));
  buf_ssid[len_ssid] = '\x0';
  data += "SSID: " + String(buf_ssid) + "\n";
  for(i = 0; i < len_pass; i++) buf_pass[i] = char(EEPROM.read(i+40));
  buf_pass[len_pass] = '\x0';
  data += "Pass: " + String(buf_pass) + "\n";
  server.send(200, "text/html", data);
}

//void handleWifi()
//{
//  String data;
//  data += "<html>\
//  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\
//  <head>\
//  <title>Wall clock WiFi Settings</title>\
//  <style>\
//     body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
//   </style>\
//  </head>\
//  <body>\
//     <form method=\"POST\" action=\"save\">\
//     <input name=\"ssid\" value=\"" + String(ssid) + "\" size=\"18\"> WIFI SSID</br></br>\
//     <input name=\"pswd\" value=\"" + String(password) + "\"size=\"18\"> Password</br></br>\
//     <input type=SUBMIT value=\"Save\">\
//   </form>\
//  </body>\
//  </html>";
//  server.send ( 200, "text/html", data);
//}

void handleWifi()
{
  char data[400] = "";
  char data2[90] = "";
  char data3[140] = "";
  char data4[630] = "";
  snprintf_P(data,sizeof(data),PSTR("<html>\n\
  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
  <head>\n\
  <title>Wall clock WiFi Settings</title>\n\
  <style>\n\t\
     body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\n\
  </style>\n\
  </head>\n\
  <body>\n\t\
     <form method=\"POST\" action=\"save\">\n\t\
     <input name=\"ssid\" value=\""));
  strcat(data, ssid);
  
  snprintf(data2,sizeof(data2),PSTR("\" size=\"18\"> WIFI SSID</br></br>\n\t\
     <input name=\"pswd\" value=\""));
  strcat(data2, password);
  
  snprintf(data3,sizeof(data3),PSTR("\"size=\"18\"> Password</br></br>\n\t\
     <input type=SUBMIT value=\"Save\">\n\t\
     </form>\n\
  </body>\n\
  </html>\n"));
  strcat(data4,data);
  strcat(data4,data2);
  strcat(data4,data3);
  server.send(200,"text/html",data4);
}

void handleSave()
{
  String ssid_ap;
  String pass_ap;
  String data;
  int i;
  ssid_ap = server.arg(0);
  pass_ap = server.arg(1);
  unsigned char* buf_ssid = new unsigned char[MAX_LENGTH_SSID_PASS+1];
  unsigned char* buf_pass = new unsigned char[MAX_LENGTH_SSID_PASS+1];
  if (ssid_ap != "")
  {
    EEPROM.begin(512);
    EEPROM.write(10,ssid_ap.length());
    EEPROM.write(11,pass_ap.length());
    ssid_ap.getBytes(buf_ssid, ssid_ap.length()+1);   
    for(byte i = 0; i < ssid_ap.length(); i++)
    {
      EEPROM.write(i+20, buf_ssid[i]);
    }
    pass_ap.getBytes(buf_pass, pass_ap.length()+1);
    for(byte i = 0; i < pass_ap.length(); i++)
    {
      EEPROM.write(i+40, buf_pass[i]);
    }
    ssid  = (char*) buf_ssid;
    password  = (char*) buf_pass;
    EEPROM.commit();
    EEPROM.end();
    server.send(200,"text/html","Saved values:\nSSID: " + ssid_ap + "\nPass: " + pass_ap + "\n");
  }
}
