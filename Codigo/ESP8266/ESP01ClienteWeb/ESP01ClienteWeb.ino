#include <ESP8266WiFi.h>                        

// SSID de la red WIFI a la que desea conectarnos
const char* ssid = "*********";                  
//contraseña de dicha red 
const char* password = "******************";        

const char* servidorWeb = "www.google.com";            
 
void setup()
{
  Serial.begin(115200);

  //Inicializa la conexión WIFI
  Serial.print("Conectando a " + String(ssid) + " ");     
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Conectado");
}
 
void loop()
{
  WiFiClient clienteWeb;                                 

  //Establece la conexión con el servidor  
  if (clienteWeb.connect(servidorWeb, 80))
  {
    //Realiza la petición de la página HTML
    clienteWeb.print(String("GET /") + " HTTP/1.1\r\n" +
                 "Host: " + servidorWeb + "\r\n" +
                 "Connection: close\r\n" + "\n");
 
    //Lee la respuesta del servidor linea a linea
    while (clienteWeb.connected())
    {
      if (clienteWeb.available())
      {
        String linea = clienteWeb.readStringUntil('\n');
        Serial.println(linea);
      }
    }
    //Finaliza la conexión con el servidor
    clienteWeb.stop();                                  
  }
  else Serial.println("Problemas de conexión con el servidor");
  
  delay(10000);
}
