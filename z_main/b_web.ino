const char *ssid = "Brazo robótico";


void handleRoot(){
  server.send(200, "text/html", "<h1>Reply from ESP8266</h1>");
}


void web_init(){
  WiFi.softAP(ssid);
  server.on("/", handleRoot);
  server.begin();
}
