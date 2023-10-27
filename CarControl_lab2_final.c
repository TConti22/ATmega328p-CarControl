#define F_CPU 1000000UL  // macro para definir la frecuencia del clock de la CPU, esto ayuda entre otras cosas a la compativilidad con algunas librerías
//librerías a incluir
#include <xc.h>					//definiciones registros y configuraciones específicas para la arquitectura AVR
#include <avr/io.h>				//definiciones para los puertos de entrada/salida y otros registros relacionados con el hardware del microcontrolador AVR
#include <avr/interrupt.h>		//funciones y macros para habilitar y gestionar interrupcionesi
#include <util/delay.h>			//generar delays

//definición de variables globales y flags
#define ESPERA 122 //constante. Aproximaciones Timer1: 122=1s
char END_TIME=1;
unsigned int cuenta=0;
float waiting=1;
unsigned int f1=0;
unsigned char contador=0; //
char FE=1; //flag para EEPROM
char F2; //flag para contador

uint8_t desfase = 4;
uint8_t switchValue;
int gear = 0;
int vel[] = {255, 214, 172, 130, 0, 130, 172, 214, 255};
uint16_t adc_value;


//Declaración de funciones
void setup1();
void setup();
void cajadeCambios(int *gear);
void hacerCambio(int *gear);
ISR(INT0_vect);
void delay(float time);
void showContador(unsigned char counter);
void EEPROM_write(unsigned int uiAddress, unsigned char ucData);
unsigned char EEPROM_read(unsigned int uiAddress);
void LDR(); //función para el sensor luminico


