
#define F_CPU	16000000

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "../Include/mcp2515.h"
#include "../Include/mcp2515_defs.h"


const uint8_t Mcp2515SpeedCnfg[8][3] = 
{
	// 10 kbps
	{	0x04,
		0xb6,
		0xe7
	},
	
	// 20 kbps
	{	0x04,
		0xb6,
		0xd3
	},
	
	// 50 kbps
	{	0x04,
		0xb6,
		0xc7
	},
	
	// 100 kbps
	{	0x04,
		0xb6,
		0xc3
	},
	
	// 125 kbps
	{	(1<<PHSEG21),					// CNF3
		(1<<BTLMODE)|(1<<PHSEG11),		// CNF2
		(1<<BRP2)|(1<<BRP1)|(1<<BRP0)	// CNF1
	},
	
	// 250 kbps
	{	
		0x03,
		0xac,
		0x81
	},
	
	// 500 kbps
	{	0x03,
		0xac,
		0x80
	},
	
	// 1 Mbps
	{	(1<<PHSEG21),
		(1<<BTLMODE)|(1<<PHSEG11),
		0
	}
};

const uint8_t Mcp2515Modes[] =
{
	(0<<REQOP0 | 0<<REQOP1 | 0<<REQOP2), // Normal Mode
	(1<<REQOP0), // Sleep Mode
	(1<<REQOP1), // Loopback Mode
	(1<<REQOP0 | 1<<REQOP1), // Listen only Mode
	(1<<REQOP2) // Configuration Mode
};


static uint8_t		Mcp2515Write( uint8_t *Frame , uint8_t Lenght , enum cs_state_enum State )
{
	if ( State == EASY_TX || State == START_TX )
	{
		MCP2515_CS_LOW;
	}
	
	for( uint8_t i = 0 ; i < Lenght ; i++ )
	{
		spiWrite(Frame[i]);
	}
	
	if ( State == STOP_TX || State == EASY_TX )
	{
		MCP2515_CS_HIGH;
	}
	
	return 0; // Alles inordnung
}

static uint8_t		Mcp2515Read( uint8_t *Address , uint8_t AddrLength , uint8_t *Buffer , uint8_t RxLength , enum cs_state_enum State )
{

	if ( State == START_TX || State == EASY_TX )
	{
		MCP2515_CS_LOW;
	}
	
	for ( uint8_t i = 0 ; i < AddrLength ; i++ )
	{
		spiWrite(Address[i]);
	}
	
	for( uint8_t x = 0 ; x < RxLength ; x++ )
	{
		Buffer[x] = spiRead(0xFF);
	}
	
	if ( State == STOP_TX || State == EASY_TX )
	{
		MCP2515_CS_HIGH;
	}
	
	return 0; // Alles inordnung
}


#if SUPPORT_EXTENDED_CANID
	void Mcp2515WriteID( const uint32_t *Identifier , bool Extended , enum cs_state_enum State )
{
	uint8_t tmp;
	if ( Extended ) 
	{		
		// naechsten Werte berechnen
		tmp  = (*((uint8_t *) Identifier + 2) << 3) & 0xe0;
		tmp |= (1 << IDE);
		tmp |= (*((uint8_t *) Identifier + 2)) & 0x03;
		
		uint8_t Frame[] = { *((uint16_t *) Identifier + 1) >> 5 , tmp , *((uint8_t *) Identifier + 1) , *((uint8_t *) Identifier) };
		Mcp2515Write( Frame , sizeof(Frame) , State );
	}
	else 
	{
		
		uint8_t Frame[] = { *((uint16_t *) Identifier) >> 3 , *((uint8_t *) Identifier) << 5 , 0 , 0 };
		Mcp2515Write( Frame , sizeof(Frame) , State );
	}
}
#else
	void Mcp2515WriteID( const uint16_t *Identifier , enum cs_state_enum State )
{
	uint8_t Frame[] = { ((*Identifier)>>3) , ((*Identifier)<<5) , 0 , 0 };

	Mcp2515Write( Frame , sizeof(Frame) , NO_ACTION );
}
#endif



