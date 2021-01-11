#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <SimpleDHT.h>

#define WIFI_SSID "**********"
#define WIFI_PASSWORD "***************"

//URL de la BD (sin "https://" al principio ni "/" al final)
#define FIREBASE_HOST "domotica-*****.firebaseio.com"
//Clave secreta BD
#define FIREBASE_AUTH "***************"

//nodo de la BD que representa el sensor de temperatura
String sensorTemperatura = "/comun/sensorTemperatura";

//Objeto para el intercambio de datos con la BD
FirebaseData firebaseData;

int pinDHT11 = 12;
SimpleDHT11 dht11;
byte temperatura = 0;
byte humedad = 0;

unsigned int tiempoAnterior;
int intervaloEnviotemperatura = 60000;

void setup(){
  Serial.begin(115200);
  pinMode(pinDHT11, INPUT);

 //Inicializamos la conexión WiFI
  Serial.print("Conectando a " + String(WIFI_SSID) + " ");     
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Conectado");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

void loop(){
  if(millis() - tiempoAnterior > intervaloEnviotemperatura){
    dht11.read(pinDHT11, &temperatura, &humedad, NULL);
    if (!isnan(temperatura)){
      //Se modifica el valor del nodo habitacion/luz de la base de datos
      if (Firebase.setInt(firebaseData, sensorTemperatura, (int)temperatura))
        Serial.println("Temperatura actualizada a " + String(temperatura) + "ºC");
      else Serial.println("Error de escritura");
    }
    tiempoAnterior = millis();
  }
}
