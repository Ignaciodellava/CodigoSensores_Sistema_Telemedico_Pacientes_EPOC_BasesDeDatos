

int sensorPin = A0;//Declaramos el Pin analógico que usaremos.
int sensorValue=0;//Variable para guardar la lectura del Pin analógico.
int temC=0;//Variable para calcular los grados Celsius.

void setup() {
  Serial.begin(9600);//Iniciamos el puero serie
  
}

void loop() {
  
  int value = analogRead(sensorPin); // lee la señal del pin analógico A0
  int millivolts = (value / 1023.0) * 5000; // convertimos los escalones al voltaje correspondiente
  temperatura = (millivolts / 10)-50;
  Serial.print(temperatura);// se imprime el valor temperatura
  Serial.println(" C");
  delay(2000);//Esperamos 2 segundos para hacer la próxima lectura.                  
}
