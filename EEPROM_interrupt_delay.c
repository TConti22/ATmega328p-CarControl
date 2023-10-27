#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 1000000UL

#include <util/delay.h>


#define ESPERA 122 //aproximaciones Timer1: 122=1s
char END_TIME=1;
unsigned int cuenta=0;
float waiting=1;
unsigned int f1=0;
unsigned char contador=0;
char FE=1; //flag para EEPROM
char F2; //flag para contador


// Function prototype for the interrupt service routine (ISR)
ISR(INT0_vect);
void delay(float time);
void showContador(unsigned char counter);
void EEPROM_write(unsigned int uiAddress, unsigned char ucData);
unsigned char EEPROM_read(unsigned int uiAddress);

void setup(){
	
	// Inicialización de temporizadores y otros pines
	DDRD |= (1 << PIND7);//PRUEBA EEPROM
	
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
	
	PRR |= (1 << PRTIM1);
	contador=EEPROM_read(0x00);
}


int main(void){
	setup();
	
	while(1){
		//
	}
}

// Esta función se ejecutará cuando ocurra la interrupción en el pin INT0 (PD2)
ISR(INT0_vect){
	if(END_TIME==1){
		F2=1;
		contador++;
		PORTD &= ~(1 << PIND7);
		delay(0.3);
	}
}


ISR(TIMER1_OVF_vect){
	cuenta++;
	if(cuenta > ESPERA*waiting) {
		if (FE==1){
			sei(); //EEPROM
			FE=0;
		}
		cuenta=0;
		PRR |= (1 << PRTIM1); //apagar el Timer1, habrá que prenderlo cuando se realice otro movimiento
		END_TIME=1; //ahora mismo en desuso
		
		if (F2==1){
			f1++;
			showContador(contador);
		}
	}
}

void delay(float time){
	END_TIME=0; //
	waiting=time;
	PRR &= ~(1 << PRTIM1); //encender el Timer1
}

void showContador(unsigned char counter){
	if (f1<(counter*2+1)){
		PORTD ^= (1 << PIND7);
		delay(0.5);
	}
	else{
		PORTD &= ~(1 << PIND7);
		f1=0;
		F2=0;
		EEPROM_write(0x00000000,contador);
	}
}

void EEPROM_write(unsigned int uiAddress, unsigned char ucData){
	cli();
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address and Data Registers */
	EEAR = uiAddress;
	EEDR = ucData;
	/* Write logical one to EEMPE */
	EECR |= (1<<EEMPE);
	/* Start eeprom write by setting EEPE */
	EECR |= (1<<EEPE);
	FE=1;
	delay(0.008);
}

unsigned char EEPROM_read(unsigned int uiAddress){
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address register */
	EEAR = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from Data Register */
	return EEDR;
}