//Setup y bucle principal

//Setup
void setup(){
  delay(1000);
  //Inicializar comunicación wifi y servidor web
  web_init();

  //Indicar que el brazo se encuentra disponiblel
  armAvailable = true;
}

//Bucle principal
void loop(){
  //Atender requests del servidor web
  server.handleClient();
}
