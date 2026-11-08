/*
 * Author: Stefan Schuster
 * File: State_Machine.h
 * Description: The header File of the state machine
 * 
 * License: This source is licensed under the Apache License V2.
 * See http://www.apache.org/licenses/LICENSE-2.0.html
 *
 * Changelog
 * V1.0: Added pre-defined matching functions and fixed bugs
 * V0.1: Initial Version
 */
 
// define TESTING to test class in local PC environment
#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#ifdef TESTING
	#define byte char
	#include <string.h>
#else
	#include <Arduino.h>
#endif

#define STATE_MACHINE_NO_MATCH 0 // input line does not trigger any change. initial state

#ifndef STATE_MACHINE_STATECOUNT 
	#warning (STATE_MACHINE_STATECOUNT not defined. Please define how many states to handle. Assuming 10.)
	#define STATE_MACHINE_STATECOUNT 10
#endif


typedef  byte (*match_func) (char *, char *);

struct transition {
	byte state;
	char * trigger_line;
	byte next_state;
	match_func f;
};

class State_Machine {     
    public:
		State_Machine();
		~State_Machine();
		byte get_state(); // returns current state
		void set_state(byte state);
		byte match_line(char * line); // returns state after match, STATE_MACHINE_NO_MATCH if no match is found
		void add_transition(byte start_state, byte next_state, char * trigger_line, match_func f); // adds a transition from start to next when line comes in
		 // Vordefinierte Funktionen für matching. 
		// Treffer bei gleichem Start
		static byte compare_lines_leading(char * trigger_line, char * input_line);
		// Treffer bei gleichem Ende
		static byte compare_lines_trailing(char * trigger_line, char * input_line);
		// Immer Treffer
		static byte compare_lines_allways_match(char * trigger_line, char * input_line);
  private:
		transition _transition_table[STATE_MACHINE_STATECOUNT];
		byte _state;
};
#endif
