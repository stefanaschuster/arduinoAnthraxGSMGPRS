/*
 * Author: Stefan Schuster
 * 
 * File: GSM.cpp
 * Description: Libary for using the GSM and GPS-Modul on the GSM/GPRS/GPS-Shield Rev.7 from antrax
 * Libary is set to work with the arduino mega and debugs on Serial0.
 * This can be changed by changing defines in GPS.h
 * 
 * License: This source is licensed under the Apache License V2.
 * See http://www.apache.org/licenses/LICENSE-2.0.html
 *
 * Changelog
 * V0.6: fixed overflow in gsm buffer. sending debug message in error case.
 * V0.5: Implemented send_gprs_data methode
 * V0.4: Refactored to use a execute method for statemachines
 * V0.3: Added GPRS Support
 * V0.1: Initial Version
 */

#include <Arduino.h>
#include "GSM.h"

#define GSM_COMMAND(command) GSM_SERIAL.print(command);
                                                        // Arduino Duemilanove (ATmega328) & Arduino UNO (ATmega328)
GSM::GSM()                             
{         
 _debug = 0;     
 pinMode(GSM_POWER_PIN, OUTPUT);                                                 
}
GSM::GSM(byte dbg)                                
{              
 _debug = dbg;
 pinMode(GSM_POWER_PIN, OUTPUT);                                                 
}

bool GSM::execute_statemachine(State_Machine * sm, char * commands[], byte end_state,int retry_time, bool gsm_special_mode) {
  byte last_state = 1;
  byte command_sent = 0;
  int  retries =0;
  byte abort = 0;
    
  while((sm->get_state() < end_state) && (!abort)) {
	
	if(command_sent == 0){
		delay(100); // wait 100 ms between commands
		char * command = commands[sm->get_state()];
		DEBUG_MSG("STATE:")
	    DEBUG_MSG(sm->get_state())
		DEBUG_MSG("Command:")
		DEBUG_MSG(command)
 
		send_command(command);
		command_sent = 1;
	}
  
	if(read_gsm_buffer()){
		// wir haben eine Antwort bekommen
		
		// im GSM Mode verwerfen wir das OK nach CGATT 
		if(gsm_special_mode && (sm->get_state() == 3) ){
			if(State_Machine::compare_lines_trailing("OK",get_gsm_buffer())){
				DEBUG_MSG("ignore OK after CGATT? in gprs_setup mode")
				continue;
			}
		}
		// prüfen ob neuer Zustand
		DEBUG_MSG("AW:")
		DEBUG_MSG(get_gsm_buffer())
		last_state = sm->get_state();
		if(sm->match_line(get_gsm_buffer())){// Zustandsübergang hat stattgefunden, match gefunden
			// ist der alte zustand = dem neuen?
			if(last_state == sm->get_state()){
				// ja. zuviele retries?
				DEBUG_MSG("returning again to current state, retiries");
				DEBUG_MSG(retries);
				if(++retries > MAX_RETRY_AT_COMMAND){
					abort = 1;
				} else {
					DEBUG_MSG("delay before command retry for ms:")
					DEBUG_MSG(retry_time * retries)
					delay (retry_time * retries); // 500ms warten bis neuer versuch					
				}
			} else {
				// neuer zustand reset retry auf 0
				retries = 0;
			}
			// im nächsten loop passendes commando senden!
			command_sent = 0;
		} else {
			if(++retries > MAX_RETRY_AT_COMMAND){
					abort = 1;
			}
			DEBUG_MSG("no match for answer, retries:");
			DEBUG_MSG(retries);
		}
	} // ende des reads und zustandsübergang
  }
  return !abort;
}

