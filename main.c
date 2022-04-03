/*
 * Sprint05.c
 *
 * Created: 13/02/2022 18:17:43
 * Author : bruno
 */ 

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1 
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "SSD1306.h"
#include "Font5x8.h"

//Inicialisacao das variaveis
uint8_t diam, letra=2, troca=0, flag_T = 0, flag_cinto = 1, flag_trava, flag_porta, id_porta, flag_ativo;
uint16_t voltas = 0, rpm, dist, timer1 = 0, tempo_ms = 0, m, porc, V, a = 1, dist_aux = 0, t_borda_subida, delta_t, dist_cm, T, T_aux, esc, n_p;
float delta_s, delta_c, aux;
char dia[5], RPM[6], DIST[5], CM[6], POC[4], TEMP[5], modo[3], teste, MAX_TEMP[5], VEL[5];

ISR(INT0_vect)
{
	voltas++;
	timer1++;
}

ISR(PCINT0_vect)
{
	static int8_t aux = 0;
	if ((PINB&0b00000010) == 0)
	{
		switch(aux)
		{
			case 0:
				flag_cinto = 0;
				aux = 1;
				break;
			case 1:
				flag_cinto = 1;
				aux = 0;
				break;	
		}
	}
}
ISR(PCINT2_vect)
{
	if ((PIND&0b00100000) == 0) //Ciclo da subtração
	{
		if(diam > 1)
		{
			diam--;
			eeprom_write_byte(0, diam);
		}
	}

	if ((PIND&0b00010000) == 0) //Ciclo da soma
	{
		if(diam < 200)
		{
			diam++;
			eeprom_write_byte(0, diam);
		}
	}

	if(!(PIND & (1<<7)))
	{
		letra = 2;	
	}
	
	else
	{	
		if(!(PIND & (1<<6)))
		{
			letra = 0;
		}
		
		else
		{
			letra = 1;
		}
	}
}

ISR(TIMER0_COMPA_vect)
{
	static float aux_1;
	static int8_t perc = 0;
	
	tempo_ms++;
	
	if ((tempo_ms % 100) == 0)
	{
		if (n_p == 0)
		{
			flag_porta = 0;
		}
		if (n_p == 1)
		{
			flag_porta = 1;
			id_porta = 1;
		}
		if (n_p == 2)
		{
			flag_porta = 1;
			id_porta = 2;
		}
		if (n_p == 5)
		{
			flag_porta = 1;
			id_porta = 3;
		}
		if (n_p == 11)
		{
			flag_porta = 1;
			id_porta = 4;
		}
		if (n_p == 4)
		{
			flag_porta = 1;
			id_porta = 5;
		}
		if (n_p == 7)
		{
			flag_porta = 1;
			id_porta = 6;
		}
		if (n_p == 13)
		{
			flag_porta = 1;
			id_porta = 7;
		}
		if (n_p == 8)
		{
			flag_porta = 1;
			id_porta = 8;
		}
		if (n_p == 14)
		{
			flag_porta = 1;
			id_porta = 9;
		}
		if (n_p == 17)
		{
			flag_porta = 1;
			id_porta = 10;
		}
		if (n_p == 10)
		{
			flag_porta = 1;
			id_porta = 11;
		}
		if (n_p == 16)
		{
			flag_porta = 1;
			id_porta = 12;
		}
		if (n_p == 19)
		{
			flag_porta = 1;
			id_porta = 13;
		}
		if (n_p == 20)
		{
			flag_porta = 1;
			id_porta = 14;
		}
		if (n_p == 22)
		{
			flag_porta = 1;
			id_porta = 15;
		}
	}
		
	if((tempo_ms % 1000)==0)
	{
		aux_1 = timer1;
		timer1 = 0;
		delta_s = (1/(aux_1));
		rpm = 60/delta_s;
		if (dist_aux != 0)
		{
			dist++;
			voltas = 0;
		}
		if (perc != dist)
		{
			eeprom_write_byte(4, dist);
			perc = dist;
		}
	}
}

ISR(TIMER1_CAPT_vect)
{
	if (TCCR1B & (1<<ICES1))
	{
		t_borda_subida = ICR1;
	}
	else
	{
		delta_t = (ICR1 - t_borda_subida)*16;
	}
	TCCR1B ^= (1<<ICES1);
}

