void setupEEPROM()
{
  char tmp[80];
  EEPROM.begin(512);
  debug_en = EEPROM.read(0);
  if (debug_en > 1)
  {
    EEPROM.write(0,0);
    EEPROM.commit();
    debug_en = 0;
    snprintf_P(tmp,sizeof(tmp),PSTR("system,info EEPROM value Debug initialized first time. Set to 0."));
    udpsend(tmp);
  }
  brightness_day = EEPROM.read(1);
  if (brightness_day > 15)
  {
    EEPROM.write(1,15);
    EEPROM.commit();
    brightness_day = 15;
    snprintf_P(tmp,sizeof(tmp),PSTR("system,info EEPROM value Brightness_day initialized first time. Set to 15."));
    udpsend(tmp);
  }
  brightness_night = EEPROM.read(2);
  if (brightness_night > 15)
  {
    EEPROM.write(2,3);
    EEPROM.commit();
    brightness_night = 1;
    snprintf_P(tmp,sizeof(tmp),PSTR("system,info EEPROM value Brightness_night initialized first time. Set to 1."));
    udpsend(tmp);
  }
  autoBri = EEPROM.read(3);
  if (autoBri > 1)
  {
    EEPROM.write(3,0);
    EEPROM.commit();
    autoBri = 0;
    snprintf_P(tmp,sizeof(tmp),PSTR("system,info EEPROM value AutoBrightness initialized first time. Set to 0."));
    udpsend(tmp);
  }
  if (debug_en == 1)
  {
    snprintf_P(tmp,sizeof(tmp),PSTR("system,info Debug is enabled in the system."));
    udpsend(tmp);
  }
}
