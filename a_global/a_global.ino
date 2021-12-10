//Variables globales y librerías

//Librerías
//Servidor web del ESP8266
#include <ESP8266WebServer.h>
#include <Servo.h>

//Servidor web
ESP8266WebServer server(80);

//Dimensiones del papel
int dimX = 30;
int dimY = 70;

//Origen de la pizarra
int orgX = 110; 
int orgY = -35; 

//Parámetros del dibujo
int dibujoX;
int dibujoY;
int dibujoTamanio;
String dibujoTipo;

//Estado del brazo
boolean armAvailable = false;