//inicialización del ADC
void ADC_init() {
	// Configurar referencia de voltaje a AVCC con capacitor de desacoplo
	ADMUX |= (1 << REFS0);
	// Habilitar el ADC y configurar el prescaler a 128 para un rango de 0-5V
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

//leer ADC
uint16_t ADC_read(uint8_t channel) {
	// Seleccionar el canal de entrada
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
	// Iniciar la conversion
	ADCSRA |= (1 << ADSC);
	// Esperar a que la conversion se complete
	while (ADCSRA & (1 << ADSC));
	return ADC;
}

void setup(){
	
	// Inicialización de temporizadores y otros pines
	
	PORTD |= (1 << PIND6); //para pwm
	DDRD |= (1 << PIND6);  //para pwm
	//configurar Timer0
	TCCR0A = 0x83;  //configurar el modo del Timer, en este caso: fastPWM
	TCCR0B = 0x01;   //configurar prescaler
	OCR0A = 0;   //marcador para el duty cycle
	
	//configuracion de interrupciones
	cli(); //desactivar interrupciones
	EICRA|=(1<<ISC01)|(1<<ISC00);		//activar en rising edge
	EIMSK|=(1<<INT0);					//interrupcion en INT0 (PIND2)
	
	TIMSK1 = TIMSK1|0b00000001; //Habilita la interrupcion por desbordamiento
	TCCR1B = 0b00000001; //Configura el Timer1 en el modo sin prescaler
	
	sei();  //activar interrupciones
	
	PRR |= (1 << PRTIM1);  //apagar el Timer1 para forzar que no haya una interrupcion por desbordamiento poco después de comenzar el programa
	contador=EEPROM_read(0x00);  //leer el registro de la EEPROM correspondiente al contador de choques, cada vez que se energiza el ATmega (luego de haber sido apagado)
	//esto permite llevar el conteo incluso cuando el ATmega ha sido desenergizado
}

int main(void){
	setup(); //inicializaciones 1
	setup1(); //inicializaciones 2
	//bucle principal (hace de Core Block)
	while (1){
		switchValue = (PINB & (1 << PINB5)) ? 1 : (PINB & (1 << PINB4)) ? 2 : 0;  //si el botón de subir velocidad se presiona: switchValue=1; sino, si se preciona bajar velocidad, switchValule=2; sino switchValule=0.
		if(PINB & (1 << PINB5) && PINB & (1 << PINB4)){ //si ambos botones de marcha están presionados, entra en el condicional. Esto hace de boton de encendido.
			_delay_ms(8000);	//margen para soltar botones
			while(!(PINB & (1 << PINB5) && PINB & (1 << PINB4))){ //si vuelven a estar ambos botones presionados a la vez, el programa se "apaga". Con lo cual ambos botones en simultaneo también hacen de boton de apagado.7
				switchValue = (PINB & (1 << PINB5)) ? 1 : (PINB & (1 << PINB4)) ? 2 : 0; //se chequea nuevamente si se presiona algun boton de velocidad por separado
				cajadeCambios(&gear); //se marca la marcha correspondiente
				hacerCambio(&gear);  //se ejecuta la marcha correspondiente
				LDR(); //función para el sensor lumínico
				_delay_ms(2000);
			}
			_delay_ms(8000);	//margen para soltar botones
		}
		//apagar motores y poner marcha en neutro
		OCR2A = 0;
		OCR0A = 0;
		PORTD = 0;
		gear = 0;
		PORTB &= ~(1 << PINB5);
	}
}

void setup1(){
	DDRB = 0xFF;
	DDRD = 0xFF;
	//configurando Timer0 para	PWM
	TCCR0A = 0x83;
	TCCR0B = 0x01;
	//configurando Timer2 para PWM
	TCCR2A = 0x83;
	TCCR2B = 0x01;
	
	// Inicializar el ADC
	ADC_init();
	// Configurar el pin de salida para indicar el estado del LDR
	DDRD |= (1 << PIND1);
}

//dependiendo de que botón se apretó (guardado en el main), se marca la marcha
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

//se pasa a la marcha correspondiente
void hacerCambio(int *gear){
	if ((*gear) == 0){ //si gear==0, se pone en neutro la marcha
		OCR2A = 0;
		OCR0A = 0;
		PORTD = 0;
		PORTB &= ~(1 << PINB5);
	}
	else if ((*gear) > 0){	//carga el dutycylce para marchas hacia delante
		OCR0A = vel[(*gear) + desfase];
		OCR2A = 0;
		PORTD = (1 << PIND3); // encender luz verde
	}
	else if ((*gear) < 0){	//carga el dutycylce para marcha atrás
		OCR2A = vel[(*gear) + desfase];
		OCR0A = 0;
		PORTD = (1 << PIND4);	//encender luz roja
	}
}

void LDR(){
	// Leer el valor ADC desde el sensor lumínico
	adc_value = ADC_read(0);
	if (adc_value < 500) {		// Si se tapa el LDR, el adc_value es menor
		PORTD |= (1 << PIND1);
	}
	else {
		PORTD &= ~(1 << PIND1);
	}
	_delay_ms(100);
}



// Esta función se ejecutará cuando ocurra la interrupción en el pin INT0 (PD2) (cuando el auto choque)
ISR(INT0_vect){
	if(END_TIME==1){ //el condicional funciona como debounce
		F2=1;	//settear flag
		contador++;	//aumentar el conador de la cantidad de veces que choca
		PORTD &= ~(1 << PIND4);		//forzar apagado del led (para luego mar
		gear=0;	//se pone el auto en neutro
		delay(0.3);	//se ramifica la interrupción
	}
}


ISR(TIMER1_OVF_vect){
	cuenta++;	//aumenta la cuenta
	if(cuenta > ESPERA*waiting) { //el Timer tendrá que desbordar varias veces para llegar a tiempo en órdenes de segundos o un incluso décimas de segundos
		if (FE==1){	//solo se escribe si es necesario
			sei(); //volver a activar las interrupciones al haberlas apagado por escritura
			FE=0;
		}
		cuenta=0; //reset para volver a contar de 0 la proxima vez
		PRR |= (1 << PRTIM1); //apagar el Timer1, habrá que prenderlo cuando se lo necesite, así no está interrumpiendo cuando no se necesita
		END_TIME=1; //se vuelve a tener en cuenta la interrupción por choque
		
		if (F2==1){  //solo se escribe si el auto chocó
			f1++;	//actualizar flag, debe seguir cambiando de estado el led de choques
			showContador(contador);	//prender y apagar led para marcar la cantidad de choques
		}
	}
}

void delay(float time){
	END_TIME=0; //para no hacer caso a la interrupción (funciona de debounce)
	waiting=time;	//para configurar la cantidad de tiempo que espera el delay
	PRR &= ~(1 << PRTIM1); //encender el Timer1 (y así esperar el tiempo que se predefinió, sin detener el flujo del programa)
}

void showContador(unsigned char counter){
	if (f1<(counter*2+1)){
		PORTD ^= (1 << PIND4);	//cambia de estado el led
		delay(0.5);		//espera un tiempo por si debe volver a cambiar de estado (a una frecuencia que permita visualizar y contar)
	}
	else{ //termino de mostrar el contador
		PORTD &= ~(1 << PIND4);	//forzar apagado del led
		f1=0;	//reset flag
		F2=0;	//reset flag
		EEPROM_write(0x00000000,contador);	//escribir contador en la EEPROM, así el registro queda guardado inlcuso si se desenergiza el cirucito.
	}
}

//escribir en la EEPROM
void EEPROM_write(unsigned int uiAddress, unsigned char ucData){
	cli();	//el proceso tarda unos milisegundos y no puede ser interrumpido
	//Esperar a que se complete el write anterior de ser necesario
	while(EECR & (1<<EEPE))
	;
	//Marcar adress de EEPROM y valor del contador
	EEAR = uiAddress;
	EEDR = ucData;
	//escribir 1 lógico en EEMPE
	EECR |= (1<<EEMPE);
	//comenzar escritura en eeprom seteando EEPE
	EECR |= (1<<EEPE);
	FE=1;	//set flag
	delay(0.008);	//esperar a que termine de escribir para reactivar interrupciones
}

//leer en la EEPROM
unsigned char EEPROM_read(unsigned int uiAddress){
	//Esperar a que se complete el write anterior de ser necesario
	while(EECR & (1<<EEPE))
	;
	//set adress de EEPROM
	EEAR = uiAddress;
	//comenzar lectura en eeprom seteando EERE */
	EECR |= (1<<EERE);
	//devuelve el valor en la dirección seleccionada
	return EEDR;
}