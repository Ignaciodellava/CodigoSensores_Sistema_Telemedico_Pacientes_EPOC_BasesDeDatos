

#if ! (ESP8266 || ESP32 )
  #error This code is intended to run on the ESP8266/ESP32 platform! Please check your Tools->Board setting
#endif

#include "Credentials.h"

#define MYSQL_DEBUG_PORT      Serial

// Debug Level from 0 to 4
#define _MYSQL_LOGLEVEL_      1

#include <MySQL_Generic.h>

#define USING_HOST_NAME     false

#if USING_HOST_NAME
  // Optional using hostname, and Ethernet built-in DNS lookup
  char server[] = "your_account.ddns.net"; // change to your server's hostname/URL
#else
//IP Adress de la base de datos 
  IPAddress server(192, 168, 2, 112);
#endif

uint16_t server_port = 3306;    //3306;

char default_database[] = "prbSafeBreath";           //"nombre de la base de datos ";
char default_table[]    = "pulsioximetro";          //"tabla a la que se van a insertar los tados ";

//Se instancian las variables 
int id_P = 0;




// bibliotecas del necesarias para el sensor

#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h" 

MAX30105 particleSensor;

#define MAX_BRIGHTNESS 255

// se inicializan las variables necesarias para el calculo de la frecuencia cardiaca 
//Como el arduino uno no tiene suficiente sram hay que truncar de 32 bits a 16 bits 
#if defined(_AVR_ATmega328P) || defined(AVR_ATmega168_)
//Arduino Uno doesn't have enough SRAM to store 100 samples of IR led data and red led data in 32-bit format
//To solve this problem, 16-bit MSB of the sampled data will be truncated. Samples become 16-bit data.
uint16_t irBuffer[100]; //infrared LED sensor data
uint16_t redBuffer[100];  //red LED sensor data
#else
uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
#endif

int32_t bufferLength; //data length
int32_t spo2; //spo2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid


// funcion que recoge los datos que se han de insertar en la base de datos 
String INSERT_SQL = String("INSERT INTO ") + default_database + "." + default_table 
                 + " (frecCard,oxSangre,id_P) VALUES (" + int(heartRate) + "," + int(spo2) + "," + id_P + ")";

MySQL_Connection conn((Client *)&client);

MySQL_Query *query_mem;

void setup()
{
  Serial.begin(115200);

  // se iniciaiza el sensor 
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }

  Serial.println(F("Attach sensor to finger with rubber band. Press any key to start conversion"));
  while (Serial.available() == 0) ; //wait until user presses a key
  Serial.read();
  // settings para configurar el sensor 
  byte ledBrightness = 60; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings


 
  
  while (!Serial && millis() < 5000); // espera al serial a que se conecte 

  MYSQL_DISPLAY1("\nStarting Basic_Insert_ESP on", ARDUINO_BOARD);
  MYSQL_DISPLAY(MYSQL_MARIADB_GENERIC_VERSION);

  // Begin WiFi section
  MYSQL_DISPLAY1("Connecting to", ssid);
  
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    MYSQL_DISPLAY0(".");
  }

  // print out info about the connection:
  MYSQL_DISPLAY1("Connected to network. My IP address is:", WiFi.localIP());

  MYSQL_DISPLAY3("Connecting to SQL Server @", server, ", Port =", server_port);
  MYSQL_DISPLAY5("User =", user, ", PW =", password, ", DB =", default_database);
}

void runInsert()
{
  // Iinicialica la clase query_mem
  MySQL_Query query_mem = MySQL_Query(&conn);

  if (conn.connected())
  {
    // inserta en la base de datos la funcion INSERT_SQL explicada anteriormente 
    MYSQL_DISPLAY(INSERT_SQL);
    
    // ejecuta la query
    // chequea si el fetching ha sido válido
    if ( !query_mem.execute(INSERT_SQL.c_str()) )
    {
      MYSQL_DISPLAY("Insert error");
    }
    else
    {
      MYSQL_DISPLAY("Data Inserted.");
    }
  }
  else
  {
    MYSQL_DISPLAY("Disconnected from Server. Can't insert.");
  }
}

void loop()
{
  bufferLength = 100; //almacena 100 muestras 

  //lee las 100 primeras señales y determina el rango de la señal 
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check(); //chequea si hay nueva data 

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample(); //We're finished with this sample so move to next sample

    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);
  }

  //calcula frecuencia cardiaca y la saturación de oxígeno en sangre tras las primeras 100 muestras 
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  //toma muestras del sensor continuamente y se realizan los calculos de frecuencia cardiaca y SO2 cada segundo
  while (1)
  {
    //los primeros 25  muestras se deshechan y se guardan los primeros 75 datos 
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    
    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor.available() == false) 
        particleSensor.check();


      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); 

      //manda las muestras y los resultados al serial
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

      // tras ser tomados todos los datos se empiezan a insertar
      id_P = 21;

      // se valida si las muestras fc y SO2 son válidas y cuando ambas son válidas se comienza a insertar
      if (validSPO2 ==1 && validHeartRate== 1){
          MYSQL_DISPLAY("Connecting...");

      //if (conn.connect(server, server_port, user, password))
      if (conn.connectNonBlocking(server, server_port, user, password) != RESULT_FAIL)
      {
        //Si la conexión es correcta 
        delay(500);
        //se ejecuta la funcion run insert 
        runInsert();
        conn.close();                     // close the connection
      }
      else
      {
        MYSQL_DISPLAY("\nConnect failed. Trying again on next iteration.");
      }

      MYSQL_DISPLAY("\nSleeping...");
      MYSQL_DISPLAY("================================================");
         delay(1000);
      }
    else{
       delay(50);
      }
      
     

    }

    //After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }
}
