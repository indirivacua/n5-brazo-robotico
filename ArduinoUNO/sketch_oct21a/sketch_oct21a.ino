// Librerias
#include <Servo.h>

//PINS
#define BASE_IN1 5
#define BASE_IN2 6
#define BASE_IN3 7
#define BASE_IN4 8
#define HOMBRO   1
#define CODO     2
#define MUNIECA  3

//MEDIDAS (MM)
#define L1 10 //SUJETO A CAMBIOS
#define L2 10 //SUJETO A CAMBIOS
#define L3 10 //SUJETO A CAMBIOS
#define L4 10 //SUJETO A CAMBIOS

//PARAMETROS GENERALES
#define ARTICULACIONES 4
#define POSICION_REPOSO {0,0,0,0}; //SUJETO A CAMBIOS

//PARAMETROS DE DENAVIT-HARTENBERG
double theta[ARTICULACIONES] = POSICION_REPOSO;
double d[ARTICULACIONES] = {L1,0,0,L4};
double a[ARTICULACIONES] = {0,L2,L3,0};
double alpha[ARTICULACIONES] = {PI/2,0,0,-PI/2};

//VARIABLES DE POSICION Y ANGULOS
double tN[3];

// Copiar matriz B en A
void matrizCopiar(double A[][4], double B[][4]){
  memcpy(&A[0][0], &B[0][0], 4*4*sizeof(A[0][0]));
}

void matrizMultiplicar(double A[][4], double B[][4], double C[][4]){
  for (int i = 0; i < 4; i++){ //filas 1
    for (int j = 0; j < 4; j++){ //columnas 2
      C[i][j] = 0;
      for (int k = 0; k < 4; k++){ //columnas 1
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }
}
void matrizImprimir(double A[][4]){
  Serial.println("IMPRIMIENDO MATRIZ\n");
  for(int i = 0; i < 4; i++){
    for (int j = 0; j < 4; j++){
      Serial.print(A[i][j], DEC);
      Serial.print(" \t"); // prints a tab
    }
    Serial.println();
  }
}

void matrizTransformacionHomogenea(double theta, double d, double a, double alpha, double Ai[][4]){
  double Aaux[4][4] = {{cos(theta), -cos(alpha)*sin(theta),  sin(alpha)*sin(theta), a*cos(theta)},
                       {sin(theta),  cos(alpha)*cos(theta), -sin(alpha)*cos(theta), a*sin(theta)},
                       {0         ,  sin(alpha)           ,  cos(alpha)           , d           },
                       {0         ,  0                    ,  0                    , 1           }};
  matrizCopiar(Ai,Aaux);
}

void cinematicaDirecta(){
  double I[4][4] = {{1, 0, 0, 0},
                    {0, 1, 0, 0},
                    {0, 0, 1, 0},
                    {0, 0, 0, 1}};
  double Ai[4][4], T[4][4];
  for (int i = 0; i < ARTICULACIONES; i++){
    matrizTransformacionHomogenea(theta[i], d[i], a[i], alpha[i], Ai);
    //matrizImprimir(Ai); //Serial.print("Hello world.");
    //I=I*Ai
    matrizMultiplicar(I, Ai, T);
    matrizCopiar(I, T); //I se vuelve acumulativa, teniendo a 0An finalmente, o sea, la matriz del robot
  }
  matrizImprimir(T);
  tN[0] = T[0][3];
  tN[1] = T[1][3];
  tN[2] = T[2][3];
}
void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  cinematicaDirecta();
}

void loop() {
  // put your main code here, to run repeatedly:

}
