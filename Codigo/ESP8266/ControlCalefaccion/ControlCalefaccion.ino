#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <SimpleDHT.h>

#define WIFI_SSID "**********"
#define WIFI_PASSWORD "***************"

//URL de la BD (sin "https://" al principio ni "/" al final)
#define FIREBASE_HOST "domotica-*****.firebaseio.com"
//Clave secreta BD
#define FIREBASE_AUTH "***************"

//nodo de la BD que representa el estado de la calefacción
String nodoEstadoCalefaccion = "/comun/calefaccion/estado";
//nodo de la BD que representa la hora programada
String nodoTemperaturaProgramada = "/comun/calefaccion/temperaturaProgramada";
//nodo de la BD que representa el sensor de temperatura
String nodoSensorTemperatura = "/comun/sensorTemperatura";

//Objeto para el intercambio de datos con la BD
FirebaseData firebaseData1, firebaseData2;
FirebaseData firebaseData3;

int pinDHT11 = 12;
SimpleDHT11 dht11;
byte temperatura = 0;
byte humedad = 0;

int pinRele = 13;

int intervaloEnviotemperatura = 60000;
unsigned int tiempoAnterior;

boolean estadoCalefaccion = false;
boolean estadoFuncionamiento;
int temperaturaProgramada;
int temperaturaActual;

void setup(){
  Serial.begin(115200);
  
  pinMode(pinDHT11, INPUT);
  pinMode(pinRele, OUTPUT);

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

  Firebase.setStreamCallback(firebaseData1, callbackEstadoCalefaccion, streamTimeoutCallback);
  if (!Firebase.beginStream(firebaseData1, nodoEstadoCalefaccion)){
    Serial.println("No se pudo establecer conexión con la base de datos");
    Serial.println("Motivo: " + firebaseData1.errorReason());
  }
  Firebase.setStreamCallback(firebaseData2, callbackTemperaturaProgramada, streamTimeoutCallback);
  if (!Firebase.beginStream(firebaseData2, nodoTemperaturaProgramada)){
    Serial.println("No se pudo establecer conexión con la base de datos");
    Serial.println("Motivo: " + firebaseData2.errorReason());
  }
}

void loop() {
  if(millis() - tiempoAnterior > intervaloEnviotemperatura){
    dht11.read(pinDHT11, &temperatura, &humedad, NULL);
    if (!isnan(temperatura)){
      temperaturaActual = (int)temperatura;
      Serial.println("Temperatura actual: " + String(temperaturaActual));
      //Se modifica el valor del nodo comun/sensorTemperatura la base de datos
      Firebase.setInt(firebaseData3, nodoSensorTemperatura, temperaturaActual);
    tiempoAnterior = millis();
    }
  }
  
  if(estadoCalefaccion){
    if(temperaturaActual < temperaturaProgramada){
      if(!estadoFuncionamiento){
        Serial.println("Pongo en funcionamiento la calefacción");
        digitalWrite(pinRele, HIGH);
        estadoFuncionamiento = true;
      }
    }
    else if(estadoFuncionamiento){
      Serial.println("Dejo de hacer funcionar la calefacción");
      digitalWrite(pinRele, LOW);
      estadoFuncionamiento = false;
    }
  }
}

void callbackEstadoCalefaccion(StreamData data){
  if (data.dataType() == "boolean"){
    estadoCalefaccion = data.boolData();
    if (estadoCalefaccion) Serial.println("Activo la calefacción");
    else Serial.println("Desactivo la calefacción");
    //solo se fuerza el apagado (funcionamiento)
    //La puesta en funcionamiento es por temperatura
    if(!estadoCalefaccion){
      digitalWrite(pinRele, false);
      estadoFuncionamiento= false;
    }
  }
}

void callbackTemperaturaProgramada(StreamData data){
  if (data.dataType() == "int"){
    temperaturaProgramada = data.intData();
    Serial.println("Temperatura programada: " + String(temperaturaProgramada));
  }
}

//Error de desconexión con la base de datos
void streamTimeoutCallback(bool timeout){
  if(timeout) Serial.println("Error de conexión. Recuperándola..."); 
}
