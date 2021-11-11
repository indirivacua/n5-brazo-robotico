//LIBRERIAS
#include <Servo.h>

//IDENTIFICADORES DE FIGURAS GEOMETRICAS
typedef enum {CIRCULO, CUADRADO, TRIANGULO} figura_t;

//PINS
#define BASE     5
#define HOMBRO   6
#define CODO     7
#define MUNIECA  8

//MEDIDAS (mm)
#define L1 45 //SUJETO A CAMBIOS
#define L2 75 //SUJETO A CAMBIOS
#define L3 75 //SUJETO A CAMBIOS
#define L4 40 //SUJETO A CAMBIOS
#define LP 20 //SUJETO A CAMBIOS (ALTURA DEL LAPIZ CON RESPECTO A LA MUNIECA)

//PARAMETROS GENERALES
#define ARTICULACIONES 4
#define POSICION_REPOSO {0,3*PI/4,-PI/2,-3*PI/4+PI/2}; //{0,0,0,0}; //SUJETO A CAMBIOS

//PARAMETROS PARA EL CONTROLADOR
#define DELTA_T   0.01  //PASO DEL TIEMPO DEL CONTROLADOR
#define GANANCIA  30    //GANANCIA DEL CONTROLADOR

//PARAMETROS DE DENAVIT-HARTENBERG
double theta[ARTICULACIONES] = POSICION_REPOSO;
double d[ARTICULACIONES] = {L1,0,0,0}; //{L1,0,0,L4};
double a[ARTICULACIONES] = {0,L2,L3,L4}; //{0,L2,L3,0};
double alpha[ARTICULACIONES] = {PI/2,0,0,0}; //{PI/2,0,0,-PI/2};

//VARIABLES DE POSICION Y ORIENTACIÓN + JACOBIANO + COORDENADAS ARTICULARES (THETA)
double x[6] = {0,0,0,0,0,0};
double J[6][ARTICULACIONES];
double q[ARTICULACIONES] = POSICION_REPOSO;

//PARAMETROS DE SERVOMOTORES
Servo servos[ARTICULACIONES];
const int posicion_reposo[ARTICULACIONES] = POSICION_REPOSO;
const int pin_servos[ARTICULACIONES] = {BASE,HOMBRO,CODO,MUNIECA};
const int min_pwm[ARTICULACIONES] = {500,500,600,500}; //{500,500,500,500}; //SUJETO A CAMBIOS
const int max_pwm[ARTICULACIONES] = {2500,2500,2600,2500}; //{2400,2400,2400,2400}; //SUJETO A CAMBIOS
const double min_angulo[ARTICULACIONES] = {-PI/2,PI,-2*PI/3,-PI/2}; //{-PI/2,-PI/2,-PI/2,-PI/2}; //SUJETO A CAMBIOS
const double max_angulo[ARTICULACIONES] = {PI/2,0,PI/2,PI/2}; //{PI/2,PI/2,PI/2,PI/2}; //SUJETO A CAMBIOS

void productoCruz(double u[3], double v[3], double w[3]){
  w[0] = u[1]*v[2] - u[2]*v[1];
  w[1] = u[2]*v[0] - u[0]*v[2];
  w[2] = u[0]*v[1] - u[1]*v[0];
}

void vectorResta(double u[3], double v[3], double w[3]){
  for (int i = 0; i < 3; i++){
    w[i] = u[i] - v[i];
  }
}

