
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
    nmae_gpgga_message gpgga_msg;

    char * gps_string = gps.get_gps_buffer();

    // weil die parse-funktion den string ändert, kopieren wir ihn erst
    char buffer[strlen(gps_string)+1]; // trailing 0
    strcpy(buffer,gps_string); // strncopy für produktiven code!   

    // wenn der string eine gpgga nachricht ist, bekommen wir die teile hiermmit und 1 als returncode.
    if(gps.parse_nmea_gpgga_string(buffer,&gpgga_msg)){
      Serial.println(gpgga_msg.quality);
      Serial.print(gpgga_msg.longitude);
      Serial.println(gpgga_msg.longitude_indicator);
      Serial.print(gpgga_msg.latitude);
      Serial.println(gpgga_msg.latitude_indicator);
    }
  }
}
