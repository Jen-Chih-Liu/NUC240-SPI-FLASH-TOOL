// USB_SPI_AP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "HID.hpp"
#include <io.h>
#include <fcntl.h>

#define USB_VID        0x0416
#define USB_PID        0x5020
#define USB_TIME_OUT    100

#define CMD_UPDATE_SPI_FLASH			0x000000A0
#define CMD_READ_SPI_FLASH			0x000000A1
#define CMD_ERASE_ALL				0x000000A3
#define CMD_GET_FWVER				0x000000A6
#define CMD_GET_DEVICEID			0x000000B1
#define outpw(port,value)     *((volatile unsigned int *)(port)) = value
#define inpw(port)            (*((volatile unsigned int *)(port)))
unsigned char out_buff[64];
unsigned char in_buff[64];

unsigned int  u32_pid;
unsigned int buff_checksum, cal_checksum;
unsigned int ack, pid;

unsigned int USB_PACKAGE(int sleeptime)
{
	CHidCmd io;
	unsigned long length;
	BOOL bRet;
	bool isDeviceOpened;

	isDeviceOpened = 0;
	if (!io.OpenDevice(USB_VID, USB_PID))
	{
		printf("Can't Open HID Device\n");
		goto lexit;
	}
	else
	{
		isDeviceOpened = TRUE;
		bRet = io.WriteFile((unsigned char *)&out_buff, sizeof(out_buff), &length, USB_TIME_OUT);
		if (!bRet)
		{
			printf("ERROR: Send test command error!\n");
			goto lexit;
		}
		Sleep(sleeptime);

		bRet = io.ReadFile((unsigned char *)&in_buff, sizeof(in_buff), &length, USB_TIME_OUT);
		if (!bRet)
		{
			printf("ERROR: Read fail!\n");
			goto lexit;
		}


	}


lexit:

	if (isDeviceOpened)
		io.CloseDevice();
	if (isDeviceOpened == TRUE)
		return 1;
	else
		return 0;
}



unsigned int Checksum(unsigned char *buf, int len)
{
	int i;
	unsigned int c;

	for (c = 0, i = 0; i < len; i++) {
		c += buf[i];
	}

	return (c);
}

unsigned int FW_version(void)
{
loop:
	for (int i = 0; i < 64; i++)
		out_buff[i] = 0;

	outpw(out_buff, CMD_GET_FWVER);
	outpw(out_buff + 4, u32_pid);
	buff_checksum = Checksum(out_buff, 60);
	outpw(out_buff + (4 * (16 - 1)), buff_checksum);
	if (USB_PACKAGE(0) == 0)
	{
		return 0;
	}
	ack = inpw(in_buff);
	if (ack != 0xaaaaaaaa)
		goto loop;
	pid = inpw(in_buff + 4);
	buff_checksum = inpw(in_buff + (4 * (16 - 1)));
	cal_checksum = Checksum(in_buff, 60);
	if (cal_checksum != buff_checksum)
	{
		return 0;
	}

	if (pid != u32_pid + 1)
	{
		goto loop;
	}
	u32_pid = u32_pid + 2;
	return inpw(in_buff + 8);
}


unsigned int TARGET_DEVICE_ID(void)
{
loop:
	for (int i = 0; i < 64; i++)
		out_buff[i] = 0;

	outpw(out_buff, CMD_GET_DEVICEID);
	outpw(out_buff + 4, u32_pid);
	buff_checksum = Checksum(out_buff, 60);
	outpw(out_buff + (4 * (16 - 1)), buff_checksum); //WRITE CHECK SUM TO BUFFER
	if (USB_PACKAGE(0) == 0)
	{
		return 0;
	}
	ack = inpw(in_buff);
	if (ack != 0xaaaaaaaa)
		goto loop;
	pid = inpw(in_buff + 4);
	buff_checksum = inpw(in_buff + (4 * (16 - 1)));
	cal_checksum = Checksum(in_buff, 60);
	if (cal_checksum != buff_checksum)
	{
		return 0;
	}

	if (pid != u32_pid + 1)
	{
		goto loop;
	}
	u32_pid = u32_pid + 2;
	return inpw(in_buff + 8);
}

