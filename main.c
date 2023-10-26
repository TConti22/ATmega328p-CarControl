/*
 * main.c
 *
 * Created: 10/24/2023 3:19:16 PM
 *  Author: svenp
 */ 

#define F_CPU 1000000UL

#include <xc.h>
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	DDRB = 0xFF;
	DDRD = 0xFF;
	TCCR0A = 0x83;
	TCCR0B = 0x01;
	
	TCCR2A = 0x83;
	TCCR2B = 0x01;
	
	int vel = 0;
	uint8_t check = 0;
	int switchValue;
	
    while(1)
    {
		switchValue = (PINB & (1 << PINB0)) ? 1 : (PINB & (1 << PINB4)) ? 2 : 0;
		_delay_ms(5000);
		switch (switchValue){
			case 1:
				if(check == 0){
					vel += 130;
					check = 1;
				}else{
					if(vel == -130){
						vel += 130;
					}
					vel += 42;
				}
				//vel += 64;
				if (vel >= 255){
					vel = 256;
					OCR2A = 0;
					OCR0A = 255;
				}else if(vel > 0 && vel <= 255){
					OCR2A = 0;
					OCR0A = vel;					
				}else if(vel < 0){
					OCR2A = vel*(-1);
					OCR0A = 0;
				}else if(vel == 0){
					OCR2A = 0;
					OCR0A = 0;
				}
				break;
			case 2:
				if(check == 0){
					vel -= 130;
					check = 1;
				}else{
					if(vel == 130){
					vel -= 130;
				}
					vel -= 42;
				}
				if(vel > 0 && vel <= 255){
					OCR2A = 0;
					OCR0A = vel;
				}else if(vel < 0 && vel >= -255){
					OCR2A = vel*(-1);
					OCR0A = 0;
				}else if(vel <= -255){
					vel = -256;				
					OCR2A = 255;
					OCR0A = 0;
				}else if(vel == 0){
					OCR2A = 0;
					OCR0A = 0;
				}
				break;
		}
		
		if(vel < 0){
			PORTD = (1<<PORTD3);
		}else if(vel == 0){
			PORTD = 0;
			check = 0;
		}else{
			PORTD = (1<<PORTD4);
		}
		
	}
}