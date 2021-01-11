#include <SoftwareSerial.h>

SoftwareSerial puertoESP01(12, 13); // RX | TX

int pinFotocelula = 0;
int nivelLuz;
 
void setup() {
   Serial.begin(115200);
   puertoESP01.begin(9600);
}
 
void loop() {
  nivelLuz = map(analogRead(pinFotocelula), 0, 1023, 0, 100);
  Serial.println(nivelLuz);
  puertoESP01.write(nivelLuz);
  delay(2000);
}

