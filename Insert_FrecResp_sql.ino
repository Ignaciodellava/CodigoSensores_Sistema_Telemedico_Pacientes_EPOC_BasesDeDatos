/*********************************************************************************************************************************
  Basic_Insert_ESP.ino
      
  Library for communicating with a MySQL or MariaDB Server
  
  Based on and modified from Dr. Charles A. Bell's MySQL_Connector_Arduino Library https://github.com/ChuckBell/MySQL_Connector_Arduino
  to support nRF52, SAMD21/SAMD51, SAM DUE, STM32F/L/H/G/WB/MP1, ESP8266, ESP32, etc. boards using W5x00, ENC28J60, LAM8742A Ethernet,
  WiFiNINA, ESP-AT, built-in ESP8266/ESP32 WiFi.

  The library provides simple and easy Client interface to MySQL or MariaDB Server.
  
  Built by Khoi Hoang https://github.com/khoih-prog/MySQL_MariaDB_Generic
  Licensed under MIT license
 **********************************************************************************************************************************/
/*
  MySQL Connector/Arduino Example : connect by wifi

  This example demonstrates how to connect to a MySQL server from an
  Arduino using an Arduino-compatible Wifi shield. Note that "compatible"
  means it must conform to the Ethernet class library or be a derivative
  with the same classes and methods.
  
  For more information and documentation, visit the wiki:
  https://github.com/ChuckBell/MySQL_Connector_Arduino/wiki.

  INSTRUCTIONS FOR USE

  1) Change the address of the server to the IP address of the MySQL server
  2) Change the user and password to a valid MySQL user and password
  3) Change the SSID and pass to match your WiFi network
  4) Connect a USB cable to your Arduino
  5) Select the correct board and port
  6) Compile and upload the sketch to your Arduino
  7) Once uploaded, open Serial Monitor (use 115200 speed) and observe

  If you do not see messages indicating you have a connection, refer to the
  manual for troubleshooting tips. The most common issues are the server is
  not accessible from the network or the user name and password is incorrect.

  Created by: Dr. Charles A. Bell
*/

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
  IPAddress server(195, 235, 211, 197);
#endif

uint16_t server_port = 3306;    //3306;

char default_database[] = "prbSafeBreath";           //"test_arduino";
char default_table[]    = "FrecResp";          //"test_arduino";


//#define mic 13

int respiraciones=12;
int id_P=0;

// Sample query
//String INSERT_SQL = String("INSERT INTO ") + default_database + "." + default_table 
//                 + " (frecuResp,id_P) VALUES (" + int(respiraciones) + "," + id_P + ")";     //Insertamos en la base de datos la variable respiraciones, 
                                                                                               //el id paciente y el id sensor es autoincrement se pone solo y el timestamp también se pone solo


MySQL_Connection conn((Client *)&client);

MySQL_Query *query_mem;



void setup()
{
  Serial.begin(115200);
  while (!Serial);

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

String INSERT_SQL = String("INSERT INTO ") + default_database + "." + default_table 
                 + " (frecuResp,id_P) VALUES (" + respiraciones + "," + id_P + ")";
  
  // Initiate the query class instance
  MySQL_Query query_mem = MySQL_Query(&conn);

  if (conn.connected())
  {
    MYSQL_DISPLAY(INSERT_SQL);
    
    // Execute the query
    // KH, check if valid before fetching
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

  MYSQL_DISPLAY("Connecting...");
  
  //if (conn.connect(server, server_port, user, password))
  if (conn.connectNonBlocking(server, server_port, user, password) != RESULT_FAIL)
  {
    delay(500);

int sensorValue = analogRead(13);           //Le damos a la variable sensorValue el valor que lee la funcion analogRead en el pin 13 que es la entrada del sensor de sonido, 
                                            //que depende de la intensidad del sonido devuelve un valor de entre 0 y 1023 bits.
if (sensorValue > 120) {                    //Creamos un código que le da a otra variable (respiraciones) el valor de 1 si el sonido es muy alto, 
                                            //lectura por encima de 120 bits y si no un 0 si está por debajo.
  respiraciones = 1;
  } else {
   respiraciones = 0;
 }
 Serial.println(sensorValue);  //Este código es de control
  
 id_P = 21;     //introducimos el id del paciente deseado
     delay(200);

      runInsert(); //Insertamos el valor de la variable respiraciones en ese momento, 
                    //de tal forma que a la hora de graficarlo tengamos la grafica de frecuencia respiratoria con unos y ceros.
    conn.close();                     // close the connection
  } 
  else 
  {
    MYSQL_DISPLAY("\nConnect failed. Trying again on next iteration.");
  }

  MYSQL_DISPLAY("\nSleeping...");
  MYSQL_DISPLAY("================================================");
 
  delay(200);
}