unsigned int ERASE_CHIP(void)
{
loop:
	for (int i = 0; i < 64; i++)
		out_buff[i] = 0;

	outpw(out_buff, CMD_ERASE_ALL);
	outpw(out_buff + 4, u32_pid);
	buff_checksum = Checksum(out_buff, 60);
	outpw(out_buff + (4 * (16 - 1)), buff_checksum); //WRITE CHECK SUM TO BUFFER
	if (USB_PACKAGE(200) == 0)
	{
		return 0;
	}
	ack = inpw(in_buff);
	if (ack != 0xaaaaaaaa)
		goto loop;
	pid = inpw(in_buff + 4);
	buff_checksum = inpw(in_buff + (4 * (16 - 1)));
	cal_checksum = Checksum(in_buff, 60);
	if (cal_checksum != buff_checksum)
	{
		return 0;
	}

	if (pid != u32_pid + 1)
	{
		goto loop;
	}
	u32_pid = u32_pid + 2;
	return 1;
}

unsigned int PROGRAM_FLASH(unsigned int address, unsigned char * u8Data)
{
loop:
	for (int i = 0; i < 64; i++)
		out_buff[i] = 0;

	outpw(out_buff, CMD_UPDATE_SPI_FLASH);
	outpw(out_buff + 4, u32_pid);
	outpw(out_buff + 8, address);
	for (int i = 0; i < 48; i++)
	{
		out_buff[12 + i] = u8Data[i];
	}

	buff_checksum = Checksum(out_buff, 60);
	outpw(out_buff + (4 * (16 - 1)), buff_checksum); //WRITE CHECK SUM TO BUFFER
	if (USB_PACKAGE(0) == 0)
	{
		return 0;
	}
	ack = inpw(in_buff);
	if (ack != 0xaaaaaaaa)
		goto loop;
	pid = inpw(in_buff + 4);
	buff_checksum = inpw(in_buff + (4 * (16 - 1)));
	cal_checksum = Checksum(in_buff, 60);
	if (cal_checksum != buff_checksum)
	{
		return 0;
	}

	if (pid != u32_pid + 1)
	{
		goto loop;
	}
	u32_pid = u32_pid + 2;
	return 1;
}

unsigned int Read_FLASH(unsigned int address, unsigned char * u8Data)
{
loop:
	for (int i = 0; i < 64; i++)
		out_buff[i] = 0;

	outpw(out_buff, CMD_READ_SPI_FLASH);
	outpw(out_buff + 4, u32_pid);
	outpw(out_buff + 8, address);
	buff_checksum = Checksum(out_buff, 60);
	outpw(out_buff + (4 * (16 - 1)), buff_checksum); //WRITE CHECK SUM TO BUFFER
	if (USB_PACKAGE(0) == 0)
	{
		return 0;
	}
	ack = inpw(in_buff);
	if (ack != 0xaaaaaaaa)
		goto loop;
	pid = inpw(in_buff + 4);
	buff_checksum = inpw(in_buff + (4 * (16 - 1)));
	cal_checksum = Checksum(in_buff, 60);
	if (cal_checksum != buff_checksum)
	{
		return 0;
	}

	for (int i = 0; i < 48; i++)
	{
		u8Data[i] = in_buff[12 + i];
	}


	if (pid != u32_pid + 1)
	{
		goto loop;
	}
	u32_pid = u32_pid + 2;
	return 1;
}



unsigned char flash_write_buffer[48];
unsigned char flash_read_buffer[48];
int _tmain(int argc, _TCHAR* argv[])
{
	int temp_cnt;
	if (USB_PACKAGE(0) == 0)
	{
		printf("the usb cann't normal work!!");
	}
	printf("The SPI Flash Process!!!\n\r");
	printf("FW version:0x%x\n\r", FW_version());
	printf("SPI FLASH ID:0x%x\n\r", TARGET_DEVICE_ID());
	
	if (ERASE_CHIP() == 1)
		printf("erase pass\n\r");
	else
		printf("erase false\n\r");
	
	Read_FLASH(0, flash_read_buffer);

	for (temp_cnt = 0; temp_cnt < 48; temp_cnt++)
		flash_write_buffer[temp_cnt] = temp_cnt;
	
	if (PROGRAM_FLASH(0, flash_write_buffer) == 1)
		printf("Program pass\n\r");
	else
		printf("Program false\n\r");

	Read_FLASH(0, flash_read_buffer);

	return 0;
}

