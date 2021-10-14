//Variables globales y librerías

//Librerías
//Servidor web del ESP8266
#include <ESP8266WebServer.h>

//Servidor web
ESP8266WebServer server(80);

//Dimensiones del papel
int dimX = 100;
int dimY = 200;

//Estado del brazo
boolean armAvailable = false;
