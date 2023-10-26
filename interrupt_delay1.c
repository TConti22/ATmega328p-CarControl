//Interrupciones y función delay con Timer1
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 1000000UL

#include <util/delay.h>


#define ESPERA 122 //aproximaciones Timer1: 122=1s
char END_TIME=1;
unsigned int cuenta=0;
float waiting=1;

void setup(){
	
	// Inicialización de temporizadores y otros pines
	PORTD |= (1 << PIND6);
	DDRD |= (1 << PIND6);
	TCCR0A = 0x83;
	TCCR0B = 0x01;
	OCR0A = 0;
	
	/* disable interrupts during timed sequence */
	cli();
	EICRA|=(1<<ISC01)|(1<<ISC00);
	EIMSK|=(1<<INT0);
	
	TIMSK1 = TIMSK1|0b00000001; //Habilita la interrupcion por desbordamiento
	TCCR1B = 0b00000001; //Configura el preescaler del Timer1 en 64 (máximo)
	
	//TIMSK2 = TIMSK2|0b00000001; //Habilita la interrupcion por desbordamiento
	//TCCR2B = 0b00000111; //Configura el preescaler del Timer2 para que FT2 sea de 7812.5Hz (máximo)
	sei();
	
	
}

// Function prototype for the interrupt service routine (ISR)
ISR(INT0_vect);
void delay(float time);

int main(void){
	setup();
	
	while(1){
		

	}
}

// Esta función se ejecutará cuando ocurra la interrupción en el pin INT0 (PD2)
ISR(INT0_vect){
	if(END_TIME==1){
		delay(0.3);
		OCR0A += 40;
	}
}


ISR(TIMER1_OVF_vect){
	cuenta++;
	if(cuenta > ESPERA*waiting) {
		cuenta=0;
		PRR |= (1 << PRTIM1); //apagar el Timer1, habrá que prenderlo cuando se realice otro movimiento
		END_TIME=1; //ahora mismo en desuso
	}
}

void delay(float time){
	END_TIME=0; //
	waiting=time;
	PRR &= ~(1 << PRTIM1); //encender el Timer1
}