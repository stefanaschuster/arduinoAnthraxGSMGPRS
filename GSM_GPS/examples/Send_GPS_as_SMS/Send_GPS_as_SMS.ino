#include <GSM.h>
#include <GPS.h>
#include <SPI.h>

GSM gsm(1);
GPS gps;

#define PHONE "012345678980"
#define SIM_PIN "8263"

byte init_error=0;
void setup(){
  Serial.begin(9600);
  Serial.println("init GSM starting...");
  if(gsm.initialize(SIM_PIN)){
      Serial.println("init GSM ok");
  }else{
      Serial.println("init GSM error!");
      init_error = 1;
      return;
  }

  Serial.println("init GPS starting...");
  if(gps.initialize()){
      Serial.println("init GPS ok");
  }else{
      Serial.println("init GPS error!");
      init_error = 1;
      return;
  }
  Serial.println("ready");
}


void loop() {
  
  gps.set_LED(GPS_LED_ON);
  
  if(init_error){
      Serial.println("no execution. Init was failing");
      delay(5000);
      return;
  }
    
  // Ist die Taste 1 gedrückt worden? Das ist die Taste am Ende der Platine
  if(gps.check_switch(GPS_SWITCH_1)){
     Serial.println("Switch pressed");
    
    nmae_gpgga_message gps_msg;
     // warten auf die GPS information
    gps.get_next_gps_position(&gps_msg);
     
     Serial.print("GPS Quality:");
     Serial.println(gps_msg.quality);
     // haben wir einen GPS fix?
     if(gps_msg.quality[0] == '0'){
       Serial.println("No GPS Fix yet");
     } else{
       // wir haben einen GPS Fix
       Serial.println("GPS:");
       char sms[240];
       construct_message(sms,gps_msg);
       Serial.println(sms);
       if(gsm.send_SMS(PHONE,sms)){
         Serial.println("Done sending SMS");
       } else {
         Serial.println("ERROR sending SMS");
       }
     }
  }  
  gps.set_LED(GPS_LED_OFF);
}

void construct_message(char * buffer, nmae_gpgga_message msg) {
  int pos = 0;
  
  strcpy(buffer+pos,msg.latitude);
  pos += strlen(msg.latitude);
  strcpy(buffer+pos,";");
  pos ++;

  strcpy(buffer+pos,msg.latitude_indicator); 
  strcpy(buffer+pos,";");
  pos ++;

  strcpy(buffer+pos,msg.longitude);
  pos += strlen(msg.longitude);
  strcpy(buffer+pos,";");
  pos ++;

  strcpy(buffer+pos,msg.longitude_indicator); 
}