ISR(ADC_vect)
	{
		switch(troca)
		{	
		case 0:
			if ((dist_cm < 300) && (V > 20))
			{
				OCR2B = 25;
			}
			else
			{
				OCR2B = (float)ADC*0.25;
			}
			ADMUX = 0b01000001;
			break;
		case 1:
			porc = ((uint32_t)ADC*100)/1023;
			ADMUX = 0b01000010;
			break;
		case 2:
			if (flag_T = 0)
			{
				T_aux = eeprom_read_byte(8);
				flag_T = 1;
			}
			T = (uint32_t)ADC*2597/(1023-ADC) - 259;
			if (T >= T_aux)
			{
				eeprom_write_byte(8, T);
				T_aux = eeprom_read_byte(8);
				m = sprintf(MAX_TEMP, "%d", T_aux);
			}
			ADMUX = 0b01000011;
			break;
		case 3:
			n_p = (ADC*3.75)/128;
			ADMUX = 0b01000000;
			troca = -1;
			break;
		}
		troca++;
	}

ISR(USART_RX_vect)
{
	char recebido;
	static int i;
	recebido = UDR0;
	if (recebido == 'd')
	{
		for (i = 0; MAX_TEMP[i] != '\0'; i++)
		{
			UDR0 = MAX_TEMP[i];
		}
	}
	if (recebido == 'l')
	{
		eeprom_write_byte(8, 0);
		T_aux = eeprom_read_byte(8);
		m = sprintf(MAX_TEMP, "%d", T_aux);
	}
	if (recebido == 't')
	{
		if (flag_porta == 0)
		{
			flag_trava = 1;
		}
		
	}
	if (recebido == 'a')
	{
		flag_trava = 0;
		flag_ativo = 0;
	}
}

