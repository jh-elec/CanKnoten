/* Generated by CodeDescriptor 1.5.0.0907 */
/*
* Project Name      -> AVR - SPI Treiber
* Version           -> 1.0.0.0621
* Author            -> Hm @ Workstadion.: QP-01-02
* Build Date        -> 21.06.2018 06:48:20
* Description       -> Description
*
*
*
*/
#include <avr/io.h>
#include "../Include/spi.h"


uint8_t spiInit( enum spiPrescaler divisor , enum spiClockOptions clock_option , enum spiDoubleSpeed doubleSpeed)
{		
	/*
	*	Datenleitungen ( MISO , MOSI , SCK , SS ) konfigurieren
	*/	
	SPI_DDR		|=  ( 1 << SPI_MOSI_bp);
	SPI_DDR		&= ~( 1 << SPI_MISO_bp);
	SPI_DDR		|=  ( 1 << SPI_SCK_bp );
	SPI_SS_DDR	|=  ( 1 << SPI_SS_bp  );
		
	/*
	*	SPI aktivieren und in den "Master Mode" setzen
	*/
	SPCR |= ( ( 1 << SPE ) | ( 1 << MSTR ) );
		
	/*
	*	Takt konfigurieren und die Polarit�t des Taktes einstellen
	*/
	switch( clock_option )
	{
		case MODE1: 
		{
			SPCR &= ~( ( 1<<CPOL ) | ( 1<<CPHA ) );	
		}break;
		
		case MODE2: 
		{
			SPCR |=  ( 1<<CPHA );				
		}break;
		
		case MODE3: 
		{
			SPCR |=  ( 1<<CPOL );				
		}break;
		
		case MODE4: 
		{
			SPCR |=  ( ( 1<<CPOL ) | ( 1<<CPHA ) );	
		}break;
	}
		
	/*
	*	Bus Frequenz einstellen
	*/
	switch ( divisor )
	{
		/*
		*	Vorteiler ohne "DoubleSpeed"
		*/
		case PRESCALER_4:
		{
			SPCR &= ~( ( 1<<SPR1 ) | ( 1<<SPR0 ) );
		}break;
		
		case PRESCALER_16:	
		{
			SPCR |=  ( 1<<SPR0 ); 	
		}break;
		
		case PRESCALER_64:	
		{
			SPCR |=  ( 1<<SPR1 ); 
		}break;
		
		case PRESCALER_128:	
		{
			SPCR |=  ( ( 1<<SPR1 ) | ( 1<<SPR0 ) );				
		}break;
		
		/*
		*	Vorteiler mit "DoubleSpeed"
		*/		
		case SPI2X_PRESCALER_2:
		{
			SPCR &= ~( ( 1<<SPR1 ) | ( 1<<SPR0 ) );
		}break;
		
		case SPI2X_PRESCALER_8:
		{
			SPCR |= ( 1<<SPR0 );	
		}break;

		case SPI2X_PRESCALER_32:
		{
			SPCR |= ( 1<<SPR1 );
		}break;

		case SPI2X_PRESCALER_64:
		{
			SPCR |= ( ( 1<<SPR1 ) | ( 1<<SPR0 ) );
		}break;
	}
	
	switch( doubleSpeed )
	{
		case DOUBLE_SPEED_OFF:
		{
			SPSR &= ~( 1<<SPI2X );
		}break;
		
		case DOUBLE_SPEED_ON:
		{
			SPSR |= ( 1<<SPI2X );
		}break;
	}
	
	return 0;	
}

void spiWrite( uint8_t data )		
{
	/*
	*	Byte in das Senderegister vom SPI schreiben
	*/
	SPDR = data;
	
	/*
	*	Warten bis alle Bits raus geschoben wurden sind..
	*/
	while( ! ( SPSR & ( 1 << SPIF ) ) );	
}

uint8_t spiRead( uint8_t data )				
{	
	/*
	*	Dummy Byte senden
	*/
	SPDR = data;
	
	/*
	*	Warten bis alle Bits raus geschoben wurden sind..
	*/
	while( ! ( SPSR & ( 1 << SPIF ) ) );

	/*
	*	Empfangenes Byte zur�ckgeben
	*/
	return SPDR;	
}

uint8_t spiWriteRead( uint8_t data )
{
	/*
	*	Byte in das Senderegister vom SPI schreiben
	*/
	SPDR = data;
	
	/*
	*	Warten bis alle Bits raus geschoben wurden sind..
	*/
	while( ! ( SPSR & ( 1 << SPIF ) ) );	
	
	/*
	*	Empfangenes Byte zur�ckgeben
	*/
	return SPDR;
}

#ifdef SPI_CS_ENABLE

void spiSlaveSelect( void )			
{
	SPI_SS_PORT &= ~ ( 1 << SPI_SS_bp );
}

void spiSlaveDeSelect( void )		
{
	SPI_SS_PORT |= ( 1 << SPI_SS_bp );
}

#endif






