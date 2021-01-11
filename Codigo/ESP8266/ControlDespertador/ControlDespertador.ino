#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include "DS3231.h"
#include <LiquidCrystal_I2C.h>

#define WIFI_SSID "**********"
#define WIFI_PASSWORD "***************"

//URL de la BD (sin "https://" al principio ni "/" al final)
#define FIREBASE_HOST "domotica-*****.firebaseio.com"
//Clave secreta BD
#define FIREBASE_AUTH "***************"

//URL de la función cloud que devuelve hora actual
#define URL_FUNCION_CLOUD "http://us-central1-domotica-*****.cloudfunctions.net/obtenerHoraActual"

//nodo de la BD que representa el estado de la alarma 
String nodoEstadoAlarma = "/dormitorio/despertador/estado";
//nodo de la BD que representa la hora programada
String nodoHoraProgramada = "/dormitorio/despertador/horaProgramada";

//Objeto para el intercambio de datos con la BD
FirebaseData firebaseData1, firebaseData2;
FirebaseData firebaseData3;

HTTPClient http;

//Se crea el objeto lcd en la dirección I2C 0x27 para una pantalla de 2 filas de 16 caracteres
LiquidCrystal_I2C lcd(0x27, 16, 2);

DS3231 reloj;
bool h12 = false; // modo 24h
bool AM;

String horaActual;
String minutoActual, minutoAnterior;

bool estadoAlarma;
String horaProgramada;
String horaParada; //un minuto posterior a la programada

int pinPulsador = 13;
int pinBuzzer = 12;

volatile bool pulsadorPresionado = false;
volatile bool alarmaSonando = false;

//Función que atiende la interrupción generada por el pulsador
void ICACHE_RAM_ATTR atenderInterrupcion() {
  if(alarmaSonando) pulsadorPresionado = true;
}

void setup(){
  Serial.begin(115200);
  
  pinMode(pinPulsador, INPUT);
  pinMode(pinBuzzer, OUTPUT);
  
  Wire.begin();

  //Se inicia el LCD y se enciende la luz de fondo
  lcd.init();
  lcd.backlight();

 //Inicializamos la conexión WiFI
  Serial.print("Conectando a " + String(WIFI_SSID) + " ");     
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Conectado");

  String fecha = obtenerHoraActual();
  String hora = fecha.substring(0,fecha.indexOf(':'));
  String minuto = fecha.substring(fecha.indexOf(':')+1,fecha.length());
  reloj.setClockMode(h12);
  reloj.setHour(byte(hora.toInt()));
  reloj.setMinute(byte(minuto.toInt()));

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  Firebase.setStreamCallback(firebaseData1, callbackEstadoAlarma, streamTimeoutCallback);
  if (!Firebase.beginStream(firebaseData1, nodoEstadoAlarma)){
    Serial.println("No se pudo establecer conexión con la base de datos");
    Serial.println("Motivo: " + firebaseData1.errorReason());
  }
  Firebase.setStreamCallback(firebaseData2, callbackHoraProgramada, streamTimeoutCallback);
  if (!Firebase.beginStream(firebaseData2, nodoHoraProgramada)){
    Serial.println("No se pudo establecer conexión con la base de datos");
    Serial.println("Motivo: " + firebaseData2.errorReason());
  }

  //Asocia la función que atenderá la interrupción generada al presionar el pulsador
  attachInterrupt(digitalPinToInterrupt(pinPulsador), atenderInterrupcion, RISING);
}

void loop() {
  horaActual = reloj.getHour(h12, AM);
  minutoActual = reloj.getMinute();
  if(horaActual.toInt() < 10) horaActual = "0" + horaActual;
  if(minutoActual.toInt() < 10) minutoActual = "0" + minutoActual;
  if(minutoAnterior != minutoActual){
    lcd.clear();
    lcd.print(horaActual + ":" + minutoActual);
    if(estadoAlarma){
      lcd.setCursor(6, 0);
      lcd.print("*");
    }
    minutoAnterior = minutoActual;
  }

  if(compararHoras(horaActual + ":" + minutoActual, horaProgramada) &&
     !compararHoras(horaActual + ":" + minutoActual, horaParada) &&
     estadoAlarma){
    if(pulsadorPresionado) alarmaSonando = false;
    else{
      alarmaSonando = true;
      beep(); beep(); delay(500);   
    }
  }
  else pulsadorPresionado = false;
}

/*****************************************
          FUNCIONES DE CALLBACK 
 ****************************************/
void callbackEstadoAlarma(StreamData data){
  if (data.dataType() == "boolean"){
    estadoAlarma = data.boolData();
    if (estadoAlarma){
      Serial.println("Activo la alarma");
      lcd.setCursor(6, 0);
      lcd.print("*");
    }
    else{
      Serial.println("Desactivo la alarma");
      lcd.setCursor(6, 0);
      lcd.print(" ");
    }
  }
}

void callbackHoraProgramada(StreamData data){
  if (data.dataType() == "string"){
    horaProgramada = data.stringData();
    horaParada = incrementarMinuto(horaProgramada);
    Serial.println("Programo la alarma a las " + horaProgramada);
    Serial.println("Paro el sonido de la alarma a las " + horaParada);
  }
}

//Error de desconexión con la base de datos
void streamTimeoutCallback(bool timeout){
  if(timeout) Serial.println("Error de conexión. Recuperándola..."); 
}

/*****************************************
          FUNCINES AUXILIARES
 ****************************************/
//obtiene la fecha de los servidores de Firebase Cloud Functions
String obtenerHoraActual(){
  http.begin(URL_FUNCION_CLOUD);  
  http.addHeader("Content-Type", "application/json");
  http.POST("{}");
  String payload = http.getString();
  http.end();
  return payload;
}

//devuelve true si tiempo1 >= tiempo2
//formato de la hora --> hh:mm
boolean compararHoras(String tiempo1, String tiempo2){
  int hora1 = tiempo1.substring(0,tiempo1.indexOf(':')).toInt();
  int minuto1 = tiempo1.substring(tiempo1.indexOf(':')+1,tiempo1.length()).toInt();
  int hora2 = tiempo2.substring(0,tiempo2.indexOf(':')).toInt();
  int minuto2 = tiempo2.substring(tiempo2.indexOf(':')+1,tiempo2.length()).toInt();

  if(hora1 > hora2) return true;
  if(hora1 < hora2) return false;
  if(minuto1 >= minuto2) return true;
}

//Añade un minuto a una hora en formato hh:ss
String incrementarMinuto(String hora){
  int horaFin = hora.substring(0,hora.indexOf(':')).toInt();
  int minutoFin = hora.substring(hora.indexOf(':')+1,hora.length()).toInt() + 1;
  if (minutoFin == 60){
    minutoFin = 0;
    horaFin += 1;
  }
  if (horaFin == 24) horaFin = 0;
  return String(horaFin) + ":" + String(minutoFin);
}

//hace sonar un beep en el buzzer
void beep(){
  analogWrite(pinBuzzer, 20);
  delay(100);
  analogWrite(pinBuzzer, 0); 
  delay(100); 
}

