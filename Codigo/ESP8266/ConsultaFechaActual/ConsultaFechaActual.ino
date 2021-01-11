#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define WIFI_SSID "**********"
#define WIFI_PASSWORD "***************"

//URL de la función cloud que devuelve hora actual
#define URL_FUNCION_CLOUD "http://us-central1-mi-primera-funcion-*****.cloudfunctions.net/obtenerFechaHoraActual"

HTTPClient http;

String fechaHora;

void setup(){
  Serial.begin(115200);

 //Inicializamos la conexión WiFI
  Serial.print("Conectando a " + String(WIFI_SSID) + " ");     
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Conectado");
}

void loop(){
  http.begin(URL_FUNCION_CLOUD);  
  http.addHeader("Content-Type", "application/json");
  http.POST("{}");
  fechaHora = http.getString();
  http.end();
  Serial.println(fechaHora);
  delay(60000);
}