bool GSM::initialize(char *pwd) { 
  State_Machine init_sm;
  byte size_at_command=sizeof("AT+CPIN=");
	
  char pin_setting_command[4 + size_at_command + 2]; // 4 pin chars, command chars, \r and 0
  strcpy (pin_setting_command,"AT+CPIN=");
  strcpy (pin_setting_command + size_at_command - 1,pwd); // write over intermedita 0
  strcpy (pin_setting_command + size_at_command + 3,"\r"); // write over inttermediate 0
  
  GSM_SERIAL.end();
 
  digitalWrite(GSM_POWER_PIN, LOW);                                              // disable Power GSM-Modul
  delay(1000);
  digitalWrite(GSM_POWER_PIN, HIGH);                                             // enable Power GSM-Modul

  GSM_SERIAL.begin(GSM_SERIAL_BAUD);
  
  buffer_pos=0;
  
  delay(3000);
  
  // run init state_machine
  // array of commands. index in array is = current state. append \r allways. ommit 1st state.
  char * init_commands[] = {"","AT\r","ATE0\r","AT+IPR=9600\r","AT#SIMDET=1\r","AT+CPIN?\r",pin_setting_command,"AT+CGATT=1\r",};
  init_sm.add_transition(1,2,"OK",State_Machine::compare_lines_trailing);
  init_sm.add_transition(2,3,"OK",State_Machine::compare_lines_trailing);
  init_sm.add_transition(3,4,"OK",State_Machine::compare_lines_trailing);
  init_sm.add_transition(4,5,"OK",State_Machine::compare_lines_trailing);
  init_sm.add_transition(5,6,"SIM PIN",State_Machine::compare_lines_trailing); // PIN not set
  init_sm.add_transition(5,5,"ERROR",State_Machine::compare_lines_trailing); // retry on PIN ERROR
  init_sm.add_transition(5,7,"READY",State_Machine::compare_lines_trailing); // PIN not set
  init_sm.add_transition(6,7,"OK",State_Machine::compare_lines_trailing); 

  init_sm.set_state(1);

  bool ret = execute_statemachine(&init_sm, init_commands,7,500,false);
  // puffer sauber hinterlassen, 1000 ms auf letzte Antworten warten und entfernen
  clear_uart(1000);
  return ret; 
}
             
bool GSM::send_SMS(char * phone_number, char * message){
  State_Machine sms_sm;
  
  // zusammenbauen des Strings für die SMS
  byte size_at_command = strlen("AT+CMGS=");
  byte size_message = strlen(message);
  byte size_phone_nr = strlen(phone_number);

  char  terminator[] = {26,0};
  char sms_command[250]; // 
  char message_terminated[250];
  strcpy (sms_command,"AT+CMGS=");
  strcpy (sms_command + size_at_command,phone_number); 
  strcpy (sms_command + size_at_command + size_phone_nr,"\r"); 
  strcpy (message_terminated,message); 
  strcpy (message_terminated + size_message, terminator); 

  DEBUG_MSG("SMS KOMMANDO:")
  DEBUG_MSG(sms_command)
  
  // setup zustandsmaschine
  char end_string[] = {26,'\r',0};
  char * sms_commands[] = {"","AT\r","AT+CMGF=1\r",sms_command,message_terminated,"AT\r"};
  sms_sm.add_transition(1,2,"",State_Machine::compare_lines_allways_match);
  sms_sm.add_transition(2,3,"OK",State_Machine::compare_lines_trailing);
  sms_sm.add_transition(3,4,">",State_Machine::compare_lines_trailing);
  sms_sm.add_transition(3,3,"ERROR: 314",State_Machine::compare_lines_trailing); 
  sms_sm.add_transition(3,3,"ERROR: 28",State_Machine::compare_lines_trailing); 
  sms_sm.add_transition(4,5,">",State_Machine::compare_lines_trailing);
  sms_sm.add_transition(3,3,"ERROR",State_Machine::compare_lines_trailing);
  sms_sm.add_transition(4,5,"OK",State_Machine::compare_lines_trailing);
  sms_sm.set_state(1);
    
  bool ret = execute_statemachine(&sms_sm, sms_commands,5,700,false);
   // puffer sauber hinterlassen, 1000 ms auf letzte Antworten warten und entfernen
  clear_uart(1000);
  return ret; 
}

