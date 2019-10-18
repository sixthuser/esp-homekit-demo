
#pragma once

#define		ow_tx_max			2	
#define		ow_tx_gpio_a			14	//NodeMCU-D5
#define		frc1_interrupt_freq		8000	//HZ
#define		RFSendByte			6

typedef struct
{	
	bool	ow_tx_used;
	bool	Flg_RFLH,Flg_RFSendEnd;	
	uint8_t	ow_tx_pin;
	uint8_t	RFBitCont,RFByteCont,RFByteBuf,RFToMs,RFSendBit,RFSendStep;
	uint8_t	RFBuf[RFSendByte];
}onewiretx_t;

extern	onewiretx_t	ow_tx_mem[ow_tx_max];

void create_ow_tx_gpio(uint8_t ow_tx_gpio);

void delete_ow_tx_gpio(uint8_t ow_tx_gpio);