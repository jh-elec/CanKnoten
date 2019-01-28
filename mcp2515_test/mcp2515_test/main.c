/*
 * MCP2515_Test.c
 *
 * Created: 25.01.2019 16:31:01
 * Author : Jan Homann
 */ 

#ifndef F_CPU	
#define F_CPU 16000000
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "Include/mcp2515.h"
#include "Include/mcp2515_defs.h"

#define SUMMER_DDR	DDRD
#define SUMMER_PORT	PORTD
#define SUMMER_BP	PD1

#define LED_DDR		DDRD
#define LED_PORT	PORTD
#define LED_BP		PD0

#define SUMMER_ON	SUMMER_PORT |= (1<<SUMMER_BP)
#define SUMMER_OFF  SUMMER_PORT &= ~(1<<SUMMER_BP)
#define SUMMER_TGL	SUMMER_PORT ^= (1<<SUMMER_BP)

#define LED_ON		LED_PORT	|= (1<<LED_BP)
#define LED_OFF		LED_PORT	&= ~(1<<LED_BP)
#define LED_TGL		LED_PORT	^= (1<<LED_BP)

int main(void)
{
	DDRD = 0xFF;

	PORTD = 0xFF;
	SUMMER_OFF;
	LED_ON;
	
    spiInit( PRESCALER_128  , MODE4 , DOUBLE_SPEED_OFF );
	Mcp2515Init(MCP2515_250KBPS);

    while (1) 
    {
		
		static tCAN RxMessage , TxMessage;
		
		static tCANError ErrorMessage;
		
		Mcp2515GetError( &ErrorMessage );
		
 		TxMessage.Identifier = 0x1;
 		TxMessage.Header.Length = 2;
		TxMessage.Data[0] = ErrorMessage.Receive;
		TxMessage.Data[1] = ErrorMessage.Transmit;
 		Mcp2515SendMessage( &TxMessage );
		
		Mcp2515ReadMessage( &RxMessage );
		
		switch( RxMessage.Identifier )
		{
			case 0x1:
			{
				for (uint8_t x = 0 ; x < 3 ; x++ )
				{
 					SUMMER_ON;
 					_delay_ms(25);
 					SUMMER_OFF;
 					_delay_ms(125);
				}					
			}break;
			
			case 0x2:
			{
				LED_TGL;	
			}break;
			
			case 0x3:
			{
				
			}break;
		}
		RxMessage.Identifier = 0;

    }
}

