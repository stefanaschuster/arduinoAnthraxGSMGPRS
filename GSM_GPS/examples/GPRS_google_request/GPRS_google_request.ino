#include <GSM.h>
#include <SPI.h>

GSM gsm(1);
#define SIM_PIN "8263"

byte init_error=0;
void setup(){
  
  Serial.begin(9600);
  Serial.println("init GSM starting...");
  if(gsm.initialize(SIM_PIN)){
      Serial.println("init GSM ok, setting up GPRS.");
  }else{
      Serial.println("init GSM error!");
      init_error = 1;
      return;
  }  
  if(!init_error) {
    // EIGENEN APN USER KENNWORT EINTRAGEN
    if(gsm.setup_gprs("internet.eplus.de","eplus","eplus")){
      Serial.println("init GPRS ok.");
     } else {
       Serial.println("init GPRS error!");
       init_error =1;
     }
  }

  if(init_error){
    Serial.println("no sending of data. Init was failing");
    return;
  }

  Serial.println("sending HTTP request to google");
  gsm.send_data_via_gprs("www.google.de", "80", "GET /index.html HTTP/1.1\r\nHost: www.google.de\r\n\r\n");
}

void loop() {
  // direkter Zugriff, da wir keine Interpretation des Puffers wollen
  if(GSM_SERIAL.available()){
    Serial.write(GSM_SERIAL.read());
  }
}

