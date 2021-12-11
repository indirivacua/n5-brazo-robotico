//Setup y bucle principal

//Setup
void setup(){
  delay(1000);
  //Inicializar comunicación wifi y servidor web
  web_init();
  servosInicializar();
  //Indicar que el brazo se encuentra disponible luego de inicializar todo
  armAvailable = true;
}

//Bucle principal
void loop(){
  //Atender requests del servidor web
  server.handleClient();

  //De haber un dibujo pendiente realizarlo
  if (!armAvailable){
    //Desplazar centro según el origen
    dibujoX += orgX;
    dibujoY += orgY;
    //Realizar dibujo correspondiente
    if (dibujoTipo == "circle"){
      dibujarCirculo(dibujoX,dibujoY,LP,dibujoTamanio);
    } else if (dibujoTipo == "square"){
      dibujarCuadrado(dibujoX,dibujoY,dibujoTamanio);
    } else if (dibujoTipo == "triangle"){
      dibujarTriangulo(dibujoX,dibujoY,dibujoTamanio);
    }
    //Indicar que se realizó el dibujo
    armAvailable = true;
  }
}
