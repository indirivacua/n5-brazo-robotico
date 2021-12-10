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

//PARAMETROS PARA EL CONTROLADOR
#define DELTA_T   0.01 //PASO DEL TIEMPO DEL CONTROLADOR

//VARIABLES DE POSICION Y ORIENTACIÓN + COORDENADAS ARTICULARES (THETA)
double x[6] = {0,0,0,0,0,0};
double q[ARTICULACIONES];

//PARAMETROS DE SERVOMOTORES
Servo servos[ARTICULACIONES];
const int pin_servos[ARTICULACIONES] = {BASE,HOMBRO,CODO,MUNIECA};
const int min_pwm[ARTICULACIONES] = {500,500,500,500};
const int max_pwm[ARTICULACIONES] = {2400,2400,2400,2400};
const double min_angulo[ARTICULACIONES] = {0,0,0,0};
const double max_angulo[ARTICULACIONES] = {PI,PI,PI,PI};

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

void dibujarCirculo(double x_coord, double y_coord, double z_coord, double radio){
  double x_fig[6];
  int N = ((int)(2/DELTA_T)+1); //Numero de pasos para cubrir el circulo dos veces
  for(int i = 0; i < N; i++){
    //CALCULO DE LA PROXIMA POSICION DEL ROBOT
    parametrizacionCirculo(x_coord, y_coord, z_coord, radio, i, x_fig);

    x[0] = x_fig[0]; x[1] = x_fig[1]; x[2] = x_fig[2];

    cinematicaInversa(x, q);

    q[0] = q[0];
    q[1] = q[1];
    q[2] = -(PI-q[2]);
    q[3] = PI-q[3];

    q[0] = q[0] + PI/2; //map(q[i], -90, 90, 0, 180);
    q[2] = q[2] + PI/2;
    q[3] = q[3] + PI/2;

    //ESCRIBIR LOS ANGULOS EN EL SERVOMOTOR
    servosArticulaciones(10);
  }
  servosReposo(50);
}

void dibujarSegmento(double p1x, double p1y, double p2x, double p2y, double altura){
  double N = 100+1; //((int)(2/DELTA_T)+1); //El +1 es porque sino va de 50 a -49 (debería ir a -50)
  for (double t = 0; t < N; t++){
    //(1-t)(p1x,p1y) + t*(p2x,p2y)
    x[0] = (1-t/N) * p1x + (t/N) * p2x;
    x[1] = (1-t/N) * p1y + (t/N) * p2y;
    x[2] = altura;

    cinematicaInversa(x, q);

    q[0] = q[0];
    q[1] = q[1];
    q[2] = -(PI-q[2]);
    q[3] = PI-q[3];

    q[0] = q[0] + PI/2; //map(q[i], -90, 90, 0, 180);
    q[2] = q[2] + PI/2;
    q[3] = q[3] + PI/2;

    //ESCRIBIR LOS ANGULOS EN EL SERVOMOTOR
    servosArticulaciones(10);
  }
}

void dibujarCuadrado(int xc, int yc, int tam) {
  double tam2 = tam/2;
  dibujarSegmento(xc - tam2, yc + tam2, xc - tam2, yc - tam2, LP);
  dibujarSegmento(xc - tam2, yc - tam2, xc + tam2, yc - tam2, LP);
  dibujarSegmento(xc + tam2, yc - tam2, xc + tam2, yc + tam2, LP);
  dibujarSegmento(xc + tam2, yc + tam2, xc - tam2, yc + tam2, LP);
  servosReposo(50);
}

void dibujarTriangulo(int xc, int yc, int tam) {
  double tam2 = tam/2;
  double h2 = sqrt(3) *  tam/4;
  dibujarSegmento(xc - h2, yc + tam2, xc - h2, yc - tam2, LP);
  dibujarSegmento(xc - h2, yc - tam2, xc + h2, yc, LP);
  dibujarSegmento(xc + h2, yc, xc - h2, yc + tam2, LP);
  servosReposo(50);
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
     servos[i].write(90);
     delay(100);
  }
  delay(2000);
}

void servosArticulaciones(int velocidad){
  for(int i = 0; i < ARTICULACIONES; i++){
    servos[i].writeMicroseconds(angulo2pwm(q[i], i));
    delay(velocidad);
  }
}

void servosReposo(int velocidad){
  for(int i = 0; i < ARTICULACIONES; i++){
    servos[i].write(90);
    delay(velocidad);
  }
}
