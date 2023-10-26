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

uint16_t adc_value;
	
void ADC_init() {
	// Configurar referencia de voltaje a AVCC con capacitor de desacoplo
	ADMUX |= (1 << REFS0);
	// Habilitar el ADC y configurar el prescaler a 128 para un rango de 0-5V
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_read(uint8_t channel) {
	// Seleccionar el canal de entrada
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
	// Iniciar la conversi?n
	ADCSRA |= (1 << ADSC);
	// Esperar a que la conversi?n se complete
	while (ADCSRA & (1 << ADSC));
	return ADC;
}
	
void LDR();

int main(void){
	setup1();
	while (1){
		switchValue = (PINB & (1 << PINB0)) ? 1 : (PINB & (1 << PINB4)) ? 2 : 0;
		cajadeCambios(&gear);
		hacerCambio(&gear);
		LDR();
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
	
	// Inicializar el ADC
	ADC_init();
	// Configurar el pin de salida para indicar el estado del potenci?metro
	DDRD |= (1 << PD7);
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
		PORTB &= ~(1 << PINB5);
	}
	else if ((*gear) > 0){
		OCR0A = vel[(*gear) + desfase];
		OCR2A = 0;
		PORTB = (1 << PINB5);
	}
	else if ((*gear) < 0){
		OCR2A = vel[(*gear) + desfase];
		OCR0A = 0;
		PORTD = (1 << PIND4);
	}
}

void LDR(){
	// Leer el valor ADC desde el potenciOmetro
	adc_value = ADC_read(0);
	if (adc_value < 700) {		// Si se tapa el LDR, el adc_value es menor
		PORTD |= (1 << PD1);
	}
	else {
		PORTD &= ~(1 << PD1);
	}
	_delay_ms(100);
}