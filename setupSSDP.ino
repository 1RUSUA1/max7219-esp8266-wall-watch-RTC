void setupSSDP()
{
  char tmp[35];
  SSDP.setDeviceType("upnp:rootdevice");
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName(SsdpName);
  SSDP.setSerialNumber("2707202002");
  SSDP.setURL("index.html");
  SSDP.setModelName("Super Wall Clock V2.1");
  SSDP.setModelNumber("2507202002");
  SSDP.setModelURL("https://rusua.org.ua/");
  SSDP.setManufacturer("RUSUA test devices");
  SSDP.setManufacturerURL("https://rusua.org.ua/");
  if (SSDP.begin())
  {
    snprintf_P(tmp,sizeof(tmp),PSTR("system,info SSDP service started."));
    udpsend(tmp);
  }
}