void matrizCopiar(double A[][4], double Acopia[][4]){
  memcpy(&Acopia[0][0], &A[0][0], 4*4*sizeof(A[0][0]));
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

//(mxn) * (nxp) = (mxp)
void matrizMultiplicar2(double A[4][6], double B[6][4], double C[4][4]){ //GENERALIZAR
  for (int i = 0; i < 4; i++){ //filas 1
    for (int j = 0; j < 4; j++){ //columnas 2
      C[i][j] = 0;
      for (int k = 0; k < 6; k++){ //columnas 1
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }
}

//(mxn) * (nxp) = (mxp)
void matrizMultiplicar3(double A[4][4], double B[4][6], double C[4][6]){ //GENERALIZAR
  for (int i = 0; i < 4; i++){ //filas 1
    for (int j = 0; j < 6; j++){ //columnas 2
      C[i][j] = 0;
      for (int k = 0; k < 4; k++){ //columnas 1
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }
}

void matrizTraspuesta(double A[][4], double Atraspuesta[][6]){
  for (int i = 0; i < 6; i++){
    for (int j = 0; j < 4; j++){
      Atraspuesta[j][i] = A[i][j];
    }
  }
}

//https://stackoverflow.com/questions/2937702/i-want-to-find-determinant-of-4x4-matrix-in-c-sharp/2937973#2937973
double matrizDeterminante(double A[][4]){
  return A[0][3] * A[1][2] * A[2][1] * A[3][0] - A[0][2] * A[1][3] * A[2][1] * A[3][0] -
         A[0][3] * A[1][1] * A[2][2] * A[3][0] + A[0][1] * A[1][3] * A[2][2] * A[3][0] +
         A[0][2] * A[1][1] * A[2][3] * A[3][0] - A[0][1] * A[1][2] * A[2][3] * A[3][0] -
         A[0][3] * A[1][2] * A[2][0] * A[3][1] + A[0][2] * A[1][3] * A[2][0] * A[3][1] +
         A[0][3] * A[1][0] * A[2][2] * A[3][1] - A[0][0] * A[1][3] * A[2][2] * A[3][1] -
         A[0][2] * A[1][0] * A[2][3] * A[3][1] + A[0][0] * A[1][2] * A[2][3] * A[3][1] +
         A[0][3] * A[1][1] * A[2][0] * A[3][2] - A[0][1] * A[1][3] * A[2][0] * A[3][2] -
         A[0][3] * A[1][0] * A[2][1] * A[3][2] + A[0][0] * A[1][3] * A[2][1] * A[3][2] +
         A[0][1] * A[1][0] * A[2][3] * A[3][2] - A[0][0] * A[1][1] * A[2][3] * A[3][2] -
         A[0][2] * A[1][1] * A[2][0] * A[3][3] + A[0][1] * A[1][2] * A[2][0] * A[3][3] +
         A[0][2] * A[1][0] * A[2][1] * A[3][3] - A[0][0] * A[1][2] * A[2][1] * A[3][3] -
         A[0][1] * A[1][0] * A[2][2] * A[3][3] + A[0][0] * A[1][1] * A[2][2] * A[3][3];
}

//https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix/1148405#1148405
bool gluInvertMatrix(double m[16], double invOut[16])
{
    double inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
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
  matrizCopiar(Aaux,Ai);
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
    theta[i] = q[i];

    matrizTransformacionHomogenea(theta[i], d[i], a[i], alpha[i], Ai);
    //matrizImprimir((double*) Ai, 4, 4);
    //I = I * Ai
    matrizMultiplicar(I, Ai, T);
    matrizCopiar(T, I); //I se vuelve acumulativa, teniendo a 0A1, 0A2, ... hasta 0An=T finalmente

    z[i+1][0]=T[0][2];
    z[i+1][1]=T[1][2];
    z[i+1][2]=T[2][2];

    t[i+1][0]=T[0][3];
    t[i+1][1]=T[1][3];
    t[i+1][2]=T[2][3];

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

  x[0] = T[0][3];
  x[1] = T[1][3];
  x[2] = T[2][3];
  /*for (int i = 0; i < 6; i++){
    Serial.print(x[i],DEC);
  }
  Serial.println();*/

  //CALCULO JACOBIANO
  //Ji = [z(i) x (t(n) - t(i)); z(i)], i=0..n-1
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
  //matrizImprimir((double*) J, 6, ARTICULACIONES);

  /*double A[4][5] = {{1,2,3,4,5},{6,7,8,9,10},{11,12,13,14,15},{16,17,18,19,20}};
  matrizImprimir((double*) A, 4, 5);*/
}

void cinematicaInversa(){
  double Jtraspuesta[ARTICULACIONES][6];
  double JtraspuestaJ[ARTICULACIONES][ARTICULACIONES];
  double JtraspuestaJ_inversa[ARTICULACIONES][ARTICULACIONES];
  double JtraspuestaJ_inversaJtraspuesta[ARTICULACIONES][6]; //PSEUDOINVERSA

  //CALCULO PSEUDOINVERSA DE MOORE-PENROSE
  matrizTraspuesta(J, Jtraspuesta);
  //matrizImprimir((double*) Jtraspuesta, ARTICULACIONES, 6);
  matrizMultiplicar2(Jtraspuesta, J, JtraspuestaJ);
  //matrizImprimir((double*) JtraspuestaJ, ARTICULACIONES, ARTICULACIONES);

  /*double A[4][4] = {{1,-2,2,2},{0,4,-2,1},{1,-2,4,0},{1,-1,2,2}};
  Serial.println("DETERMINANTE");
  Serial.println(matrizDeterminante(A),DEC); //2
  double Ainv[4][4];
  Serial.println("INVERSA");
  gluInvertMatrix((double*) A, (double*) Ainv);
  matrizImprimir((double*) Ainv, 4, 4);*/

  //COMPROBAR SI Jtraspuesta*J ES INVERTIBLE (ES NO SINGULAR)
  //Esto se hace para no entrar en un calculo recursivo, donde se hace la pseudoinversa del caso i+1 para calcular la pseudoinversa del caso i (y asi sucesivamente)
  if(matrizDeterminante(JtraspuestaJ) >= 0.1){
    //CONTINUAR CON EL CALCULO PSEUDOINVERSA DE MOORE-PENROSE
    gluInvertMatrix((double*) JtraspuestaJ, (double*) JtraspuestaJ_inversa);
    matrizMultiplicar3(JtraspuestaJ_inversa, Jtraspuesta, JtraspuestaJ_inversaJtraspuesta);
    //matrizImprimir((double*) JtraspuestaJ_inversaJtraspuesta, ARTICULACIONES, 6);

    //RESOLVER SISTEMA x=J(THETA)*THETA MEDIANTE THETA=J(THETA)^-1*x (METODO MINIMOS CUADRADOS)
    for(int i = 0; i < ARTICULACIONES; i++){
      q[i] = 0;
      for(int j = 0; j < 6; j++){
        q[i] += JtraspuestaJ_inversaJtraspuesta[i][j]*x[j];
      }
    }

    /*Serial.println("COORD ARTICULARES");
    for(int i = 0; i < 6; i++){
      Serial.println(q[i],DEC);
      Serial.print(" ");
    }*/
  }
}

void parametrizacionCirculo(double x_coord, double y_coord, double z_coord, double radio, int i, double x_fig[]){
  x_fig[0] = x_coord + radio * cos(2*PI*DELTA_T*i);
  x_fig[1] = y_coord + radio * sin(2*PI*DELTA_T*i);
  x_fig[2] = z_coord;
  x_fig[3] = 0;
  x_fig[4] = 0;
  x_fig[5] = 0;
}

void dibujarFigura(double x_coord, double y_coord, double z_coord, double tamanio, figura_t tipoFigura){
  double x_fig[6], x_fig_sigpos[6], q_aux[ARTICULACIONES];
  int N = ((int)(2/DELTA_T)+1); //Numero de pasos para cubrir el circulo dos veces
  for(int i = 0; i < N; i++){
    //CALCULO DE LA POSICION DEL ROBOT
    cinematicaDirecta();

    //CALCULO DE LA PROXIMA POSICION DEL ROBOT SEGUN EL TIPO DE FIGURA
    switch(tipoFigura){
      case CIRCULO:
        parametrizacionCirculo(x_coord, y_coord, z_coord, tamanio, i, x_fig);
        /*Serial.println("x_fig");
        for(int j = 0; j < 6; j++){
          Serial.println(x_fig[j],DEC);
        }*/
        parametrizacionCirculo(x_coord, y_coord, z_coord, tamanio, i+1, x_fig_sigpos);
        /*Serial.println("x_fig_sigpos");
        for(int j = 0; j < 6; j++){
          Serial.println(x_fig_sigpos[j],DEC);
        }*/
        break;
      case CUADRADO:
        //TODO
        break;
      case TRIANGULO:
        //TODO
        break;
      default: break;
    }
    
    //Serial.println("x");
    for(int j = 0; j < 6; j++){
      x[j] = x_fig_sigpos[j] - x_fig[j] + GANANCIA * DELTA_T * (x_fig[j] - x[j]);
      //Serial.println(x[j],DEC);
    }

    //CALCULA LOS ANGULOS DEL MOTOR PARA ALCANZAR LA SIGUIENTE POSICION
    for(int j = 0; j < ARTICULACIONES; j++){
      q_aux[j] = q[j]; //POSICION_REPOSO
    }
    cinematicaInversa();
    //matrizImprimir((double*) J, 6, ARTICULACIONES);
    //Serial.println("q");
    for(int j = 0; j < ARTICULACIONES; j++){
      //Serial.println(q[j]);
      q[j] = q_aux[j] + q[j];
    }

    Serial.print("ITERACION:");
    Serial.println(i);

    //ESCRIBIR LOS ANGULOS EN EL SERVOMOTOR
    servosArticulaciones(10);

    /*for(int j = 0; j < ARTICULACIONES; j++){
      Serial.print(q[j]*180/PI,DEC); //rad2deg
      Serial.print(" ");
    }*/
    Serial.println();
  }
  servosReposo(500);
}

int angulo2pwm(double angulo, int i){
  //Ancho de pulso (o tiempo en alto) de la senial PWM
  return min(max(((min_pwm[i] - max_pwm[i]) / (min_angulo[i] - max_angulo[i])) * (angulo - min_angulo[i]) + min_pwm[i], min_pwm[i]), max_pwm[i]);
}

void servosInicializar(){
  for(int i = 0; i < ARTICULACIONES; i++){
     servos[i].attach(pin_servos[i], min_pwm[i], max_pwm[i]);
     servos[i].write(0);
  }
  delay(2000);
}

void servosArticulaciones(int velocidad){
  for(int i = 0; i < ARTICULACIONES; i++){
    servos[i].writeMicroseconds(angulo2pwm(q[i], i));
    Serial.print(angulo2pwm(q[i], i),DEC);
    Serial.print(" ");
    delay(velocidad);
  }
}

void servosReposo(int velocidad){
  for(int i = 0; i < ARTICULACIONES; i++){
    servos[i].writeMicroseconds(angulo2pwm(posicion_reposo[i], i));
    delay(velocidad);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //cinematicaDirecta();
  //cinematicaInversa();
  servosInicializar();
  Serial.println("DIBUJAR CIRCULO");
  dibujarFigura(165,0,LP,20,CIRCULO);
  Serial.println("FINALIZADO CIRCULO");
}

void loop() {
  // put your main code here, to run repeatedly:

}
