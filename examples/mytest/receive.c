
#include <esp/gpio.h>
#include <stdint.h>
#include <espressif/esp_common.h>
#include <string.h>
#include <esplibs/libmain.h>

#include "receive.h"
#include "onewiretx.h"


rec_t	receivers[receiver_max]={ { 0 } };


int8_t find_rec_by_gpio(uint8_t rec_pin)
{
    for (uint8_t i = 0; i < receiver_max; i++)
    {    if ( receivers[i].buf_used == 1 && receivers[i].rec_pin == rec_pin) return i;
    }
    return -1;
}


// GPIO interrupt handler
void handle_rx(uint8_t gpio_num)
{	
	uint32_t hl_cnt_buf;
	int8_t receive_no = find_rec_by_gpio(gpio_num);
	if (receive_no < 0)
	{	
		printf("gpio %d no init into receiver!\n",gpio_num);
		return;
	}
	rec_t *receive = receivers + receive_no;
				     
	uint32_t system_time_stamp = 0x7FFFFFFF & sdk_system_get_time();
				
	if (system_time_stamp < receive->old_time) 
	{	
		hl_cnt_buf = system_time_stamp+0x80000000 - receive->old_time;		
		printf("time over 0x7FFFFFFF!\n");
	}
	else hl_cnt_buf = system_time_stamp - receive->old_time;
	
	receive->old_time = system_time_stamp;
			
	bool Flg_RFEr = 0;	
	if (gpio_read(receive->rec_pin))	//gpio high
	{	receive->RFLValu = hl_cnt_buf;		 
		if(receive->Flg_RFLH == 0)
		{	receive->Flg_RFLH = 1;	 
			if(receive->Flg_RFInt == 0) if((receive->RFLValu < 67*125)||(receive->RFLValu > 75*125)) Flg_RFEr = 1;				 			 
		}
	}		
	else					//gpio low
	{	receive->RFHValu = hl_cnt_buf;
		if(receive->Flg_RFLH == 1)
		{	receive->Flg_RFLH = 0;		
			if(receive->Flg_RFInt == 0)
			{	if((receive->RFHValu < 30*125)||(receive->RFHValu > 40*125)) Flg_RFEr = 1;
				else
				{	receive->Flg_RFInt = 1;
					receive->RFByteCont = 0;
					receive->RFBitCont = 0;
				}
			}
			else
			{	uint32_t ByteTimer = (receive->RFHValu + receive->RFLValu);
			//	if((ByteTimer > 12*125) && (ByteTimer < 18*125))
				{	receive->RFByteBuf >>= 1;
				//	if(receive->RFHValu > (receive->RFLValu + 2)) receive->RFByteBuf &= 0x7f;
				//	else if(receive->RFLValu > (receive->RFHValu + 2)) receive->RFByteBuf |= 0x80;
				//	else Flg_RFEr = 1;
					
					if((ByteTimer < 13*125) && (ByteTimer > 6*125))  receive->RFByteBuf &= 0x7f;
					else if((ByteTimer < 22*125) && (ByteTimer > 15*125))  receive->RFByteBuf |= 0x80;
					else Flg_RFEr = 1;
				}
			//	else Flg_RFEr = 1;
				receive->RFBitCont++;
				if(receive->RFBitCont > 7)
				{	receive->RFBitCont = 0;
					receive->RFBuf[receive->RFByteCont] = receive->RFByteBuf;
					receive->RFByteCont++;
					if(receive->RFByteCont >= RFResiveByte)
					{	Flg_RFEr = 1;
						receive->Flg_RFEd = 1;
					}
				}
			}
		}
	}
	
	if(Flg_RFEr == 1) receive->Flg_RFInt = 0;					
}


void create_ir_gpio(uint8_t gpio_num)
{	
	if(find_rec_by_gpio(gpio_num) < 0)
	{	
		for (uint8_t i = 0; i < receiver_max; i++)
		{    	
			if( receivers[i].buf_used == 0 )
			{	
				receivers[i].buf_used = 1;	
				receivers[i].rec_pin = gpio_num;
				gpio_set_pullup(gpio_num, true, true);
				gpio_set_interrupt(gpio_num, GPIO_INTTYPE_EDGE_ANY, handle_rx);
			}
		}
	}
	else printf("gpio %d existing!\n",gpio_num);	
}

int8_t delete_ir_gpio(uint8_t gpio_num)
{		
	uint8_t	ir_no = find_rec_by_gpio(gpio_num);	
	if (ir_no < 0)
	{	
		printf("delete ir gpio %d no exist!\n",gpio_num);
		return -1;
	}		
	else
	{	   	
		if ( receivers[ir_no].buf_used == 1 && receivers[ir_no].rec_pin == gpio_num )
		{	
			receivers[ir_no].buf_used = 0;	
			receivers[ir_no].rec_pin = 0;
			printf("delete ir gpio %d success!\n",gpio_num);			
		}
		return ir_no;
	}
}

