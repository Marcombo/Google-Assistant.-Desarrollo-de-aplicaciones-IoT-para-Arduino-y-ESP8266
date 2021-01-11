#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>

#define WIFI_SSID "**********"
#define WIFI_PASSWORD "***************"

//URL de la BD (sin "https://" al principio ni "/" al final)
#define FIREBASE_HOST "mi-primera-base-de-datos-*****.firebaseio.com"
//Clave secreta BD
#define FIREBASE_AUTH "***************"

//Objetos para el intercambio de datos con la BD
FirebaseData firebaseData1;
FirebaseData firebaseData2;

//nodo de la BD que representa el GPIO0 del ESP-01
String nodoD0 = "/ESP-01/D0";
//nodo de la BD que representa el GPIO2 del ESP-01
String nodoD2 = "/ESP-01/D2";

int pinLed = 0;
int pinPulsador = 2;
bool estadoDispositivo = false;
bool pulsadorLiberado = true;

void setup(){
  Serial.begin(115200);

  pinMode(pinLed, OUTPUT);
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
  if (!Firebase.beginStream(firebaseData1, nodoD0)){
    Serial.println("No se pudo establecer conexión con la base de datos");
    Serial.println("Motivo: " + firebaseData1.errorReason());
  }
}

void loop(){
  //Se ha presionado el pulsador
  if (digitalRead(pinPulsador)){
    if (pulsadorLiberado){
      pulsadorLiberado = false;
      estadoDispositivo = !estadoDispositivo;
      //Se modifica el valor del nodo D2 de la base de datos
      if (Firebase.setBool(firebaseData2, nodoD2, estadoDispositivo)){
        if (estadoDispositivo)Serial.println("Asignando nivel alto al nodo ESP-01/D2");
        else Serial.println("Asignando nivel bajo al nodo ESP-01/D2");
      }
      else Serial.println("No se ha podido asignar ningún valor al nodo ESP-01/D2");
      delay(150);
    }
  }
  else pulsadorLiberado = true;
}

//Función de callback que se ejecuta cuando se modifica el valor de algún nodo
void streamCallback(StreamData data){
  if (data.dataType() == "boolean"){
      //se lee el valor del nodo ESP-01/D0
      bool estadoLed = firebaseData1.boolData();
      if (estadoLed) Serial.println("Asignando nivel alto al GPIO0");
      else Serial.println("Asignando nivel bajo al GPIO0");
      digitalWrite(pinLed, estadoLed);
  }
}

//Error de desconexión con la base de datos
void streamTimeoutCallback(bool timeout){
  if(timeout) Serial.println("Error de conexión. Recuperándola..."); 
}
