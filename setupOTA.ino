void setupOTA()
{
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(OtaHostName);
  ArduinoOTA.onStart([]() {
  char tmp[42];
  snprintf_P(tmp,sizeof(tmp),PSTR("system,info Started OTA firmware update!"));
  udpsend(tmp);
  //this shows up symbols < < < < on the screen when being OTA updating 
  int y = (matrix.height() - 8) / 2;
  matrix.drawChar(2, y, '<', HIGH, LOW, 1);
  matrix.drawChar(8, y, '<', HIGH, LOW, 1);
  matrix.drawChar(19, y, '<', HIGH, LOW, 1);
  matrix.drawChar(25, y, '<', HIGH, LOW, 1);
  matrix.write();
  });
  ArduinoOTA.begin();
}
