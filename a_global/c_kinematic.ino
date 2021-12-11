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

/* Cinematica Inversa: realiza los calculos trigonometricos para transformar una posicion en el espacio 3D en angulos para los servomotores.
 * x[]: vector de posicion a transformar a angulos
 * q[]: vector de angulos resultantes de la transformacion aplicada (theta1=base, theta2=hombro, theta3=codo, theta4=munieca)
 */
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

/*
 * Parametrizacion de un Circulo: realiza los calculos parametricos de un circulo en sentido horario (antihorario en el modelo fisico).
 * x_coord: coordenada x del circulo a dibujar (desde el centro)
 * y_coord: coordenada y del circulo a dibujar (desde el centro)
 * z_coord: coordenada z del circulo a dibujar (desde el centro, se recomienda siempre usar LP)
 * radio: radio del circulo a dibujar
 * i: iteracion a la que corresponde el calculo parametrico
 * x_fig[]: vector de coordenadas resultantes de la parametrizacion
 */
void parametrizacionCirculo(double x_coord, double y_coord, double z_coord, double radio, int i, double x_fig[]){
  x_fig[0] = x_coord + radio * cos(2*PI*DELTA_T*i);
  x_fig[1] = y_coord + radio * sin(2*PI*DELTA_T*i);
  x_fig[2] = z_coord;
  x_fig[3] = 0;
  x_fig[4] = 0;
  x_fig[5] = 0;
}

/*
 * Dibujar Circulo: realiza el dibujo de un circulo a partir de las coordenadas y radio indicado en los parametros.
 * x_coord: coordenada x del circulo a dibujar (desde el centro)
 * y_coord: coordenada y del circulo a dibujar (desde el centro)
 * z_coord: coordenada z del circulo a dibujar (desde el centro, se recomienda siempre usar LP)
 * radio: radio del circulo a dibujar
 */
void dibujarCirculo(double x_coord, double y_coord, double z_coord, double radio){
  double x_fig[6];
  int N = ((int)(2/DELTA_T)+1); //Numero de pasos para cubrir el circulo dos veces
  for(int i = 0; i < N; i++){
    //CALCULO DE LA PROXIMA POSICION DEL ROBOT
    parametrizacionCirculo(x_coord, y_coord, z_coord, radio, i, x_fig);

    x[0] = x_fig[0]; x[1] = x_fig[1]; x[2] = x_fig[2];

    cinematicaInversa(x, q);

    //CONVERSION DE ANGULOS RELATIVOS A LAS ARTICULACIONES A SER ABSOLUTOS PARA CADA UNA DE ELLAS
    q[0] = q[0];
    q[1] = q[1];
    q[2] = -(PI-q[2]);
    q[3] = PI-q[3];

    //CONVERSION DE ANGULOS DEL MODELO MATEMATICO (-90..90) AL FISICO (0..180)
    q[0] = q[0] + PI/2; //map(q[i], -90, 90, 0, 180);
    q[2] = q[2] + PI/2;
    q[3] = q[3] + PI/2;

    //ESCRIBIR LOS ANGULOS EN EL SERVOMOTOR
    servosArticulaciones(10);
  }
  servosReposo(50);
}

/*
 * Dibujar Segmento de Recta: realiza el dibujo de un segmento de recta a partir de las coordenadas para dos puntos 2D con altura fija indicados en los parametros.
 * p1x: coordenada x del punto 1 a dibujar
 * p1y: coordenada y del punto 1 a dibujar
 * p2x: coordenada x del punto 2 a dibujar
 * p2y: coordenada y del punto 2 a dibujar
 * altura: coordenada z de ambos puntos a dibujar (se recomienda siempre usar LP)
 */