int main(void)
{
	//Indicando estado das portas (entradas ou saídas)
	DDRB = 0b11111100;
	DDRC = 0b10111001;
	DDRD &= 0b00001001;
	PORTB = 0b00000010;
	PORTD = 0b00110000;
	//Interrupções externas
	PCICR = 0b00000101;
	PCMSK2 = 0b11110000;
	PCMSK0 = 0b00000010;
	EICRA = 0b00000010;
	EIMSK = 0b00000001;
	//Timers
	TCCR0A = 0b00000010;
	TCCR0B = 0b00000011;
	TCCR2A = 0b00100011;
	TCCR2B = 0b00000011;
	OCR0A = 249;
	TIMSK0 = 0b00000010;
	//Configurações ADC
	ADMUX = 0b01000000;
	ADCSRA = 0b11101111;
	ADCSRB = 0b00000000;
	DIDR0 = 0b00111110;
	//Modo de Captura
	TCCR1B = (1<<ICES1)|(1<<CS12);
	TIMSK1 = 1<<ICIE1;
	//Inicialisação USART
	UBRR0H = (unsigned char)(MYUBRR>>8);
	UBRR0L = (unsigned char)MYUBRR;
	UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);
	UCSR0C = (3<<UCSZ00);
	//Flag de interrupções globais
	sei();
	//Inicialisação do display
	GLCD_Setup();
	GLCD_SetFont(Font5x8, 5, 8, GLCD_Overwrite);
	GLCD_InvertScreen();
	while (1)
	{
		if (a = 1)
		{
			dist = eeprom_read_byte(4);
			a = 0;
		}
		dist_aux = (((voltas)*(delta_c/100000)));
		diam = eeprom_read_byte(0);
		delta_c = (3.14*diam);
		aux = delta_c/delta_s;
		V = aux * 0.036;
		m = sprintf(DIST, "%d", dist);
		m = sprintf(RPM, "%d", rpm);
		m = sprintf(dia, "%d", diam);
		dist_cm = delta_t/58;
		m = sprintf(CM, "%d", dist_cm);
		m = sprintf(POC, "%d", porc);
		m = sprintf(TEMP, "%d", T);
		m = sprintf(VEL, "%d", V);
		esc = (0.34*porc)+50;
		switch(flag_trava)
		{
			case 0:
				if ((flag_cinto==1) & (letra==0) & (flag_trava==0))
				{
					PORTB = 0b00000110;
				}
				if ((flag_cinto == 0) | (letra!=0) | (flag_trava==1))
				{
					PORTB = 0b00000010;
				}
				break;
			case 1:
				if (flag_porta == 1)
				{
					PORTB ^= 0b00001000;
					flag_ativo = 1;
				}
				if ((flag_porta == 0) & (flag_ativo == 1))
				{
					PORTB ^= 0b00001000;
				}
				break;
		}
		switch(letra)
		{
		case 0:
			teste = 'D';
			m = sprintf(modo,"%c",teste);
			break;
		case 1:
			teste = 'R';
			m = sprintf(modo,"%c",teste);
			break;
		case 2:
			teste = 'P';
			m = sprintf(modo,"%c",teste);
			break;
		}
		if (flag_porta == 1)
		{
			switch(id_porta)
			{
				case 1:
					GLCD_Clear();
					GLCD_DrawRoundRectangle(20,15,100,45,5,255);
					GLCD_DrawRoundRectangle(40,15,90,45,5,255);
					GLCD_DrawRoundRectangle(50,20,80,40,5,255);
					GLCD_DrawLine(65,15,65,20,255);
					GLCD_DrawLine(65,45,65,40,255);
					GLCD_DrawLine(45,15,55,20,255);
					GLCD_DrawLine(45,45,55,40,255);
					GLCD_DrawLine(85,15,75,20,255);
					GLCD_DrawLine(85,45,75,40,255);
					GLCD_DrawLine(45,15,60,5,255);
					GLCD_Render();
					break;
				case 2:
					GLCD_Clear();
					GLCD_DrawRoundRectangle(20,15,100,45,5,255);
					GLCD_DrawRoundRectangle(40,15,90,45,5,255);
					GLCD_DrawRoundRectangle(50,20,80,40,5,255);
					GLCD_DrawLine(65,15,65,20,255);
					GLCD_DrawLine(65,45,65,40,255);
					GLCD_DrawLine(45,15,55,20,255);
					GLCD_DrawLine(45,45,55,40,255);
					GLCD_DrawLine(85,15,75,20,255);
					GLCD_DrawLine(85,45,75,40,255);
					GLCD_DrawLine(45,45,60,55,255);
					GLCD_Render();
					break;
				case 3:
					GLCD_Clear();
					GLCD_DrawRoundRectangle(20,15,100,45,5,255);
					GLCD_DrawRoundRectangle(40,15,90,45,5,255);
					GLCD_DrawRoundRectangle(50,20,80,40,5,255);
					GLCD_DrawLine(65,15,65,20,255);
					GLCD_DrawLine(65,45,65,40,255);
					GLCD_DrawLine(45,15,55,20,255);
					GLCD_DrawLine(45,45,55,40,255);
					GLCD_DrawLine(85,15,75,20,255);
					GLCD_DrawLine(85,45,75,40,255);
					GLCD_DrawLine(65,15,80,5,255);
					GLCD_Render();
					break;
				case 4:
					GLCD_Clear();
					GLCD_DrawRoundRectangle(20,15,100,45,5,255);
					GLCD_DrawRoundRectangle(40,15,90,45,5,255);
					GLCD_DrawRoundRectangle(50,20,80,40,5,255);
					GLCD_DrawLine(65,15,65,20,255);
					GLCD_DrawLine(65,45,65,40,255);
					GLCD_DrawLine(45,15,55,20,255);
					GLCD_DrawLine(45,45,55,40,255);
					GLCD_DrawLine(85,15,75,20,255);
					GLCD_DrawLine(85,45,75,40,255);
					GLCD_DrawLine(65,45,80,55,255);
					GLCD_Render();
					break;
				case 5:
					GLCD_Clear();
					GLCD_DrawRoundRectangle(20,15,100,45,5,255);
					GLCD_DrawRoundRectangle(40,15,90,45,5,255);
					GLCD_DrawRoundRectangle(50,20,80,40,5,255);
					GLCD_DrawLine(65,15,65,20,255);
					GLCD_DrawLine(65,45,65,40,255);
					GLCD_DrawLine(45,15,55,20,255);
					GLCD_DrawLine(45,45,55,40,255);
					GLCD_DrawLine(85,15,75,20,255);
					GLCD_DrawLine(85,45,75,40,255);
					GLCD_DrawLine(45,15,60,5,255);
					GLCD_DrawLine(45,45,60,55,255);
					GLCD_Render();
					break;
				case 6:
					GLCD_Clear();
					GLCD_DrawRoundRectangle(20,15,100,45,5,255);
					GLCD_DrawRoundRectangle(40,15,90,45,5,255);
					GLCD_DrawRoundRectangle(50,20,80,40,5,255);
					GLCD_DrawLine(65,15,65,20,255);
					GLCD_DrawLine(65,45,65,40,255);
					GLCD_DrawLine(45,15,55,20,255);
					GLCD_DrawLine(45,45,55,40,255);
					GLCD_DrawLine(85,15,75,20,255);
					GLCD_DrawLine(85,45,75,40,255);
					GLCD_DrawLine(45,15,60,5,255);
					GLCD_DrawLine(65,15,80,5,255);
					GLCD_Render();
					break;
				case 7:
					GLCD_Clear();
					GLCD_DrawRoundRectangle(20,15,100,45,5,255);
					GLCD_DrawRoundRectangle(40,15,90,45,5,255);
					GLCD_DrawRoundRectangle(50,20,80,40,5,255);
					GLCD_DrawLine(65,15,65,20,255);
					GLCD_DrawLine(65,45,65,40,255);
					GLCD_DrawLine(45,15,55,20,255);
					GLCD_DrawLine(45,45,55,40,255);
					GLCD_DrawLine(85,15,75,20,255);
					GLCD_DrawLine(85,45,75,40,255);
					GLCD_DrawLine(45,15,60,5,255);
					GLCD_DrawLine(65,45,80,55,255);
					GLCD_Render();
					break;
				case 8:
					GLCD_Clear();
					GLCD_DrawRoundRectangle(20,15,100,45,5,255);
					GLCD_DrawRoundRectangle(40,15,90,45,5,255);
					GLCD_DrawRoundRectangle(50,20,80,40,5,255);
					GLCD_DrawLine(65,15,65,20,255);
					GLCD_DrawLine(65,45,65,40,255);
					GLCD_DrawLine(45,15,55,20,255);
					GLCD_DrawLine(45,45,55,40,255);
					GLCD_DrawLine(85,15,75,20,255);
					GLCD_DrawLine(85,45,75,40,255);
					GLCD_DrawLine(45,45,60,55,255);
					GLCD_DrawLine(65,15,80,5,255);
					GLCD_Render();
					break;
				case 9:
					GLCD_Clear();
					GLCD_DrawRoundRectangle(20,15,100,45,5,255);
					GLCD_DrawRoundRectangle(40,15,90,45,5,255);
					GLCD_DrawRoundRectangle(50,20,80,40,5,255);
					GLCD_DrawLine(65,15,65,20,255);
					GLCD_DrawLine(65,45,65,40,255);
					GLCD_DrawLine(45,15,55,20,255);
					GLCD_DrawLine(45,45,55,40,255);
					GLCD_DrawLine(85,15,75,20,255);
					GLCD_DrawLine(85,45,75,40,255);
					GLCD_DrawLine(45,45,60,55,255);
					GLCD_DrawLine(65,45,80,55,255);
					GLCD_Render();
					break;
				case 10:
					GLCD_Clear();
					GLCD_DrawRoundRectangle(20,15,100,45,5,255);
					GLCD_DrawRoundRectangle(40,15,90,45,5,255);
					GLCD_DrawRoundRectangle(50,20,80,40,5,255);
					GLCD_DrawLine(65,15,65,20,255);
					GLCD_DrawLine(65,45,65,40,255);
					GLCD_DrawLine(45,15,55,20,255);
					GLCD_DrawLine(45,45,55,40,255);
					GLCD_DrawLine(85,15,75,20,255);
					GLCD_DrawLine(85,45,75,40,255);
					GLCD_DrawLine(65,15,80,5,255);
					GLCD_DrawLine(65,45,80,55,255);
					GLCD_Render();
					break;
				case 11:
					GLCD_Clear();
					GLCD_DrawRoundRectangle(20,15,100,45,5,255);
					GLCD_DrawRoundRectangle(40,15,90,45,5,255);
					GLCD_DrawRoundRectangle(50,20,80,40,5,255);
					GLCD_DrawLine(65,15,65,20,255);
					GLCD_DrawLine(65,45,65,40,255);
					GLCD_DrawLine(45,15,55,20,255);
					GLCD_DrawLine(45,45,55,40,255);
					GLCD_DrawLine(85,15,75,20,255);
					GLCD_DrawLine(85,45,75,40,255);
					GLCD_DrawLine(45,15,60,5,255);
					GLCD_DrawLine(45,45,60,55,255);
					GLCD_DrawLine(65,15,80,5,255);
					GLCD_Render();
					break;
				case 12:
					GLCD_Clear();
					GLCD_DrawRoundRectangle(20,15,100,45,5,255);
					GLCD_DrawRoundRectangle(40,15,90,45,5,255);
					GLCD_DrawRoundRectangle(50,20,80,40,5,255);
					GLCD_DrawLine(65,15,65,20,255);
					GLCD_DrawLine(65,45,65,40,255);
					GLCD_DrawLine(45,15,55,20,255);
					GLCD_DrawLine(45,45,55,40,255);
					GLCD_DrawLine(85,15,75,20,255);
					GLCD_DrawLine(85,45,75,40,255);
					GLCD_DrawLine(45,15,60,5,255);
					GLCD_DrawLine(45,45,60,55,255);
					GLCD_DrawLine(65,45,80,55,255);
					GLCD_Render();
					break;
				case 13:
					GLCD_Clear();
					GLCD_DrawRoundRectangle(20,15,100,45,5,255);
					GLCD_DrawRoundRectangle(40,15,90,45,5,255);
					GLCD_DrawRoundRectangle(50,20,80,40,5,255);
					GLCD_DrawLine(65,15,65,20,255);
					GLCD_DrawLine(65,45,65,40,255);
					GLCD_DrawLine(45,15,55,20,255);
					GLCD_DrawLine(45,45,55,40,255);
					GLCD_DrawLine(85,15,75,20,255);
					GLCD_DrawLine(85,45,75,40,255);
					GLCD_DrawLine(45,15,60,5,255);
					GLCD_DrawLine(65,15,80,5,255);
					GLCD_DrawLine(65,45,80,55,255);
					GLCD_Render();
					break;
				case 14:
					GLCD_Clear();
					GLCD_DrawRoundRectangle(20,15,100,45,5,255);
					GLCD_DrawRoundRectangle(40,15,90,45,5,255);
					GLCD_DrawRoundRectangle(50,20,80,40,5,255);
					GLCD_DrawLine(65,15,65,20,255);
					GLCD_DrawLine(65,45,65,40,255);
					GLCD_DrawLine(45,15,55,20,255);
					GLCD_DrawLine(45,45,55,40,255);
					GLCD_DrawLine(85,15,75,20,255);
					GLCD_DrawLine(85,45,75,40,255);
					GLCD_DrawLine(45,45,60,55,255);
					GLCD_DrawLine(65,15,80,5,255);
					GLCD_DrawLine(65,45,80,55,255);
					GLCD_Render();
					break;
				case 15:
					GLCD_Clear();
					GLCD_DrawRoundRectangle(20,15,100,45,5,255);
					GLCD_DrawRoundRectangle(40,15,90,45,5,255);
					GLCD_DrawRoundRectangle(50,20,80,40,5,255);
					GLCD_DrawLine(65,15,65,20,255);
					GLCD_DrawLine(65,45,65,40,255);
					GLCD_DrawLine(45,15,55,20,255);
					GLCD_DrawLine(45,45,55,40,255);
					GLCD_DrawLine(85,15,75,20,255);
					GLCD_DrawLine(85,45,75,40,255);
					GLCD_DrawLine(45,15,60,5,255);
					GLCD_DrawLine(45,45,60,55,255);
					GLCD_DrawLine(65,15,80,5,255);
					GLCD_DrawLine(65,45,80,55,255);
					GLCD_Render();
					break;
			}
		}
		else
		{
			GLCD_Clear();
			GLCD_GotoXY(0,0);
			GLCD_PrintString("LASD Car");
			GLCD_GotoXY(0,10);
			GLCD_PrintString("Diam(cm):");
			GLCD_GotoXY(60,10);
			GLCD_PrintString(dia);
			GLCD_GotoXY(0,25);
			GLCD_PrintString("RPM:");
			GLCD_GotoXY(20,25);
			GLCD_PrintString(MAX_TEMP);
			GLCD_GotoXY(50,25);
			GLCD_PrintString("Sonar:");
			GLCD_GotoXY(90,25);
			GLCD_PrintString(CM);
			GLCD_GotoXY(0,35);
			GLCD_PrintString("Velocidade:");
			GLCD_GotoXY(70,35);
			GLCD_PrintString(VEL);
			GLCD_GotoXY(100,35);
			GLCD_PrintString("km/h");
			GLCD_GotoXY(0,50);
			GLCD_PrintString(DIST);
			GLCD_GotoXY(30,50);
			GLCD_PrintString("Km");
			GLCD_GotoXY(100,50);
			GLCD_PrintString(modo);
			GLCD_GotoXY(100,0);
			GLCD_PrintString(TEMP);
			GLCD_GotoXY(120,0);
			GLCD_PrintString("C");
			GLCD_GotoXY(100,10);
			GLCD_PrintString(POC);
			GLCD_GotoXY(120,10);
			GLCD_PrintString("%");
			GLCD_DrawRectangle(50,1,85,9,255);
			GLCD_FillRectangle(85,4,87,8,255);
			GLCD_FillRectangle(50,1,esc,9,255);
			GLCD_Render();
		}
	}
}