void		Mcp2515BitModify( uint8_t Adress , uint8_t Mask , uint8_t Data )
{	
	MCP2515_CS_LOW;
	
	uint8_t Frame[] = { SPI_BIT_MODIFY , Adress , Mask , Data };
	
	Mcp2515Write(Frame,sizeof(Frame),EASY_TX);
	
	MCP2515_CS_HIGH;
}

uint8_t		Mcp2515ReadState( uint8_t Type )
{
	uint8_t Data;
	
	Mcp2515Read( &Type , 1 , &Data , 1 , EASY_TX );
	
	return Data;
}

uint8_t		Mcp2515Init( enum mcp2515_bitrate Bitrate )
{
	if( Bitrate >= 8 )
	{
		return 1;
	}
	
	MCP2515_INT_DDR	|= (1<<MCP2515_INT_BP);
	MCP2515_CS_DDR  |= (1<<MCP2515_CS_BP);
		
	// reset MCP2515 by software reset.
	// After this he is in configuration mode.

	Mcp2515Write( (uint8_t*) SPI_RESET , 1 , EASY_TX );
	
	_delay_ms(10);
	
	uint8_t Frame[] = { SPI_WRITE , CNF3 , 0 , 0 , 0 , MCP2515_INTERRUPTS }; // 0 = Platzhalter für Timinig Werte
	
	for( uint8_t i = 0 ; i < 3 ; i++ )
	{
		Frame[2+i] = Mcp2515SpeedCnfg[Bitrate][i];
	}
	
	Mcp2515Write(Frame,sizeof(Frame),EASY_TX); // Init schreiben
		
	// TXnRTS Bits als Inputs schalten
	Frame[0] = SPI_WRITE;
	Frame[1] = TXRTSCTRL;
	Frame[2] = 0;
	
	Mcp2515Write(Frame,3,EASY_TX);
			
	// Device zurueck in den normalen Modus versetzten
	// und aktivieren/deaktivieren des Clkout-Pins
	// Frame[0] sollte hier noch "SPI_WRITE" beeinhalten
	Frame[1] = CANCTRL;
	Frame[2] = (CLKOUT_PRESCALER_ | CLKEN);
	Mcp2515Write(Frame,3,EASY_TX);
	
	
	uint8_t Result = 0;
	uint32_t Timeout = 10e6;
	while ( (Result & 0xE0) != 0 && Timeout-- > 0 ) 
	{
		// warten bis der neue Modus uebernommen wurde
		uint8_t Address[] = { SPI_READ , CANSTAT };
		Mcp2515Read( Address , sizeof(Address) , &Result , 1 , EASY_TX);
	}
	
	if(Timeout == 0) 
	{
		return 2;
	}
		
	return 0;
}

uint8_t		Mcp2515CheckNewMessage( void ) 
{
	/* INT Pin auswerten */
	return (!((MCP2515_INT_PORT) & (1<<MCP2515_INT_BP)));
}

uint8_t		Mcp2515CheckFreeBuffer( void )
{
	uint8_t Status = Mcp2515ReadState(SPI_READ_STATUS);
	
	if ((Status & 0x54) == 0x54) 
	{
		// Alle Puffer sind in benutzung..
		return 0;
	}
	
	return 1;// Mindestens 1 Puffer ist frei..
}

