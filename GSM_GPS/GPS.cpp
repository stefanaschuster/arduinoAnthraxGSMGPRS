/*
 * Author: Stefan Schuster
 * Initialization code and constants for talking to the SCI16IS750 are taken from the original
 * library from antrax datentechnik gmbh (http://antrax.de)
 * 
 * File: GPS.cpp
 * Description: Libary for using the GSM and GPS-Modul on the GSM/GPRS/GPS-Shield Rev.7 from antrax
 * Libary is set to work with the arduino mega and debugs on Serial0.
 * This can be changed by changing defines in GPS.h
 * 
 * License: This source is licensed under the Apache License V2.
 * See http://www.apache.org/licenses/LICENSE-2.0.html
 *
 * Changelog
 * V0.1: Initial Version
 */

#ifndef TESTING
	#include <Arduino.h>
#else
	#define byte char
#endif
#include "GPS.h"
#include <SPI.h>


#define GPS_SPI_SELECT digitalWrite(SS, 0);     
#define GPS_SPI_RELEASE digitalWrite(SS, 1);    

// Adresses of the SCI16IS750 registers
#define RHR       0x00 << 3
#define FCR       0x02 << 3
#define LCR       0x03 << 3
#define LSR       0x05 << 3
#define SPR       0x07 << 3
#define IODIR     0x0A << 3
#define IOSTATE   0x0B << 3
#define IOCTL     0x0E << 3

// special registers
#define DLL       0x00 << 3  
#define DLM       0x01 << 3
#define EFR       0x02 << 3


char GPS::initialize() {
    char tmp = 0;
  
  // set pin's for SPI
  // we do not use SPI.begin()
  pinMode(SS,OUTPUT);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV64); // 16 MHz asssumed, this will result in 25khz Clock
  SPI.begin();
  
  GPS_SPI_RELEASE;
      
  write_SPI(LCR, 0x80);                                                // set Bit 7 so configure baudrate
  write_SPI(DLL, 0x18);                                                // 0x18 = 9600 with Xtal = 3.6864MHz
  write_SPI(DLM, 0x00);                                                // 0x00 = 9600 with Xtal = 3.6864MHz

  write_SPI(LCR, 0xBF);                                                // configure uart
  write_SPI(EFR, 0x10);                                                // activate enhanced registers
  write_SPI(LCR, 0x03);                                                // Uart: 8,1,0
  write_SPI(FCR, 0x06);                                                // Reset FIFO registers
  write_SPI(FCR, 0x01);                                                // enable FIFO Mode   

  // configure GPIO-Ports
  write_SPI(IOCTL, 0x01);                                              // set as GPIO's
  write_SPI(IODIR, 0x04);                                              // set the GPIO directions                                     
  write_SPI(IOSTATE, 0x00);                                            // set default GPIO state

  // Check functionality
  write_SPI(SPR, 'A');                                                 // write an a to register and read it
  return (read_SPI(SPR) == 'A');
}

// returns true when buffer is valid
// writes into gps_buffer;
bool GPS::read_gps_buffer(){
  // read from GPS sensor if data available
  if(read_SPI(LSR) & 0x01) {
	char tmp = read_SPI(RHR);
	
    if(buf_pos < 80){
      gps_buffer[buf_pos++] = tmp;
    }      
    if(buf_pos == 80) {
      // error - sentence to long!
      buf_pos = 0;
    }
    if(tmp == '\n'){
      gps_buffer[buf_pos] = 0;
      buf_pos = 0;
      return true;
	}
  }
  return false;
}

char * GPS::get_gps_buffer(){
	return gps_buffer;
}

bool GPS::check_switch(byte s){
  return !(read_SPI(IOSTATE) & s);                              // read S1 button state    
}

// use GPS_LED_ON / GPS_LED_OFF defines
void GPS::set_LED(char state){
  write_SPI(IOSTATE, state);                                          // turn on LED
}

void GPS::write_SPI(char adress, char data){
  GPS_SPI_SELECT;
  SPI.transfer(adress);                                                // write data
  SPI.transfer(data);                                                // write data
  GPS_SPI_RELEASE;                                                           
}

char GPS::read_SPI(char adress){
  char incomming_data;
  adress = (adress | 0x80);
  
  GPS_SPI_SELECT;
  SPI.transfer(adress);
  incomming_data = SPI.transfer(0xFF);
  GPS_SPI_RELEASE;

  return incomming_data;     
}


// Attention - changes argument string!
byte GPS::parse_nmea_gpgga_string(char * gps_string, nmae_gpgga_message  * gpgga_msg){
   // wir interessieren uns nur für GPGGA Sätze. Copmare der ersten zeichen wenn überhaupt genug da sind
  if((strlen(gps_string) >= strlen("$GPGGA")) && (strncmp("$GPGGA",gps_string,strlen("$GPGGA")) == 0)){
    // GPGGA Satz gefunden
    
	// zeiger auf rest des strings und aktuellen Teil
	char * gps_string_part = gps_string;
	// position im ursprungsstring
	int pos = 0;
	// ersten delimiter durch 0 ersetzen
	int initial_len = strlen(gps_string);
	while (pos < initial_len){
		if(gps_string[pos] == ','){
			gps_string[pos] = 0;
			break;
		}
		pos++;
	}
	// pos steht auf aktuellem delimiter
	// gps_string_part steht auf start
	// dlimiter ist durch 0 ersetzt
	
    int part =0; 
    while(gps_string_part != 0) {
      switch(part) {
        case 2:
          gpgga_msg->latitude = gps_string_part;
          break;
        case 3:
          gpgga_msg->latitude_indicator = gps_string_part;
          break;
        case 4:
          gpgga_msg->longitude = gps_string_part;
          break;
        case 5:
          gpgga_msg->longitude_indicator = gps_string_part;
          break;
        case 6:
          gpgga_msg->quality = gps_string_part;
          break;
      }
      part++;
	  
	  // pos steht auf aktuellem delimiter
	  // gps_string_part nächstem string teil
	  // dlimiter ist durch 0 ersetzt
	  
	  // sind wir am ende?
	  if(pos == initial_len){
		  // ende des strings
		  gps_string_part = 0;
		  break;
	  } else {
		  pos++;
		  gps_string_part = gps_string+pos;
		  // nicht am ende, weitersuchen
		  while (pos < initial_len){
			if(gps_string[pos] == ','){
				gps_string[pos] = 0;
			
				break;
			}
			pos++;
		  }
	  }        
	}
    return 1;
  } else {
   // kein GPGGA string
    return 0;
  } 
}

void  GPS::get_next_gps_position(nmae_gpgga_message  * gpgga_msg) {

	while(true) {		
		while(!read_gps_buffer()){ // wait for next line
			;
		}
		if(parse_nmea_gpgga_string(get_gps_buffer(),gpgga_msg)){
			// wir haben einen GPGGA string!
			return;
		}
	}
}