bool GSM::setup_gprs(char * apn, char * user, char * pwd){
  State_Machine gprs_sm;
  
  // strings für commands vorbereiten
  char * command_tmp = "AT+CGDCONT=1,\"IP\",\"%s\",\"0.0.0.0\"\r";
  char at_ctcont_command[strlen(command_tmp)+1+strlen(apn)];
  sprintf(at_ctcont_command,command_tmp,apn);

  command_tmp = "AT#SGACT = 1, 1, \"%s\",\"%s\"\r";
  char at_ctatg_command[strlen(command_tmp)+1+strlen(user)+strlen(pwd)];
  sprintf(at_ctatg_command,command_tmp,user,pwd);
  
 
  // setup zustandsmaschine
  char end_string[] = {26,'\r',0};
  char * gprs_commands[] = {"",at_ctcont_command, "AT+CGATT=1\r","AT+CGATT?\r",at_ctatg_command};

  gprs_sm.add_transition(1,2,"OK",State_Machine::compare_lines_trailing); // CGDCOnt ok
  gprs_sm.add_transition(2,3,"OK",State_Machine::compare_lines_trailing); // CGATT=1 ok
  gprs_sm.add_transition(2,2,"ERROR",State_Machine::compare_lines_trailing); // CGATT:0
  gprs_sm.add_transition(3,3,"ERROR",State_Machine::compare_lines_trailing); // CGATT:0
  gprs_sm.add_transition(3,3,"+CGATT: 0",State_Machine::compare_lines_trailing); // CGATT:0
  gprs_sm.add_transition(3,4,"+CGATT: 1",State_Machine::compare_lines_trailing); // ok  
  
  gprs_sm.add_transition(4,5,"OK",State_Machine::compare_lines_trailing);
  
  gprs_sm.set_state(1);

  bool ret = execute_statemachine(&gprs_sm, gprs_commands,5,1500,true);
   // puffer sauber hinterlassen, 1000 ms auf letzte Antworten warten und entfernen
  clear_uart(3000);
  return ret; 
}

// returns true when buffer is valid (once)
// removes trailing <CR> and <LF>
bool GSM::read_gsm_buffer(){
	if(buffer_pos >= sizeof(gsm_buffer)){
		DEBUG_MSG("gsm buffer overvflow, resetting buffer");
		buffer_pos = 0 ;
		// return true so the application can handle overflow situations
	}
	if (GSM_SERIAL.available()) {
		byte tmp = GSM_SERIAL.read();
		gsm_buffer[buffer_pos++] = tmp;
		
		// check if we have reached the end of the answer and we did receive an answer
		if(buffer_pos > 2) { // more received than leading <LF><CR>
			if(gsm_buffer[buffer_pos-2] == '\r' && gsm_buffer[buffer_pos-1] == '\n'){
				gsm_buffer[buffer_pos-2] = 0; // terminate string before trailing 
				buffer_pos = 0;
				// ignore comments
				if(gsm_buffer[2] == '#'){
					return false;
				}
				return true;
			}
		}
	}		
	return false;
}

char * GSM::get_gsm_buffer(){
	return gsm_buffer;
}

void GSM::send_command(char * command){
	GSM_SERIAL.print(command);
}

char GSM::get_gsm_state(void) { 
	send_command("AT+CREG?\r");
	// blocking right now until response arives
	while(!read_gsm_buffer());
	char * response = get_gsm_buffer();
	
	// the status is after the first ','
	int pos = 0;
	while(response[pos++] != ',');
	return response[pos];
}

void GSM::clear_uart(int timeout){
	DEBUG_MSG("clearing uart, found command during time:")
	DEBUG_MSG(timeout);
	int time = millis();
	while((millis() - time) < timeout) {
		if(read_gsm_buffer()){
			DEBUG_MSG(get_gsm_buffer());
		}
	}
	DEBUG_MSG("done clearing");
}

bool GSM::send_data_via_gprs(char * server, char * port, char * message){
	State_Machine send_sm;
	
	char * tmp = "AT#SD=3,0,%s,\"%s\"\r";
	char connect_command[strlen(tmp) + 1 + strlen(server) + strlen(port)];
	sprintf(connect_command,tmp,port,server);
	
	char* commands[] = {"",connect_command,message};
	send_sm.add_transition(1,2,"CONNECT",State_Machine::compare_lines_trailing);
	send_sm.add_transition(2,3,"",State_Machine::compare_lines_allways_match);
	send_sm.set_state(1);
		
	bool ret = execute_statemachine(&send_sm, commands,3,700,false);
	DEBUG_MSG("sm executed");
	return ret; 
}