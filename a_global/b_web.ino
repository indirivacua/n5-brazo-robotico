const char *ssid = "Brazo robótico";
const char *pwd = "1234";
const int mc = 1;

void handleRoot(){
  server.send(200, "text/html", "<h1>Reply from ESP8266</h1>");
}

void handleNotFound(){
}

void handleDraw(){
}

void handleAvailable(){
}


void web_init(){
  //WiFi.softAP(ssid,pwd,1,false,mc);
  WiFi.softAP(ssid);
  server.on("/", handleRoot);
  server.begin();
}
