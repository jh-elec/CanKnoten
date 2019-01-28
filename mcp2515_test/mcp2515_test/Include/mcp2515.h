#ifndef	__MCP2515_H__
#define	__MCP2515_H__

#include <inttypes.h>
#include "mcp2515_defs.h"
#include "spi.h"

typedef struct
{
	#ifdef SUPPORT_EXTENDED_CANID
		uint32_t Identifier;
	#else
		uint16_t Identifier;
	#endif
	
	struct 
	{
		int8_t	Rtr			: 1;
		uint8_t Extended	: 2;
		uint8_t Length		: 4;
	}Header;
		
	uint8_t Data[8];
} tCAN;

typedef struct
{
	uint8_t Receive;
	uint8_t Transmit;
}tCANError;

enum cs_state_enum
{
	START_TX	= 1<<0,
	STOP_TX		= 1<<1,
	EASY_TX		= 1<<2,
	NO_ACTION	= 1<<3
};

void Mcp2515BitModify(uint8_t Adress , uint8_t Mask , uint8_t Data );

uint8_t Mcp2515ReadState(uint8_t Type);

uint8_t Mcp2515Init(uint8_t Bitrate);

uint8_t Mcp2515CheckNewMessage(void);

uint8_t Mcp2515CheckFreeBuffer(void);

uint8_t Mcp2515ReadMessage(tCAN *Message);

uint8_t Mcp2515SendMessage(tCAN *Message);

uint8_t	Mcp2515GetError( tCANError *Object );

#endif	// __MCP2515_H__
