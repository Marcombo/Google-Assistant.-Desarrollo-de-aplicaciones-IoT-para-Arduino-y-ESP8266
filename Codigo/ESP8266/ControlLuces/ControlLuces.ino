#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>

#define WIFI_SSID "**********"
#define WIFI_PASSWORD "***************"

//URL de la BD (sin "https://" al principio ni "/" al final)
#define FIREBASE_HOST "domotica-*****.firebaseio.com"
//Clave secreta BD
#define FIREBASE_AUTH "***************"

//nodo de la BD que representa el GPIO0 del ESP-01
//Descomentar la constante donde se encuentre la luz
//String habitacion = "salon";
String habitacion = "dormitorio";

//Objetos para el intercambio de datos con la BD
FirebaseData firebaseData1;
FirebaseData firebaseData2;

int pinRele = 13;
int pinPulsador = 12;
bool estadoRele = false;

void setup(){
  Serial.begin(115200);

  pinMode(pinRele, OUTPUT);
  pinMode(pinPulsador, INPUT);

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

  Firebase.setStreamCallback(firebaseData1, streamCallback, streamTimeoutCallback);
  if (!Firebase.beginStream(firebaseData1, habitacion + "/luz")){
    Serial.println("No se pudo establecer conexión con la base de datos");
    Serial.println("Motivo: " + firebaseData1.errorReason());
  }
  
  delay(500);
  //Se modifica el valor del nodo habitacion/luz a false
  if(Firebase.setBool(firebaseData2, habitacion + "/luz", false))
    Serial.println("Asignado el valor false al nodo " + habitacion + "/luz");
  else Serial.println("Error de escritura");;
}

void loop(){
  //Se ha presionado el pulsador
  if (digitalRead(pinPulsador)){
    estadoRele = !estadoRele;
    digitalWrite(pinRele, estadoRele);

    //Se modifica el valor del nodo habitacion/luz de la base de datos
    if (Firebase.setBool(firebaseData2, habitacion + "/luz", estadoRele))
      Serial.println("Asignado el valor " + String(estadoRele) + " al nodo " + habitacion + "/luz");
    else Serial.println("Error de escritura");

    delay(150);
  }
}

//Función de callback que se ejecuta cuando se modifica el valor de algún nodo
void streamCallback(StreamData data){
  if (data.dataType() == "boolean"){
      //se lee el valor del nodo habitacion/luz
      if (data.boolData()) Serial.println("Enciendo la luz");
      else Serial.println("Apago la luz");
      digitalWrite(pinRele, firebaseData1.boolData());
  }
}

//Error de desconexión con la base de datos
void streamTimeoutCallback(bool timeout){
  if(timeout) Serial.println("Error de conexión. Recuperándola..."); 
}
