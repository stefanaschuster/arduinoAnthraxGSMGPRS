/*
 * Author: Stefan Schuster
 * 
 * File: GSM.h
 * Description: Libary for using the GSM and GPS-Modul on the GSM/GPRS/GPS-Shield Rev.7 from antrax
 * Libary is set to work with the arduino mega and debugs on Serial0.
 * This can be changed by changing defines in GPS.h
 * 
 * License: This source is licensed under the Apache License V2.
 * See http://www.apache.org/licenses/LICENSE-2.0.html
 *
 * Changelog
 * V0.4: Refactored to use a execute method for statemachines
 * V0.3: Added GPRS Support
 * V0.1: Initial Version
 */
#ifndef GSM_h
#define GSM_h
#include <Arduino.h>
#include "State_Machine.h"

// which serial port to use? mega supports Serial, Serial1 ... Serial4
#define GSM_SERIAL Serial1

// undefine to avoid debugging
#define GSM_DEBUG_SERIAL Serial // where to put debug messages? needs to be setup/started by the main programm itself!

#ifdef GSM_DEBUG_SERIAL
	#define DEBUG_MSG(text)  {if(_debug != 0){GSM_DEBUG_SERIAL.println(text);}}
#else
	#define DEBUG_MSG(text) ;
#endif

#define GSM_SERIAL_BAUD 9600

// pin settings
#define GSM_POWER_PIN 7
#define MAX_RETRY_AT_COMMAND 6

class GSM
  {     
    public:
      GSM();
	  GSM(byte debug);
	  // blockierend. kann 5-8 Sekunden dauern.
      bool initialize(char* pwd);
      bool read_gsm_buffer();
	  char read_raw_gsm_answer();
	  char * get_gsm_buffer();
	  void send_command(char * command);
	  // blockierend. kehrt nach erfolg oder 3 Fehlversuchen zurück
	  bool send_SMS(char * phone_number, char * message);
	  // blockierend. kehrt nach antwort von GSM zurück.
      char get_gsm_state(void);
	  // Bereitet eine Datenübertragung mit GPRS vor. 
      // Zugangsdaten des Providers eintragen.
      bool setup_gprs(char * apn, char * user, char * pwd);
	  bool send_data_via_gprs(char * server, char * port, char * message);
private:
	  bool execute_statemachine(State_Machine * sm, char ** commands,byte end_state, int retry_time, bool gsm_special_mode);
      byte _debug;
	  char gsm_buffer[100];
	  byte buffer_pos;
	  void clear_uart(int timeout);
};
#endif