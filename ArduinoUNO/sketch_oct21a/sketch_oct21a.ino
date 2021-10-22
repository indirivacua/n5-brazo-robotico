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
double J[6][ARTICULACIONES];

void productoCruz(double u[3], double v[3], double w[3]){
  w[0] = u[1]*v[2] - u[2]*v[1];
  w[1] = u[2]*v[0] - u[0]*v[2];
  w[2] = u[0]*v[1] - u[1]*v[0];
}

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

void vectorResta(double A[3], double B[3], double C[3]){
  for (int i = 0; i < 3; i++){
    C[i] = A[i] - B[i];
  }
}

void matrizImprimir(double* A, char m, char n){
  Serial.println("IMPRIMIENDO MATRIZ\n");
  for(int i = 0; i < m; i++){
    for (int j = 0; j < n; j++){
      Serial.print(*(A+i*n+j), DEC); //A[i*n+j]
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
  //VARIABLES PARA JACOBIANO
  double z[ARTICULACIONES+1][3];
  double t[ARTICULACIONES+1][3];
  double prodcruz[3];
  double auxresta[3];
  
  z[0][0] = 0;
  z[0][1] = 0;
  z[0][2] = 1;

  t[0][0] = 0;
  t[0][1] = 0;
  t[0][2] = 0;

  //CALCULO MATRIZ DEL ROBOT + ARGUMENTOS PARA EL JACOBIANO
  for (int i = 0; i < ARTICULACIONES; i++){
    matrizTransformacionHomogenea(theta[i], d[i], a[i], alpha[i], Ai);
    //matrizImprimir((double*) Ai, 4, 4);
    //I=I*Ai
    matrizMultiplicar(I, Ai, T);
    matrizCopiar(I, T); //I se vuelve acumulativa, teniendo a 0A1, 0A2, ... hasta 0An finalmente, o sea, la matriz del robot T

    t[i+1][0]=T[0][3];
    t[i+1][1]=T[1][3];
    t[i+1][2]=T[2][3];

    z[i+1][0]=T[0][2];
    z[i+1][1]=T[1][2];
    z[i+1][2]=T[2][2];

    /*Serial.print("Iteracion:");
    Serial.print(i+1,DEC);
    Serial.println();
    for(int j = 0; j < 3; j++){
      Serial.print(z[i+1][j],DEC);
      Serial.print(" ");
    }
    Serial.println();
    for(int j = 0; j < 3; j++){
      Serial.print(t[i+1][j],DEC);
      Serial.print(" ");
    }
    Serial.println();*/
  }
  //matrizImprimir((double*) T, 4, 4);
  
  tN[0] = T[0][3];
  tN[1] = T[1][3];
  tN[2] = T[2][3];

  //CALCULO JACOBIANO
  //Ji = [z(i-1) x tN - t(i-1); z(i-1)]
  for(int i = 0; i < ARTICULACIONES; i++){
    vectorResta(t[ARTICULACIONES], t[i], auxresta);
    productoCruz(z[i], auxresta, prodcruz);
    J[0][i] = prodcruz[0];
    J[1][i] = prodcruz[1];
    J[2][i] = prodcruz[2];
    J[3][i] = z[i][0];
    J[4][i] = z[i][1];
    J[5][i] = z[i][2];
  }
  matrizImprimir((double*) J, 6, ARTICULACIONES);
}

void cinematicaInversa(){
  
}

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  cinematicaDirecta();
  cinematicaInversa();
}

void loop() {
  // put your main code here, to run repeatedly:

}
