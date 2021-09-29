const char *ssid = "Robotic arm";
const char *pwd = "1234";
const int mc = 1;

boolean armAvailable = false;

void handleRoot(){
  server.send(200, "text/html", "<h1>Reply from ESP8266</h1>");
}

void handleNotFound(){
  server.send(200, "text/html", "<h1>404: Not Found</h1>");
}

void handleDraw(){
}

void handleAvailable(){
  if(armAvailable) server.send(200, "text/html", "true");
  else server.send(200, "text/html", "false");
}

void handleDimensions(){
  String msg = "x:";
  msg+=String(dimX) + ",y:"+String(dimY);
  server.send(200, "text/html", msg);
}

void web_init(){
  //WiFi.softAP(ssid,pwd,1,false,mc);
  WiFi.softAP(ssid);
  
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  
  server.on("/available",handleAvailable);
  server.on("/dimensions",handleDimensions);
  
  server.begin();
}
