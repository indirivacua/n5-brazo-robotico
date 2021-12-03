//Manejo del ESP8266 para comunicación wifi y servidor web de este

//Macro para calcular el tamaño
#define CHECKSIZE (x+h2 > dimX) || (x-h2 < 0) || (y+h2 > dimY) || (y-h2 < 0)

//Parámetros de la red wifi
const char *ssid = "Robotic arm"; //Id

//Handlers para requests
//Root: ruta raiz, usada para testeos
void handleRoot(){
  server.send(200, "text/html", "<h1>Reply from ESP8266</h1>");
}

//Ruta no encontrada
void handleNotFound(){
  server.send(200, "text/html", "<h1>404: Not Found</h1>");
}

//Draw: le indica al robot que dibuje una figura
/*Recibe los siguientes parámetros HTTP:
 * posX: coordenada x del centro de la figura
 * posY: coordenada y del centro de la figura 
 * shape: figura a dibujar
 * size: parámetro de tamaño de la figura (radio para círculo, lado para cuadrado y triángulo)
 */
void handleDraw(){
  String msg;
  int x,y,s,h2;
  //Chequear disponibilidad del brazo
  if(!armAvailable) msg = "Brazo ocupado";
  //Chequear número mínimo de parámetros
  else if(server.args() < 4) msg = "Parámetros insuficientes";
  //Chequear nombres de los parámetros
  else if(!(server.argName(0) == "posX" && server.argName(1) == "posY" && server.argName(2) == "shape" && server.argName(3) == "size")) msg = "Parámetros incorrectos";
  //Chequear que el tamaño no sea 0
  else if(server.arg(3).toInt() == 0) msg = "El tamaño no puede ser 0";
  //Chequear si la figura es válida y si el dibujo entra en la hoja, si es así dibujarla
  else{
    x=server.arg(0).toInt();
    y=server.arg(1).toInt();
    s=server.arg(3).toInt();
    dibujoX = x;
    dibujoY = y;
    dibujoTamanio = s;
    dibujoTipo = server.arg(2);
    if(server.arg(2) == "triangle"){
      h2 = sqrt(3) * s/4;
      if(CHECKSIZE) msg = "Espacio insuficiente";
      else
      {
        msg="Dibujando";
        armAvailable = false; //Indicar que el brazo está ocupado
        //Dibujar triangulo
      }
    }
    else if(server.arg(2) == "square"){
      h2 = s/2;
      if(CHECKSIZE) msg = "Espacio insuficiente";
      else
      {
        armAvailable = false; //Indicar que el brazo está ocupado
        msg="Dibujando";
        //Dibujar cuadrado
      }
    }
    else if(server.arg(2) == "circle"){
      h2 =s;
      if(CHECKSIZE) msg = "Espacio insuficiente";
      else
      {
        armAvailable = false; //Indicar que el brazo está ocupado
        msg="Dibujando";
        //Dibujar círculo
      }
    }
    else msg = "La figura indicada no existe";
  }
  //Enviar respuesta
  server.send(200, "text/html", msg);
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
  //Levantar red wifi
  WiFi.softAP(ssid);

  //linkear handlers con sus rutas
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.on("/available",handleAvailable);
  server.on("/dimensions",handleDimensions);
  server.on("/draw",handleDraw);
  //Levantar servidor
  server.begin();
}
