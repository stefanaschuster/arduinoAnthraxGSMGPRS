#include <GPS.h>
#include <SPI.h>
 
GPS gps;
void setup(){
 
  Serial.begin(9600);
  if(gps.initialize()){
    Serial.println("Init ok");
  } else {
    Serial.println("Init failure");
  }
}
 
void loop(){
    // SPI zeichen lesen und prüfen ob damit der String komplett ist
  if(gps.read_gps_buffer()){
    // kompletter string ist vorhanden
    Serial.print(gps.get_gps_buffer());  
  }
}

