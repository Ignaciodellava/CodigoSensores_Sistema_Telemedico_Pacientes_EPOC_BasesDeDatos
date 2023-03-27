#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
MAX30105 particleSensor;
#define MAX_BRIGHTNESS 255
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
//Arduino Uno no tiene sufieciente SRAM para almacenar 100 muestras de los datos del led IR y R en formato de 32 bits
//Para solventar este problema, truncamos las muestras para obtener el formato de 16 bits 
uint16_t irBuffer[100]; // Almacena los datos del Led IR en formato 16 bits
uint16_t redBuffer[100];  //Almacena los datos del Led R en formato 16 bits
#else
uint32_t irBuffer[100]; //Almacena los datos del Led IR en formato 32 bits
uint32_t redBuffer[100];  //Almacena los datos del Led R en formato 32 bits
#endif
int32_t bufferLength; //longitud de los datos
int32_t spo2; //valor de la saturacion de oxigeno en sangre
int8_t validSPO2; //indica si el dato medido es valido
int32_t heartRate; //valor de la frecuencia cardiaca
int8_t validHeartRate; //indica si el dato medido es valido
byte pulseLED = 11; //tiene que ser el pin PWM
byte readLED = 13; //parpadea cada vez que se lee un dato 
void setup()
{
  Serial.begin(115200); // se inicializa el serial con una comunicación de 11500 baudios
  // se inicializan los pines digitales
  pinMode(pulseLED, OUTPUT); 
  pinMode(readLED, OUTPUT);
  // Se inicializa el sensor 
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //usa el puerto determinado I2C y una velocidad de 400KHz
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }
  Serial.println(F("Attach sensor to finger with rubber band. Press any key to start conversion"));
  while (Serial.available() == 0) ; //espera hasta que presiones cualquier tecla 
  Serial.read();
  //settings del sensor
  byte ledBrightness = 60; //Opciones: 0=Off a 255=50mA  --> hay que seleccionar una opción 
  byte sampleAverage = 4; //Opciones: 1, 2, 4, 8, 16, 32 -->hay que seleccionar una opción 
  byte ledMode = 2; //Options: 1 = solo rojo, 2 =rojo + IR, 3 = rojo + IR + verde    --> hay que seleccionar una opción 
  byte sampleRate = 100; //Opciones: 50, 100, 200, 400, 800, 1000, 1600, 3200  --> hay que seleccionar una opción 
  int pulseWidth = 411; //Options: 69, 118, 215, 411 -->hay que seleccionar una opción 
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384 --> hay que seleccionar una opción 
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Se configura el sensor con los settings elegidos anteriormente
}
void loop()
{
  bufferLength = 100; //Guarda 4 segundos de samples a 25sps
  //rlee las 100 primeras muestras y determina el rango de la señal 
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false) //¿Hay nuevos datos?
      particleSensor.check(); //comprueba si hay nuevos datos
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample(); //Pasa a la siguiente muestra
    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);
  }
  //calcula la frecuencia cardiaca y la saturación de oxígeno en sangre tras las primeras 100 muestras (4 segundos )
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  //Se toman muetsras continuamente del sensor y se calcula la fc y SO2 cada segundo
  while (1)
  {
    //deshecha las primeras 25 muestras y mueve las 75 posteriores al inicio
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }
   
    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor.available() == false) 
        particleSensor.check(); 
      digitalWrite(readLED, !digitalRead(readLED)); //blink cuando se leen le los datos 
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); 
      //manda las muestras y los resultados al serial del programa 
      Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.print(irBuffer[i], DEC);
      Serial.print(F(", HR="));
      Serial.print(heartRate, DEC);
      Serial.print(F(", HRvalid="));
      Serial.print(validHeartRate, DEC);
      Serial.print(F(", SPO2="));
      Serial.print(spo2, DEC);
      Serial.print(F(", SPO2Valid="));
      Serial.println(validSPO2, DEC);
    }
    //Despues de 25 nuevas muetsras se recalculan los datos 
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }
}