void dibujarSegmento(double p1x, double p1y, double p2x, double p2y, double altura){
  double N = 100+1; //((int)(2/DELTA_T)+1); //El +1 es porque sino va de 50 a -49 (deberia ir a -50)
  for (double t = 0; t < N; t++){
    //(1-t)(p1x,p1y) + t*(p2x,p2y)
    x[0] = (1-t/N) * p1x + (t/N) * p2x;
    x[1] = (1-t/N) * p1y + (t/N) * p2y;
    x[2] = altura;

    cinematicaInversa(x, q);

    //CONVERSION DE ANGULOS RELATIVOS A LAS ARTICULACIONES A SER ABSOLUTOS PARA CADA UNA DE ELLAS
    q[0] = q[0];
    q[1] = q[1];
    q[2] = -(PI-q[2]);
    q[3] = PI-q[3];

    //CONVERSION DE ANGULOS DEL MODELO MATEMATICO (-90..90) AL FISICO (0..180)
    q[0] = q[0] + PI/2; //map(q[i], -90, 90, 0, 180);
    q[2] = q[2] + PI/2;
    q[3] = q[3] + PI/2;

    //ESCRIBIR LOS ANGULOS EN EL SERVOMOTOR
    servosArticulaciones(10);
  }
}

/*
 * Dibujar Cuadrado: realiza el dibujo de un cuadrado a partir del dibujo de los 4 segmentos individuales que lo conforman.
 * xc: coordenada x del cuadrado a dibujar
 * yc: coordenada y del cuadrado a dibujar
 * tam: tamanio del lado del cuadrado a dibujar
 */
void dibujarCuadrado(int xc, int yc, int tam) {
  double tam2 = tam/2;
  dibujarSegmento(xc - tam2, yc + tam2, xc - tam2, yc - tam2, LP);
  dibujarSegmento(xc - tam2, yc - tam2, xc + tam2, yc - tam2, LP);
  dibujarSegmento(xc + tam2, yc - tam2, xc + tam2, yc + tam2, LP);
  dibujarSegmento(xc + tam2, yc + tam2, xc - tam2, yc + tam2, LP);
  servosReposo(50);
}

/*
 * Dibujar Triangulo: realiza el dibujo de un triangulo equilatero a partir del dibujo de los 3 segmentos individuales que lo conforman.
 * xc: coordenada x del triangulo equilatero a dibujar
 * yc: coordenada y del triangulo equilatero a dibujar
 * tam: tamanio del lado del triangulo equilatero a dibujar
 */
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

/*
 * Angulo a PWM: realiza la conversion de un angulo entre 0 a 180 grados a una senial PWM con pulso activo en un intervalo entre 500 a 2400 microsegundos.
 * angulo: es el angulo a convertir a senial PWM
 * i: articulacion a la que corresponde la conversion del angulo (i=0..3)
 */
int angulo2pwm(double angulo, int i){
  //Ancho de pulso (o tiempo en alto) de la senial PWM
  return MIN(MAX(((min_pwm[i] - max_pwm[i]) / (min_angulo[i] - max_angulo[i])) * (angulo - min_angulo[i]) + min_pwm[i], min_pwm[i]), max_pwm[i]);
}

/*
 * Inicializacion de Servos: realiza la conexion de los servomotores a los terminales asociados y su inicializacion en 90 grados (posicion de reposo).
 */
void servosInicializar(){
  for(int i = 0; i < ARTICULACIONES; i++){
     servos[i].attach(pin_servos[i], min_pwm[i], max_pwm[i]);
     servos[i].write(90);
     delay(100);
  }
  delay(2000);
}

/*
 * Articular los Servos: realiza la escritura de los angulos obtenidos de la cinematica inversa en los servomotores a una velocidad indicada en los parametros.
 * velocidad: retraso en milisegundos entre escrituras de angulos en los servomotores
 */
void servosArticulaciones(int velocidad){
  for(int i = 0; i < ARTICULACIONES; i++){
    servos[i].writeMicroseconds(angulo2pwm(q[i], i));
    delay(velocidad);
  }
}

/*
 * Colocar Servos en Reposo: realiza la escritura de los angulos en 90 grados (posicion de reposo) a una velocidad indicada en los parametros.
 * velocidad: retraso en milisegundos entre escrituras de angulos en los servomotores
 */
void servosReposo(int velocidad){
  for(int i = 0; i < ARTICULACIONES; i++){
    servos[i].write(90);
    delay(velocidad);
  }
}
