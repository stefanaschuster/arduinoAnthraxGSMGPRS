/*
 * Author: Stefan Schuster
 * File: State_Machine.cpp
 * Description: The implementation file of the State Machine
 * 
 * License: This source is licensed under the Apache License V2.
 * See http://www.apache.org/licenses/LICENSE-2.0.html
 *
 * Changelog
 * V1.0: Added pre-defined matching functions and fixed bugs
 * V0.1: Initial Version
 */
 
#include "State_Machine.h"

#ifdef TESTING
	#define byte char
	#include <string.h>
#else
	#include <Arduino.h>
#endif

byte State_Machine::get_state(){
	return _state;
}

void  State_Machine::set_state(byte state){
	 _state = state;
}

byte State_Machine::match_line(char * line){
	byte done = 0;
	for(int i=0;i<STATE_MACHINE_STATECOUNT;i++){
		if(done == 1){
			// only handle first transition that is 
			// a) found for current state and b) matching
			break;
		}
		// find a transition from the current state 
		// (we loop so we can have more than one transition for a given state
		if(_transition_table[i].state == _state){
			// found transition for current state
			transition t = _transition_table[i];
			if(t.f(t.trigger_line,line)){
				// match!
				_state = t.next_state;
				done = 1;
			}
		} // end of current found state
	} // looped through all states
	return (done == 1) ? _state : STATE_MACHINE_NO_MATCH;
}

void State_Machine::add_transition(byte start_state, byte next_state, char * trigger_line,  match_func  f){
	for(int i=0;i<STATE_MACHINE_STATECOUNT;i++){
		if(_transition_table[i].state == 0){ // 0 is unitiliazied state transition
			_transition_table[i].state = start_state;
			_transition_table[i].next_state = next_state;
			_transition_table[i].trigger_line = trigger_line;
			if(f != 0){
				_transition_table[i].f = f;
			}else{
				_transition_table[i].f = State_Machine::compare_lines_leading;
			}
			break;
		}
	}
}

State_Machine::State_Machine(){
	for(int i=0;i<STATE_MACHINE_STATECOUNT;i++){
		_transition_table[i].state = 0;
		_transition_table[i].f = 0;
	}
}

State_Machine::~State_Machine(){}

byte State_Machine::compare_lines_leading(char * trigger_line, char * input_line){
	byte maxlen = (strlen(trigger_line) >= strlen(input_line)) ? strlen(input_line) : strlen(trigger_line);
	return !strncmp(trigger_line, input_line, maxlen);
}
// Treffer bei gleichem Ende
byte State_Machine::compare_lines_trailing(char * trigger_line, char * input_line){
	// check if input line ends with trigger line
  int trigger_len = strlen(trigger_line);
  int input_len = strlen(input_line);
  
  if(trigger_len > input_len){
    return 0; // no match, to short
  }  
  // start 1 = 1 to avoid check for empty 0
  for (int i = 0;i< trigger_len;i++){
    if(trigger_line[trigger_len - i] != input_line[input_len - i]){
      // not matching. 
      return 0;
    }
  }
  // all chars match
  return 1; 
}
// Immer Treffer
byte State_Machine::compare_lines_allways_match(char * trigger_line, char * input_line){
	return 1;
}