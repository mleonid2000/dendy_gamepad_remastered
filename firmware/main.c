/* Используемые библиотеки */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

/*===========================================================================*/
/* Конфигурация портов ATmega8                                               */
/*===========================================================================*/

//#define F_CPU 8000000L #Задается в 'makefile'

/* Биты нажатых кнопок */
#define INPUT_RIGHT		((PINC & _BV(PC4)) ? 0x80 : 0)
#define INPUT_LEFT		((PINC & _BV(PC2)) ? 0x40 : 0)
#define INPUT_DOWN		((PINC & _BV(PC5)) ? 0x20 : 0)
#define INPUT_UP		((PINC & _BV(PC3)) ? 0x10 : 0)

#define INPUT_START		((PIND & _BV(PD5)) ? 0x08 : 0)
#define INPUT_SELECT		((PIND & _BV(PD6)) ? 0x04 : 0)
#define INPUT_B			((PIND & _BV(PD7)) ? 0x02 : 0)
#define INPUT_A			((PINB & _BV(PB0)) ? 0x01 : 0)

/* Биты кнопок Турбо B A */
#define INPUT_TB		((PINB & _BV(PB2)) ? 0 : 0x02)
#define INPUT_TA		((PINB & _BV(PB1)) ? 0 : 0x01)

/* Переменные */
volatile uint8_t shift, button_data = 0;

volatile uint8_t turbo_data = 0, turbo_shift = 0, turbo_pattern_shift;
volatile uint16_t turbo_pattern = 0xA995;

/* Инициализация портов и регистров */
void hwInit(void)
{
	//Все входы
	//PB0,PB1,PB2 остальные в 0
	PORTB	 =	0x07;
	DDRB	 =	( 0 << PB0 ) | ( 0 << PB1 ) | ( 0 << PB2 ) |
			( 0 << PB3 ) | ( 0 << PB4 ) | ( 0 << PB5 ) ;

	//Все входы
	PORTC	 =	0x3C;
	DDRC	 =	( 0 << PC0 ) | ( 0 << PC1 ) | ( 0 << PC2 ) |
			( 0 << PC3 ) | ( 0 << PC4 ) | ( 0 << PC5 ) ;

	PORTD	 =	0xE0;
	DDRD	 =	( 1 << PD0 ) | ( 0 << PD1 ) | ( 0 << PD2 ) |
			( 0 << PD3 ) | ( 0 << PD4 ) | ( 0 << PD5 ) |
			( 0 << PD6 ) | ( 0 << PD7 );

#if (F_CPU == 8000000L)
	//8 МГц  предделитель 1024
	TCCR2	 =	( 0 << WGM20 ) | ( 1 << WGM21) | ( 1 << CS22  ) |
			( 1 << CS21  ) | ( 1 << CS20 );

	//Регистр сравнения, время счета 3.968 мс при частоте 8 МГц
	OCR2  = 0x1F;
	TCNT2 = 0xE1;

#elif (F_CPU == 16000000L)
	// 16 МГц  предделитель 1024
	TCCR2	 =	( 0 << WGM20 ) | ( 1 << WGM21) | ( 1 << CS22  ) |
			( 1 << CS21  ) | ( 1 << CS20 );

	//Регистр сравнения, время счета 3.968 мс при частоте 16 МГц
	OCR2  = 0x3E;
	TCNT2 = 0xC2;
#endif

	//Включение прерывания при растущем Latch, Clock
	MCUCR = (1 << ISC01) | (1 << ISC00) | (1 << ISC11) | (1 << ISC10);
	GICR  = (1 << INT0 ) | (1 << INT1 );
}

/* Прерывание при растущем Clock */
ISR(INT0_vect)
{
	shift <<= 1;
	if (button_data & shift)
		PORTD |= 1;
	else
		PORTD &= ~1;
}

/* Прерывание при растущем Latch */
ISR(INT1_vect)
{
	shift = 1;

	if (button_data & shift)
		PORTD |= 1;
	else
		PORTD &= ~1;

	turbo_shift++;
}

/* Считать нажатые кнопки */
void buttons_read(void)
{
//Пропустить,  если не прошло время 4 мс ("Защита от дребезга контактов")
if (bit_is_set(TIFR, OCF2))
	{
		// Сброс таймера отсчета
		TIFR |= _BV(OCF2);

#if (F_CPU == 8000000L)
		// 8Mhz
		OCR2  = 0x1F;
		TCNT2 = 0xE1;

#elif (F_CPU == 16000000L)
		// 16Mhz
		OCR2  = 0x3E;
		TCNT2 = 0xC2;
#endif
		/* 
			Счтитать нажатые кнопки
			Right, Left, Down, Up, Start, Select, B, A
		*/
		button_data =	INPUT_UP	|	INPUT_DOWN	|	INPUT_LEFT	|	INPUT_RIGHT	|
					INPUT_B		|	INPUT_A		|
					INPUT_START	|	INPUT_SELECT;
		
		/* Если нажаты кнопки турбо TB, TA */
		if (INPUT_TB) button_data = (button_data & ~0x02) | (turbo_data & 0x02);
		if (INPUT_TA) button_data = (button_data & ~0x01) | (turbo_data & 0x01);
	}
}

/*===========================================================================*/
/* Начало основной программы */
int main(void)
{
	hwInit();
	sei();

	while (1)
		{
			/* считать нажатые кнопки */
			buttons_read();

			/* Инверсия турбо */
			if (turbo_shift >= 1)
				{
					turbo_data = ((turbo_pattern >> turbo_pattern_shift) & 0x01) * 3;
					turbo_shift = 0;
				}

			if (turbo_pattern_shift >= 0x000F)
				turbo_pattern_shift = 0;
		}
}
