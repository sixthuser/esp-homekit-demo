#pragma once

#define		receive_gpio_a		5	//NodeMCU - D1

#define		receiver_max		2

#define		rec_buf_length		20

#define		RFResiveByte		4


typedef	struct
{
	uint8_t		rec_pin;
	bool		buf_used;
	bool		Flg_RFLH,Flg_RFInt,Flg_RFEd;
	uint32_t	old_time;	
	uint32_t	RFHValu,RFLValu;
	uint8_t		RFBitCont,RFByteCont,RFByteBuf;	
	uint8_t		RFBuf[rec_buf_length];
	
} rec_t;

extern	rec_t	receivers[receiver_max];

int8_t find_rec_by_gpio(uint8_t rec_pin);

void handle_rx(uint8_t gpio_num);

void create_ir_gpio(uint8_t gpio_num);

int8_t delete_ir_gpio(uint8_t gpio_num);

//void receive_task(void *_args) ;