uint8_t		Mcp2515ReadMessage( tCAN *Message )
{	
	uint8_t Status = Mcp2515ReadState(SPI_RX_STATUS);
	uint8_t Address;
	
	if( ((Status & 1<<6)) ) 
	{
		// Nachricht in Puffer 0
		Address = SPI_READ_RX;
	}
	else if( ((Status & 1<<7)) ) 
	{
		// Nachricht in Puffer 1
		Address = (SPI_READ_RX | 0x04);
	}
	else 
	{
		// Error: Keine neue Nachricht vorhanden
		return 0;
	}
	
	Mcp2515Write( &Address , 1 , START_TX );
	
	uint8_t Buff[] = { 0 , 0 };
	Mcp2515Read( 0 , 0 , Buff , 2 , NO_ACTION );
	
	// Empfangenen Nachrichten "Idetifier" lesen
	Message->Identifier  = (uint16_t) Buff[0] << 3;
	Message->Identifier |= (uint16_t) Buff[1] >> 5;
	
	// Dummy Bytes senden um internen Adresszeiger beim MCP2515 zu inkrementieren
	Buff[0] = 0xFF; Buff[1] = 0xFF;
	Mcp2515Write( Buff , 2 , NO_ACTION );
	
	// DLC lesen
	Mcp2515Read( 0 , 0 , Buff , 1 , NO_ACTION );
	
	uint8_t Length = Buff[0] & 0x0F;
	Message->Header.Length = Length;
	
	Message->Header.Rtr = ( (Status & 1<<3) == 1) ? 1 : 0;
	
	// Daten abholen
	Mcp2515Read( 0 , 0 , Message->Data , Length , NO_ACTION );
		
	// Lösche Interrupt Flags
	if ((Status & 1<<6) == 1) 
	{
		Mcp2515BitModify(CANINTF , (1<<RX0IF) , 0 );
	}
	else 
	{
		Mcp2515BitModify(CANINTF , (1<<RX1IF) , 0 );
	}
	
	return (Status & 0x07) + 1;
}

uint8_t		Mcp2515SendMessage( tCAN *Message )
{
	uint8_t Status = Mcp2515ReadState(SPI_READ_STATUS);
	 
	uint8_t Address;
	
	if (((Status & 1<<2) == 0)) 
	{
		Address = 0x00;
	}
	else if (((Status & 1<<4) == 0)) 
	{
		Address = 0x02;
	} 
	else if (((Status & 1<<6) == 0)) 
	{
		Address = 0x04;
	}
	else 
	{
		/* Alle Puffer sind in benutzung, es kann gerade keine 
		*  Nachricht gesendet werden 
		*/		
		return 0;
	}
	
	uint8_t BuffAddress = SPI_WRITE_TX | Address;
	Mcp2515Write( &BuffAddress , 1 , START_TX );
	
	#if SUPPORT_EXTENDED_CANID
		Mcp2515WriteID( &Message->Identifier , Message->Header.Extended , NO_ACTION );
	#else
		Mcp2515WriteID( &Message->Identifier , NO_ACTION );
	#endif

	uint8_t Length = Message->Header.Length & 0x0F;
	
 	// Ist die Nachricht ein "Remote Transmit Request" ?
 	if ( Message->Header.Rtr )
 	{
 		// Ein RTR hat zwar eine Laenge,
 		// enthaelt aber keine Daten

 		// Nachrichten Laenge + RTR einstellen
		Length |= 1<<RTR;
 		Mcp2515Write( &Length , 1 , STOP_TX );
 	}
 	else
 	{
 		// Nachrichten Laenge einstellen
		Mcp2515Write( &Length , 1 , NO_ACTION );
 		
		// Daten
		Mcp2515Write( Message->Data , Length , STOP_TX );
	}
	
 	// CAN Nachricht verschicken
 	// die letzten drei Bit im RTS Kommando geben an welcher
 	// Puffer gesendet werden soll.
 	Address = (Address == 0) ? 1 : Address;
 
	Address |= SPI_RTS;

	Mcp2515Write( &Address , 1 , EASY_TX ); 
	
	return Address;
}

uint8_t		Mcp2515GetError( tCANError *Object )
{
	uint8_t Data[] = { 0x00 , 0x00 };
	
	/*
	*	0: Fehlerspeicher für TEC (Transmit Error Counter)
	*	1: Fehlerspeicher für REC (Receive Error Counter)
	*/
	Mcp2515Read( (uint8_t*)(TEC) , 1 , Data , 2 , EASY_TX );
	Object->Transmit = Data[0];
	Object->Receive  = Data[1];

	return 0;
}

