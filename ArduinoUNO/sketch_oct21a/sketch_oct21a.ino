//LIBRERIAS
#include <Servo.h>

//IDENTIFICADORES DE FIGURAS GEOMETRICAS
typedef enum {CIRCULO, CUADRADO, TRIANGULO} figura_t;

//PINS
#define BASE     15
#define HOMBRO   13
#define CODO     12
#define MUNIECA  14

//MEDIDAS (mm)
#define L1 50
#define L2 50
#define L3 50
#define L4 50
#define LP 20 //SUJETO A CAMBIOS (ALTURA DEL LAPIZ CON RESPECTO A LA MUNIECA)

//PARAMETROS GENERALES
#define ARTICULACIONES 4
#define POSICION_REPOSO {PI/2,0,PI/2,PI/2};

//PARAMETROS PARA EL CONTROLADOR
#define DELTA_T   0.01  //PASO DEL TIEMPO DEL CONTROLADOR
#define GANANCIA  30    //GANANCIA DEL CONTROLADOR

//PARAMETROS DE DENAVIT-HARTENBERG
double theta[ARTICULACIONES] = POSICION_REPOSO;
double d[ARTICULACIONES] = {L1,0,0,0}; //{L1,0,0,L4};
double a[ARTICULACIONES] = {0,L2,L3,L4}; //{0,L2,L3,0};
double alpha[ARTICULACIONES] = {PI/2,0,0,0}; //{PI/2,0,0,-PI/2};

//VARIABLES DE POSICION Y ORIENTACIÓN + COORDENADAS ARTICULARES (THETA)
double x[6] = {0,0,0,0,0,0};
double q[ARTICULACIONES] = POSICION_REPOSO;

//PARAMETROS DE SERVOMOTORES
Servo servos[ARTICULACIONES];
const int posicion_reposo[ARTICULACIONES] = POSICION_REPOSO;
const int pin_servos[ARTICULACIONES] = {BASE,HOMBRO,CODO,MUNIECA};
const int min_pwm[ARTICULACIONES] = {500,500,500,500};
const int max_pwm[ARTICULACIONES] = {2400,2400,2400,2400};
const double min_angulo[ARTICULACIONES] = {0,0,0,0};
const double max_angulo[ARTICULACIONES] = {PI,PI,PI,PI};

