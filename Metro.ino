#include <ESP8266WiFi.h>

const char ssid[20] = "METRO_DIGITAL";
const char pass[20] = "12345678";

String ID;
String Kwh;
String Invert;
WiFiServer server(80);

void setup() {
  delay(1000);
  uint32_t ip = 0x0104A8C0; // 192.168.4.1
  IPAddress subnet = {255, 255, 255, 0};

  Serial.begin(1200, SERIAL_7E1);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pass);
  WiFi.softAPConfig(ip, ip, subnet);
  server.begin();  
}

void ask() {
  char ACK[] = {0x06, 0x30, 0x32, 0x30, 0x0d, 0x0a};
  boolean found = false;
  
  ID = "";
  Kwh = "";
  Invert = "";
  Serial.flush();
  Serial.println("/?!");
  String line = Serial.readStringUntil('\n');
  while (line && line.length() > 0) {
    if (line.equals("/ZTY2ZT\r")) 
      found = true;
    line = Serial.readStringUntil('\n');
  } 
  if (found) {
    Serial.flush();
    Serial.write(ACK, 6);
    line = Serial.readStringUntil('\n');
    while (line && line.length() > 0) {
      int start = line.indexOf("96.1.0(");
      if (start != -1) {
        start += 7;
        int end = line.indexOf(")", start);
        if (end != -1 && end - start == 12) {
          ID = line.substring(start, end);
        }    
      } else {
        int start = line.indexOf("1.8.0(");
        if (start != -1) {
          start += 6;
          int end = line.indexOf("*kWh)", start);
          if (end != -1 && end - start == 9) {
            Kwh = line.substring(start, end);
          }    
        } else {
          int start = line.indexOf("2.8.0(");
          if (start != -1) {
            start += 6;
            int end = line.indexOf("*kWh)", start);
            if (end != -1 && end - start == 9) {
              Invert = line.substring(start, end);
            }    
          }
        } 
      }    
      line = Serial.readStringUntil('\n');
    } 
  }
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String line = client.readStringUntil('\n');
    while (line && line.length() > 0) {
      line = client.readStringUntil('\n');
    }  
    client.flush();
    ask();
    String s = F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\" /><title>Metro</title></head><body>");
    s += F("<table align=\"center\" border=\"0\"><tr><th align=\"right\" scope=\"col\">ID:</th><th scope=\"col\">");
    s += ID;
    s += F("</th></tr><tr><td align=\"right\">Value:</td><td>");
    s += Kwh;
    s += F("</td></tr><tr><td align=\"right\">Invert:</td><td>");
    s += Invert;
    s += F("</td></tr></table></body></html>");
    client.print(s);
  }
}
