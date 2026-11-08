#include <State_Machine.h>
#include <SPI.h> // notwendig wegen Arudino Build und einbettung in GSM/GPS lib

State_Machine sm;
byte i = 0;
byte found_string = 0;

void setup(){
  // wenn im Zustand 1, und eine Antwort kommt, die mit OK beginnt,
  // dann gehe in Zustand 2 über. Verwende keine spezielle Funktion 
  // um den OK String zu erkennen.
  sm.add_transition(1,2,"OK",0);
  // 2 zu 3 bei ATE OK
  sm.add_transition(2,3,"ATE OK",0);

  // bei Zustand 2 oder 3 und Antwort, die mit error beginnt, gehe zu 1 zurück
  sm.add_transition(2,1,"ERROR",0);
  sm.add_transition(3,1,"ERROR",0);
  
  // Startzustand setzen
  sm.set_state(1);
  
  Serial.begin(9600);
}

void loop() {
  // Wir lesen immer eine Zeile aus dem Serial Interface ein und
  // geben den Zustand zurück. KEIN produktiver code, kein check auf 
  // überlauf etc!
  char buffer[100];
  char tmp = 0;
 
  // einlesen des Puffers
  if(Serial.available()) {
    if((tmp = Serial.read()) != '\n') {
      buffer[i++] = tmp;
    } else {
      buffer[i] = 0;
      i = 0;
      found_string = 1;
    }
  }
  
  // String komplett. An Zustandsautomat geben und schauen ob ein Übergang stattfindet
  // dann wird der neue Zustand zurückgegeben. Kein match = 0 als Rückgabe
  if(found_string) {
    // echo
    Serial.println(buffer);
    byte match = sm.match_line(buffer);
    if(match){
      Serial.print("found match, going to state ");
      Serial.println(sm.get_state());
    } else {
      Serial.println("String not matched");
    }
    // Gleichen String nicht nochmal übergeben
    found_string = 0;
  }
}