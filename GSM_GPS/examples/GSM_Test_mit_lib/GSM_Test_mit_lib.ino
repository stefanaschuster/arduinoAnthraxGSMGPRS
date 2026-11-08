#include <GSM.h>
#include <SPI.h>

GSM gsm(1);
char buffer[250];
char tmp = 0;
char found_string = 0;
byte i;

void setup(){
  Serial.begin(9600);
  gsm.initialize("8263");
  Serial.println("ready");
}

void loop() {
  // Wir lesen immer eine Zeile aus dem Serial Interface ein und
  // geben den Zustand zurück. KEIN produktiver code, kein check auf 
  // überlauf etc!
  // einlesen des Puffers
  if(Serial.available()) {
    if((tmp = Serial.read()) != '\n') {
      if(tmp == '$'){
        // subsituiere $ zu CTRL+Z
        buffer[i++] = 26;
      } else if(tmp == '%'){
        // subsituiere $ zu CTRL+Z
        buffer[i++] = '\r';
        buffer[i++] = '\n';
      } else {
        buffer[i++] = tmp;
      }
    } else {
      buffer[i] = 0;
      i = 0;
      found_string = 1;
    }
  }
  
  // String komplett, an GSM senden
  if(found_string) {
    // echo
    gsm.send_command(buffer);
    // Gleichen String nicht nochmal übergeben
    found_string = 0;
  }
  
  // GSM prüfen ob AW kommt und dann auf Serial schreiben
  if(gsm.read_gsm_buffer()){
    Serial.write(gsm.get_gsm_buffer());
  }
}

