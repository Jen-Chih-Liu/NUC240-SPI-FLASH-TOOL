#include <stdio.h>
#include "NUC230_240.h"
#include "hid_transfer.h"
#include "SPI_FLASH.h"

#define CMD_UPDATE_SPI_FLASH			0x000000A0
#define CMD_READ_SPI_FLASH			0x000000A1
#define CMD_ERASE_ALL				0x000000A3
#define CMD_GET_FWVER				0x000000A6
#define CMD_GET_DEVICEID			0x000000B1
#define FW_VERSION     0X01234567
extern uint8_t volatile g_u8EP3Ready;
uint32_t u32_cmd;
uint32_t u32_pid;
uint32_t u32_cal_checksum;
uint32_t u32_buf_checksum;
uint8_t out_buffer[64];
uint8_t in_buffer[64];
uint32_t temp,i;
unsigned char write_array[64];
unsigned char read_array[64];

uint32_t Checksum(unsigned char *buf, int len)
{
    int i;
    uint32_t c;

    for(c = 0 , i = 0 ; i < len; i++) {
        c += buf[i];
    }

    return (c);
}

void MemCopy(uint8_t *dest, uint8_t *src, int32_t size)
{
	while (size--) *dest++ = *src++;
}

void USB_Process(void)
{
uint8_t *ptr_USB_OUT_Buffer;
uint8_t *ptr_USB_IN_Buffer;
 	uint32_t address;
		uint8_t temp_cnt;
      ptr_USB_OUT_Buffer = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP3));		  
	  ptr_USB_IN_Buffer = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2));        
while(1){		
		if(g_u8EP3Ready==1)
		  {	
     g_u8EP3Ready=0;				
      
	    MemCopy(out_buffer, ptr_USB_OUT_Buffer, 64);
		u32_cmd = inpw(out_buffer);
		u32_pid = inpw(out_buffer + 4);
		u32_buf_checksum = inpw(out_buffer + (4 * (16 - 1)));
		u32_cal_checksum = Checksum(out_buffer, 60);
      
			//clear buffer
			for(temp_cnt=0;temp_cnt<64;temp_cnt++)
				in_buffer[temp_cnt] = 0x00;
				
			if(u32_buf_checksum!=u32_cal_checksum)			
			{			
				goto EXIT;
			}
      //COMMAND read fw version
			if(u32_cmd==CMD_GET_FWVER)
			{	
				outpw(in_buffer, 0xaaaaaaaa);//0xaaaaaaaa is ack
				outpw(in_buffer + 8, FW_VERSION);
			}
			
			
			
			//COMMAND erase all
			if(u32_cmd==CMD_ERASE_ALL)
			{	
				outpw(in_buffer, 0xaaaaaaaa);//0xaaaaaaaa is ack
				//erase spi flash
				SpiFlash_ChipErase();
        /* Wait ready */
        SpiFlash_WaitReady();
			}
			
			//COMMAND get device id
			if(u32_cmd==CMD_GET_DEVICEID)
			{	
				   outpw(in_buffer, 0xaaaaaaaa);//0xaaaaaaaa is ack			
           outpw(in_buffer + 8, SpiFlash_ReadMidDid());
			}
			
									
			if(u32_cmd==CMD_UPDATE_SPI_FLASH)
			{		
      address = inpw(out_buffer+8);	  		
      for (temp_cnt = 0; temp_cnt < 48; temp_cnt++)
	    {
		  write_array[temp_cnt] = out_buffer[temp_cnt+12];
	    }
			SpiFlash_PageProgram(write_array, address, 48);
      SpiFlash_WaitReady();
			SpiFlash_ReadData(read_array, address, 48);
      for (temp_cnt = 0; temp_cnt < 48; temp_cnt++)
	    {
		  if(write_array[temp_cnt] != read_array[+temp_cnt])
				goto EXIT;
	    }			
			
				outpw(in_buffer, 0xaaaaaaaa);//0xaaaaaaaa is ack							
			}
			
			
			if(u32_cmd==CMD_READ_SPI_FLASH)
			{		
      address = inpw(out_buffer+8);	  		      			
			SpiFlash_ReadData(read_array, address, 48);
	    outpw(in_buffer+8,address);	  
			 for (temp_cnt = 0; temp_cnt < 48; temp_cnt++)
	    {
		  in_buffer[temp_cnt+12]= read_array[temp_cnt];
	    }
				
			outpw(in_buffer, 0xaaaaaaaa);//0xaaaaaaaa is ack							
			}
			
			
           //ACK PACKAGE ID +1
			outpw(in_buffer + 4, (u32_pid + 1));
			
			//ACK CHECK SUM
			u32_buf_checksum = Checksum(in_buffer, 60);
			outpw(in_buffer + (4 * (16 - 1)), u32_buf_checksum);
			
      EXIT:		
u32_cmd=0;
MemCopy(ptr_USB_IN_Buffer, in_buffer, 64);
      //ACK IN BUFFER			
			USBD_SET_PAYLOAD_LEN(EP2, EP2_MAX_PKT_SIZE);
			USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);	
			}
		}
}



