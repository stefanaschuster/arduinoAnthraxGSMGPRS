/*
 * Author: Stefan Schuster
 * Initialization code and constants for talking to the SCI16IS750 are taken from the original
 * library from antrax datentechnik gmbh (http://antrax.de)
 * 
 * File: GPS.h
 * Description: Settings and definitions für the GPS library
 * 
 * License: This source is licensed under the Apache License V2.
 * See http://www.apache.org/licenses/LICENSE-2.0.html
 *
 * Changelog
 * V0.1: Initial Version
 */
typedef struct {
  char  * quality;
  char  * longitude;
  char  * longitude_indicator;
  char  * latitude;
  char  * latitude_indicator;
} nmae_gpgga_message;


#ifndef TESTING
	#include <Arduino.h>
	#include <SPI.h>
#endif

#ifndef GPS_Shield_h
#define GPS_Shield_h

#define GPS_LED_ON		0x07
#define GPS_LED_OFF		0x00

#define GPS_SWITCH_0	0x00
#define GPS_SWITCH_1	0x01
  
class GPS {     
    public:
	  // returns true if buffer is valid (once)
	  bool read_gps_buffer();
	  char * get_gps_buffer();
	  // attention. changes gps_string !
      byte parse_nmea_gpgga_string(char * gps_string, nmae_gpgga_message  * gpgga_msg);
	  // blockierend. kehrt zurück wenn ein ganzer GPGGA String angekommen ist und füllt die info in die Struktur
	  void get_next_gps_position(nmae_gpgga_message  * gpgga_msg);
      char initialize();
      bool check_switch(byte s);  // use GPS_SWITCH_0, GPS_SWITCH_1
      void set_LED(char state); // use GPS_LED_ON, GPS_LED_OFF
  private:
      void write_SPI(char adress, char data);
      char read_SPI(char adress);
	  char gps_buffer[80];
	  byte buf_pos;
};
#endif