/*	Author: tlafo001
 *  Partner(s) Name: 
 *	Lab Section: 022
 *	Assignment: Lab # 9  Exercise # 2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#endif

double frequency;
unsigned char pos = 0;
double notes[8] = { 261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25 };

enum Toggle_States { Toggle_SMStart, Toggle_OffUnpress, Toggle_OnPress, Toggle_OnUnpress, Toggle_OffPress } Toggle_State;

enum Scale_States { Scale_SMStart, Scale_Neutral, Scale_Change } Scale_State;

// 0.954 hz is lowest frequency possible with this function,
// based on settings in PWM_on()
// Passing in 0 as the frequency will stop the speaker from generating sound
void set_PWM(double frequency) {
	static double current_frequency; // Keeps track of the current set frequency
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted.
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; } //stops timer/counter
		else { TCCR3B |= 0x03; } // resumes/continues timer/counter

		// prevents OCR3A from overflowing, using prescalar 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR3A = 0xFFFF; }

		// prevents OCR3A from underflowing, using prescalar 64
		// 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR3A = 0x0000; }

		// set OCR3A based on desired frequency
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }

		TCNT3 = 0; // resets counter
		current_frequency = frequency; // Updates the current_frequency
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
		// COM3A0: Toggle PB3 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
		// WGM32: When counter (TCNT3) matches OCR3A, reset counter
		// CS31 & CS30: Set a prescalar of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

void Tick_Toggle() {
	switch(Toggle_State) {
		case Toggle_SMStart:
			set_PWM(0);
			Toggle_State = Toggle_OffUnpress;
			break;
		case Toggle_OffUnpress:
			if ((PINA & 0x01) == 0x00)
			{
				Toggle_State = Toggle_OnPress;
			}
			else if ((PINA & 0x01) == 1)
			{
				Toggle_State = Toggle_OffUnpress;
			}
			break;
		case Toggle_OnPress:
			if ((PINA & 0x01) == 0x01)
			{
				Toggle_State = Toggle_OnUnpress;
			}
			else if ((PINA & 0x01) == 0x00)
			{
				Toggle_State = Toggle_OnPress;
			}
			break;
		case Toggle_OnUnpress:
			if ((PINA & 0x01) == 0x00)
			{
				Toggle_State = Toggle_OffPress;
			}
			else if ((PINA & 0x01) == 0x01)
			{
				Toggle_State = Toggle_OnUnpress;
			}
			break;
		case Toggle_OffPress:
			if ((PINA & 0x01) == 0x01)
			{
				Toggle_State = Toggle_OffUnpress;
			}
			else if ((PINA & 0x01) == 0x00)
			{
				Toggle_State = Toggle_OffPress;
			}
			break;
		default:
			Toggle_State = Toggle_SMStart;
			break;
	}

	switch(Toggle_State) {
		case Toggle_OffUnpress:
			set_PWM(0);
			break;
		case Toggle_OnPress:
			set_PWM(frequency);
			break;
		case Toggle_OnUnpress:
			set_PWM(frequency);
			break;
		case Toggle_OffPress:
			set_PWM(0);
			break;
		default:
			break;
	}
}
void Tick_Scale() {
	switch(Scale_State) {
		case Scale_SMStart:
			frequency = notes[pos];
			Scale_State = Scale_Neutral;
			break;
		case Scale_Neutral:
			if ((PINA & 0x06) == 0x04)
			{
				if (pos > 0)
				{
					pos--;;
				}
				frequency = notes[pos];
				Scale_State = Scale_Change;
			}
			else if ((PINA & 0x06) == 0x02)
			{
				if (pos < 7)
				{
					pos++;
				}
				frequency = notes[pos];
				Scale_State = Scale_Change;
			}
			else if (((PINA & 0x06) == 0x00) || ((PINA & 0x06) == 0x06))
			{
				Scale_State = Scale_Neutral;
			}
			break;
		case Scale_Change:
			if ((PINA & 0x06) == 0x06)
			{
				Scale_State = Scale_Neutral;
			}
			else if (!((PINA & 0x06) == 0x06))
			{
				Scale_State = Scale_Change;
			}
			break;
		default:
			Scale_State = Scale_SMStart;
			break;
	}

	switch(Scale_State) {
		case Scale_Neutral:
			break;
		case Scale_Change:
			break;
		default:
			break;
	}
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
    /* Insert your solution below */
	TimerSet(100);
	PWM_on();
	TimerOn();
	Toggle_State = Toggle_SMStart;
	Scale_State = Scale_SMStart;

    while (1) {
	Tick_Toggle();
	Tick_Scale();
	while(!TimerFlag);
	TimerFlag = 0;
    }
    return 1;
}
