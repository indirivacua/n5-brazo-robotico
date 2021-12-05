//Setup y bucle principal

//Setup
void setup(){
  delay(1000);
  //Inicializar comunicación wifi y servidor web
  web_init();
  servosInicializar();
  Serial.begin(9600);
  //Indicar que el brazo se encuentra disponible luego de inicializar todo
  armAvailable = true;
}

//Bucle principal
void loop(){
  //Atender requests del servidor web
  server.handleClient();
  if (!armAvailable){
    
    dibujoX += orgX;
    dibujoY += orgY;
    Serial.println(dibujoX);
    Serial.println(dibujoY);
    Serial.println(dibujoTamanio);
    if (dibujoTipo == "circle"){
      dibujarCirculo(dibujoX,dibujoY,LP,dibujoTamanio);
    } else if (dibujoTipo == "square"){
      dibujarCuadrado(dibujoX,dibujoY,dibujoTamanio);
    } else if (dibujoTipo == "triangle"){
      dibujarTriangulo(dibujoX,dibujoY,dibujoTamanio);
    }
    armAvailable = true;
  }
}
