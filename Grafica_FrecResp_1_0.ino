#define mic 13
int respiraciones;
int times;

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
 
}

void loop() {
 //  check if the mic digitalout is high.

int sensorValue = analogRead(13);           //Le damos a la variable sensorValue el valor que lee la funcion analogRead en el pin 13 que es la entrada del sensor de sonido, 
                                            //que depende de la intensidad del sonido devuelve un valor de entre 0 y 1023 bits.
if (sensorValue > 120) {                    //Creamos un código que le da a otra variable (respiraciones) el valor de 1 si el sonido es muy alto, 
                                            //lectura por encima de 120 bits y si no un 0 si está por debajo.
  respiraciones = 1;
  } else {
   respiraciones = 0;
 }
 Serial.println(sensorValue);  //Este código es de control para en el serial plotter sacar la gráfica que sale correctamente
  
   delay(200);
}
