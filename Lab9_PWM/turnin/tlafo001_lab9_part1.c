/*	Author: tlafo001
 *  Partner(s) Name: 
 *	Lab Section: 022
 *	Assignment: Lab # 9  Exercise # 1
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
unsigned char tempA;

enum PWM_States { PWM_SMStart, PWM_NoButton, PWM_OneButton, PWM_MoreButtons } PWM_State;

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

void Tick_PWM() {
	switch(PWM_State) {
		case PWM_SMStart:
			set_PWM(0);
			PWM_State = PWM_NoButton;
			break;
		case PWM_NoButton:
			tempA = ((~PINA) & 0x07);
			if (tempA == 0x01 || tempA == 0x02 || tempA == 0x04)
			{
				if (tempA == 0x01)
				{
					frequency = 261.63;
				}
				else if (tempA == 0x02)
				{
					frequency = 293.66;
				}
				else if (tempA == 0x04)
				{
					frequency = 329.63;
				}
				PWM_State = PWM_OneButton;
			}
			else if (tempA)
			{
				PWM_State = PWM_MoreButtons;
			}
			else if (tempA == 0x00)
			{
				PWM_State = PWM_NoButton;
			}
			break;
		case PWM_OneButton:
			tempA = ((~PINA) & 0x07);
			if (tempA == 0x00)
			{
				PWM_State = PWM_NoButton;
			}
			else if (!(tempA == 0x01 || tempA == 0x02 || tempA == 0x04))
			{
				PWM_State = PWM_MoreButtons;
			}
			else if (tempA == 0x01 || tempA == 0x02 || tempA == 0x04)
			{
				if (tempA == 0x01)
				{
					frequency = 261.63;
				}
				else if (tempA == 0x02)
				{
					frequency = 293.66;
				}
				else if (tempA == 0x04)
				{
					frequency = 329.63;
				}
				PWM_State = PWM_OneButton;
			}
			break;
		case PWM_MoreButtons:
			tempA = ((~PINA) & 0x07);
			if (tempA == 0x00)
			{
				PWM_State = PWM_NoButton;
			}
			else if (tempA == 0x01 || tempA == 0x02 || tempA == 0x04)
			{
				if (tempA == 0x01)
				{
					frequency = 261.63;
				}
				else if (tempA == 0x02)
				{
					frequency = 293.66;
				}
				else if (tempA == 0x04)
				{
					frequency = 329.63;
				}
				PWM_State = PWM_OneButton;
			}
			else if (!(tempA == 0x01 || tempA == 0x02 || tempA == 0x04))
			{
				PWM_State = PWM_MoreButtons;
			}
			break;
		default:
			PWM_State = PWM_SMStart;
			break;
	}

	switch(PWM_State) {
		case PWM_NoButton:
			set_PWM(0);
			break;
		case PWM_OneButton:
			set_PWM(frequency);
			break;
		case PWM_MoreButtons:
			set_PWM(0);
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
	TimerOn();
	PWM_on();
	PWM_State = PWM_SMStart;

    while (1) {
	Tick_PWM();
	while(!TimerFlag);
	TimerFlag = 0;
    }
    return 1;
}
