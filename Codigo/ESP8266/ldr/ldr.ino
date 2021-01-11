#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

SoftwareSerial puertoESP01(0, 2); // RX | TX

#define WIFI_SSID "**********"
#define WIFI_PASSWORD "***************"

//URL de la BD (sin "https://" al principio ni "/" al final)
#define FIREBASE_HOST "mi-primera-base-de-datos-*****.firebaseio.com"
//Clave secreta BD
#define FIREBASE_AUTH "***************"

//Objetos para el intercambio de datos con la BD
FirebaseData firebaseData1;

//nodo de la BD que representa el pin A0 de Arduino
String nodoA0 = "/Arduino/A0";

void setup(){
  Serial.begin(115200);
  puertoESP01.begin(9600);

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
   if (puertoESP01.available()){
    int nivelLuz = (int)puertoESP01.read();
    //Se modifica el valor del nodo D2 de la base de datos
    if (Firebase.setInt(firebaseData1, nodoA0, nivelLuz)){
      Serial.println("Asignando al nodo Arduino/A0 el valor: " + String(nivelLuz));
    }
    else{
      Serial.println("No se ha podido asignar ningún valor al nodo Arduino/A0");
    }
  }
}
