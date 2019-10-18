
//#include <stdio.h>
//#include <espressif/esp_common.h>
//#include <espressif/sdk_private.h>
#include <FreeRTOS.h>
#include <esp8266.h>
#include <esp/gpio.h>
#include <stdint.h>	//数据类型
#include <string.h>

#include "onewiretx.h"

onewiretx_t	ow_tx_mem[ow_tx_max]={ { 0 } };

const uint8_t	RFBitTab[8]={4,4,4,20,15,5,5,5};

static void IRAM frc1_interrupt_handler(void *arg)
{
	//125us	
	for(uint8_t i = 0; i < ow_tx_max;i ++)
	{
		if(ow_tx_mem[i].ow_tx_used == 1)
		{
			if(ow_tx_mem[i].Flg_RFSendEnd == 1)
			{	ow_tx_mem[i].RFToMs++;
				if(ow_tx_mem[i].RFToMs >= RFBitTab[ow_tx_mem[i].RFSendStep] )
				{	ow_tx_mem[i].RFToMs = 0;
					if(ow_tx_mem[i].RFSendStep < 5) ow_tx_mem[i].RFSendStep++;
					if(ow_tx_mem[i].RFSendStep == 5)
					{	ow_tx_mem[i].RFSendBit++;
						if(ow_tx_mem[i].RFSendBit == 1) gpio_write(ow_tx_mem[i].ow_tx_pin,0);
						else if(ow_tx_mem[i].RFSendBit == 2)
						{	if(ow_tx_mem[i].RFByteCont < RFSendByte)
							{	if((ow_tx_mem[i].RFByteBuf & 0x01) != 0) gpio_write(ow_tx_mem[i].ow_tx_pin,0);
								else gpio_write(ow_tx_mem[i].ow_tx_pin,1);
								ow_tx_mem[i].RFByteBuf >>= 1;
							}
							else
							{	ow_tx_mem[i].Flg_RFSendEnd = 0;
							//	MisoDelay = 12;
								gpio_write(ow_tx_mem[i].ow_tx_pin,0);
							}
						}
						else
						{	ow_tx_mem[i].RFSendBit = 0;
							gpio_write(ow_tx_mem[i].ow_tx_pin,1);
							ow_tx_mem[i].RFBitCont++;
						}
						if(ow_tx_mem[i].RFBitCont > 7)
						{	ow_tx_mem[i].RFBitCont = 0;
							ow_tx_mem[i].RFByteCont++;
							if(ow_tx_mem[i].RFByteCont < RFSendByte) ow_tx_mem[i].RFByteBuf = ow_tx_mem[i].RFBuf[ow_tx_mem[i].RFByteCont];
						}
					}
					else if(ow_tx_mem[i].RFSendStep == 4)
					{	gpio_write(ow_tx_mem[i].ow_tx_pin,1);
						ow_tx_mem[i].RFBitCont = 0;
						ow_tx_mem[i].RFByteCont = 0;
						ow_tx_mem[i].RFSendBit = 0;
						ow_tx_mem[i].RFByteBuf = ow_tx_mem[i].RFBuf[ow_tx_mem[i].RFByteCont];
					}
					else if(ow_tx_mem[i].RFSendStep == 3) gpio_write(ow_tx_mem[i].ow_tx_pin,0);
					else if(ow_tx_mem[i].RFSendStep == 2) gpio_write(ow_tx_mem[i].ow_tx_pin,1);
					else if(ow_tx_mem[i].RFSendStep == 1) gpio_write(ow_tx_mem[i].ow_tx_pin,0);
					else ow_tx_mem[i].Flg_RFSendEnd = 0;
				}
				if(ow_tx_mem[i].Flg_RFSendEnd == 0)
				{	ow_tx_mem[i].RFSendBit = 0;
					ow_tx_mem[i].RFSendStep = 0;
					gpio_write(ow_tx_mem[i].ow_tx_pin,1);
				}
			}			
		}
	}		
}

void init_frc1(void)
{
	timer_set_interrupts(FRC1, false);
	timer_set_run(FRC1, false);
	
	  /* set up ISRs */
	_xt_isr_attach(INUM_TIMER_FRC1, frc1_interrupt_handler, NULL);
	
	if(!timer_set_frequency(FRC1, frc1_interrupt_freq))
	{
		printf("Set frc1 interrupt freq %d Hz\n",frc1_interrupt_freq);
		
	//	timer_set_load(FRC1, pwmInfo._onLoad);
	//	timer_set_reload(FRC1, false);
		timer_set_interrupts(FRC1, true);
		timer_set_run(FRC1, true);	
	}
}

void create_ow_tx_gpio(uint8_t ow_tx_gpio)
{
	for(uint8_t i = 0; i < ow_tx_max;i ++)
	{
		if((ow_tx_mem[i].ow_tx_pin == ow_tx_gpio) && (ow_tx_mem[i].ow_tx_used == 1))
		{	
			printf("onewire tx: gpio %d exist!",ow_tx_gpio);
			return;
		}
		if(ow_tx_mem[i].ow_tx_used == 0)
		{	
			ow_tx_mem[i].ow_tx_used = 1;
			ow_tx_mem[i].ow_tx_pin = ow_tx_gpio;
			gpio_enable(ow_tx_gpio, GPIO_OUTPUT);	
			init_frc1();	
		}			
	}
	printf("onewire tx: gpio used > 2 !");
}

void delete_ow_tx_gpio(uint8_t ow_tx_gpio)
{	
	bool	 ow_tx_idle = 1;
	for(uint8_t i = 0; i < ow_tx_max;i ++)
	{
		if((ow_tx_mem[i].ow_tx_pin == ow_tx_gpio) && (ow_tx_mem[i].ow_tx_used == 1))
		{	
			ow_tx_mem[i].ow_tx_used = 0;
			ow_tx_mem[i].ow_tx_pin = 0;
			printf("onewire tx: delete gpio %d success!",ow_tx_gpio);
		}			
		if(ow_tx_mem[i].ow_tx_used == 1) ow_tx_idle = 0;
	}
	
	if(ow_tx_idle == 1) 		//pause timer FRC1
	{
		timer_set_interrupts(FRC1, false);
		timer_set_run(FRC1, false);
	}
}