void matrizCopiar(double A[][4], double Acopia[][4]){
  for (int i = 0; i < 4; i++){
    for (int j = 0; j < 4; j++){
      Acopia[i][j] = A[i][j];
    }
  }
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

void matrizImprimir(double* A, char m, char n){
  Serial.println("IMPRIMIENDO MATRIZ\n");
  for(int i = 0; i < m; i++){
    for (int j = 0; j < n; j++){
      Serial.print(*(A+i*n+j)); //A[i*n+j]
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

void cinematicaDirecta(double theta[], double d[], double a[], double alpha[], double q[], double x[]){
  double I[4][4] = {{1, 0, 0, 0},
                    {0, 1, 0, 0},
                    {0, 0, 1, 0},
                    {0, 0, 0, 1}};
  double Ai[4][4], T[4][4];

  //CALCULO MATRIZ DEL ROBOT
  for (int i = 0; i < ARTICULACIONES; i++){
    theta[i] = q[i];
    matrizTransformacionHomogenea(theta[i], d[i], a[i], alpha[i], Ai);
    //I = I * Ai
    matrizMultiplicar(I, Ai, T);
    matrizCopiar(T, I); //I se vuelve acumulativa, teniendo a 0A1, 0A2, ... hasta 0An=T finalmente
  }

  x[0] = T[0][3];
  x[1] = T[1][3];
  x[2] = T[2][3];
}

void cinematicaInversa(double x[], double q[]){
  double x_EE = x[0], z_EE = x[1], y_EE = x[2];
  double D, theta1, d, Yoffset, R, alpha1, alpha2, theta2, theta3, theta4;
  D = sqrt((x_EE*x_EE) + (z_EE*z_EE));
  theta1 = atan(z_EE/x_EE);
  if (y_EE >= L1) {
      d = D - L4;
      Yoffset = y_EE - L1;
      R = sqrt((d*d) + (Yoffset*Yoffset));
      alpha1 = acos(d/R);
      alpha2 = acos((L2*L2+R*R-L3*L3)/(2*L2*R));
      theta2 = alpha1 + alpha2;
      theta3 = acos((L2*L2+L3*L3-R*R)/(2*L2*L3));
      theta4 = 180 - ((180-(alpha2+theta3))-alpha1);
  } else if (y_EE <= L1) {
      d = D - L4;
      Yoffset = y_EE - L1;
      R = sqrt((d*d) + (Yoffset*Yoffset));
      alpha1 = acos(d/R);
      alpha2 = acos((L2*L2+R*R-L3*L3)/(2*L2*R));
      theta2 = -alpha1 + alpha2; //change
      theta3 = acos((L2*L2+L3*L3-R*R)/(2*L2*L3));
      theta4 = 180 - ((180-(alpha2+theta3))+alpha1); //change
  }
  q[0] = theta1;
  q[1] = theta2;
  q[2] = theta3;
  q[3] = theta4;
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
    cinematicaDirecta(theta, d, a, alpha, q, x);

    //CALCULO DE LA PROXIMA POSICION DEL ROBOT SEGUN EL TIPO DE FIGURA
    switch(tipoFigura){
      case CIRCULO:
        parametrizacionCirculo(x_coord, y_coord, z_coord, tamanio, i, x_fig);
        parametrizacionCirculo(x_coord, y_coord, z_coord, tamanio, i+1, x_fig_sigpos);
        break;
      case CUADRADO:
        //TODO
        break;
      case TRIANGULO:
        //TODO
        break;
      default: break;
    }
    
    for(int j = 0; j < 6; j++){
      x[j] = x_fig_sigpos[j] - x_fig[j] + GANANCIA * DELTA_T * (x_fig[j] - x[j]);
    }

    //CALCULA LOS ANGULOS DEL MOTOR PARA ALCANZAR LA SIGUIENTE POSICION
    for(int j = 0; j < ARTICULACIONES; j++){
      q_aux[j] = q[j]; //POSICION_REPOSO
    }
    cinematicaInversa(x, q);
    for(int j = 0; j < ARTICULACIONES; j++){
      q[j] = q_aux[j] + q[j];
    }

    Serial.print("ITERACION:");
    Serial.println(i);

    //ESCRIBIR LOS ANGULOS EN EL SERVOMOTOR
    servosArticulaciones(10);

    /*for(int j = 0; j < ARTICULACIONES; j++){
      Serial.print(q[j]*180/PI); //rad2deg
      Serial.print(" ");
    }*/
    Serial.println();
  }
  servosReposo(500);
}

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int angulo2pwm(double angulo, int i){
  //Ancho de pulso (o tiempo en alto) de la senial PWM
  return MIN(MAX(((min_pwm[i] - max_pwm[i]) / (min_angulo[i] - max_angulo[i])) * (angulo - min_angulo[i]) + min_pwm[i], min_pwm[i]), max_pwm[i]);
}

void servosInicializar(){
  for(int i = 0; i < ARTICULACIONES; i++){
     servos[i].attach(pin_servos[i], min_pwm[i], max_pwm[i]);
     servos[i].write(0);
     delay(100);
  }
  delay(2000);
}

void servosArticulaciones(int velocidad){
  for(int i = 0; i < ARTICULACIONES; i++){
    servos[i].writeMicroseconds(angulo2pwm(q[i], i));
    Serial.print(angulo2pwm(q[i], i));
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
  delay(5000);
  servosInicializar();
  x[0] = 120; x[1] = 50; x[2] = 20;
  cinematicaInversa(x, q);
  Serial.println("ANGULOS:");
  Serial.print(q[0]*180/PI);
  Serial.print(" ");
  Serial.print(q[1]*180/PI);
  Serial.print(" ");
  Serial.print(-(180-q[2]*180/PI));
  Serial.print(" ");
  Serial.print(180-q[3]*180/PI);
  Serial.println();
  for (int i = 0; i < 6; i++) x[i] = 0; //REINICIO
  q[0] = q[0];
  q[1] = q[1];
  q[2] = -(PI-q[2]);
  q[3] = PI-q[3];
  cinematicaDirecta(theta, d, a, alpha, q, x);
  Serial.println("POSICIONES:");
  for (int i = 0; i < 6; i++){
    Serial.print(x[i]);
    Serial.print(" ");
  }
  Serial.println();
  q[0] = q[0] + PI/2; //map(q[i], -90, 90, 0, 180);
  q[2] = q[2] + PI/2;
  q[3] = q[3] + PI/2;
  Serial.println("IMPULSOS:");
  servosArticulaciones(10);
  Serial.println();
  /*Serial.println("DIBUJAR CIRCULO");
  dibujarFigura(120,50,LP,20,CIRCULO);
  Serial.println("FINALIZADO CIRCULO");*/
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Hola");
  delay(100000);
}
