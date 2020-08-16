void setupWifi() 
{
  WiFi.mode(WIFI_STA);
  WiFi.hostname(OtaHostName);
  if (WiFi.getAutoConnect() != true)    //configuration will be saved into SDK flash area
  {
    WiFi.setAutoConnect(true);   //on power-on automatically connects to last used hwAP
    WiFi.setAutoReconnect(true);    //automatically reconnects to hwAP in case it's disconnected
  }
  Read_EEPROM_Wifi();
  WiFi.begin(ssid, password);
  int ind=0, mod=0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    if (mod == 0)
    {
      int y = (matrix.height() - 8) / 2;
      matrix.drawChar(2, y, '-', HIGH, LOW, 1);
      matrix.drawChar(8, y, '-', HIGH, LOW, 1);
      matrix.drawChar(19, y, '-', HIGH, LOW, 1);
      matrix.drawChar(25, y, '-', HIGH, LOW, 1);
      matrix.write();
      mod = 1;
    }
    else if (mod == 1)
    {
      int y = (matrix.height() - 8) / 2;
      matrix.drawChar(2, y, '|', HIGH, LOW, 1);
      matrix.drawChar(8, y, '|', HIGH, LOW, 1);
      matrix.drawChar(19, y, '|', HIGH, LOW, 1);
      matrix.drawChar(25, y, '|', HIGH, LOW, 1);
      matrix.write();
      mod = 0;
    }
    ind++;
    if (ind>20) break;
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    scrollText("No WiFi!");
    wifi_conn = 0;
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    scrollText("Ready!");
    scrollText(WiFi.localIP().toString());
    wifi_conn = 1;
  }
}

void Init_EEPROM_Wifi()
{
  String ssid_ap;
  String pass_ap;
  int i;
  unsigned char* buf_ssid = new unsigned char[MAX_LENGTH_SSID_PASS+1];
  unsigned char* buf_pass = new unsigned char[MAX_LENGTH_SSID_PASS+1];
  ssid_ap = "apname";
  pass_ap = "appass";
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
}

void Read_EEPROM_Wifi()
{
  byte len_ssid=0, len_pass=0;
  int i = 0;
  EEPROM.begin(512);
  len_ssid = EEPROM.read(10); // length of the wifi ap name in eeprom cell 10
  len_pass = EEPROM.read(11); // length of the wifi ap name in eeprom cell 11
  if (len_ssid > MAX_LENGTH_SSID_PASS || len_pass > MAX_LENGTH_SSID_PASS)
  {
    Init_EEPROM_Wifi();
  }
  char* buf_ssid = new char[len_ssid+1];
  char* buf_pass = new char[len_pass+1];
  for(i = 0; i < len_ssid; i++) buf_ssid[i] = char(EEPROM.read(i+20));
  buf_ssid[len_ssid] = '\x0';
  ssid  = (char*) buf_ssid;
  for(i = 0; i < len_pass; i++) buf_pass[i] = char(EEPROM.read(i+40));
  buf_pass[len_pass] = '\x0';
  password  = (char*) buf_pass;
}

void startAP()
{
  if (lock_ap == 0)
  {
    char *ssid_ap=new char [20];
    String S="Wall_Clock";
    S.toCharArray(ssid_ap, S.length()+1);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid_ap);
    delay(2000);
    scrollText("AP ready! " + WiFi.softAPIP().toString());
    lock_ap = 1;
  } 
}
