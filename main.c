#define F_CPU 1000000UL

#include <xc.h>
#include <avr/io.h>
#include <util/delay.h>

void setup1();
void cajadeCambios(int *gear);
void hacerCambio(int *gear);

uint8_t desfase = 4;
uint8_t switchValue;
int gear = 0;
int vel[] = {255, 214, 172, 130, 0, 130, 172, 214, 255};

int main(void){
	setup1();
	while (1){
		switchValue = (PINB & (1 << PINB0)) ? 1 : (PINB & (1 << PINB4)) ? 2 : 0;
		cajadeCambios(&gear);
		hacerCambio(&gear);
		_delay_ms(2000);
	}
}

void setup1(){
	DDRB = 0xFF;
	DDRD = 0xFF;
	TCCR0A = 0x83;
	TCCR0B = 0x01;
	
	TCCR2A = 0x83;
	TCCR2B = 0x01;
}

void cajadeCambios(int *gear){
	switch (switchValue){
		case 1:
		(*gear) += 1;
		if ((*gear) > 4){
			(*gear) = 4;
		}
		break;
		case 2:
		(*gear) -= 1;
		if ((*gear) < -4){
			(*gear) = -4;
		}
		break;
	}
}

void hacerCambio(int *gear){
	if ((*gear) == 0){
		OCR2A = 0;
		OCR0A = 0;
		PORTD = 0;
	}
	else if ((*gear) > 0){
		OCR0A = vel[(*gear) + desfase];
		OCR2A = 0;
		PORTD = (1 << PIND4);
	}
	else if ((*gear) < 0){
		OCR2A = vel[(*gear) + desfase];
		OCR0A = 0;
		PORTD = (1 << PIND3);
	}
}