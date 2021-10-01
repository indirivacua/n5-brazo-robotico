//Manejo del ESP8266 para comunicación wifi y servidor web de este

//Parámetros de la red wifi
const char *ssid = "Robotic arm"; //Id
const char *pwd = "1234";         //Contraseña
const int mc = 1;                 //Máxima cantidad de conexiones

//Handlers para requests
//Root: ruta raiz, usada para testeos
void handleRoot(){
  server.send(200, "text/html", "<h1>Reply from ESP8266</h1>");
}

//Ruta no encontrada
void handleNotFound(){
  server.send(200, "text/html", "<h1>404: Not Found</h1>");
}

//Draw: le indica al robot que dibuje
void handleDraw(){
}

//Available: indica si el brazo robótico está disponible para dibujar
void handleAvailable(){
  if(armAvailable) server.send(200, "text/html", "true");
  else server.send(200, "text/html", "false");
}

//Dimensions: devuleve las dimensiones de la hoja en un formato x:dimx,y:dimy;
void handleDimensions(){
  String msg = "x:";
  msg+=String(dimX) + ",y:"+String(dimY);
  server.send(200, "text/html", msg);
}


//Inicializar la comunicación wifi y el servidor web 
void web_init(){
  //WiFi.softAP(ssid,pwd,1,false,mc);
  //Activar wifi
  WiFi.softAP(ssid);

  //linkear handlers con sus rutas
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.on("/available",handleAvailable);
  server.on("/dimensions",handleDimensions);

  //Levantar servidor
  server.begin();
}
