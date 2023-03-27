MYSQL_DISPLAY1("\nStarting Basic_Insert_ESP on", ARDUINO_BOARD);
  MYSQL_DISPLAY(MYSQL_MARIADB_GENERIC_VERSION);

  // Iniciar la conexión a la Wifi
  MYSQL_DISPLAY1("Connecting to", ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    MYSQL_DISPLAY0(".");
  }

  // Imprimir la información de la conexión:
  MYSQL_DISPLAY1("Connected to network. My IP address is:", WiFi.localIP());

  MYSQL_DISPLAY3("Connecting to SQL Server @", server, ", Port =", server_port);
  MYSQL_DISPLAY5("User =", user, ", PW =", password, ", DB =", default_database);
}

void runInsert()
{
// recoge la informacion y el lugar donde se desea insertar la base de datos 
  String INSERT_SQL = String("INSERT INTO ") + default_database + "." + default_table
                    + " (temperatura,id_P) VALUES (" + temperatura + "," + id_P + ")";
                    
  // Iniciar la consulta de la clase de instancia
  MySQL_Query query_mem = MySQL_Query(&conn);

  if (conn.connected())
  {
    MYSQL_DISPLAY(INSERT_SQL);

    // Ejecutar la consulta
    // KH, comprobar si es válido antes de buscarlo
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
   //sensor temp
  int value = analogRead(sensorPin);
  int millivolts = (value / 1023.0) * 5000;
  temperatura = (millivolts / 10)-50;
  Serial.print(temperatura);
  Serial.println(" C");
  
  

  id_P = 21;
  MYSQL_DISPLAY("Connecting...");

  delay(2000);

  //if (conn.connect(server, server_port, user, password))
  if (conn.connectNonBlocking(server, server_port, user, password) != RESULT_FAIL)
  {
    delay(500);
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
