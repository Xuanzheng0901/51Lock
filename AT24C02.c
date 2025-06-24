#include <REGX52.H>
#include "I2C.h"

#define AT24C02_ADDRESS 0xA0

void eepromWritePage(unsigned char addr, unsigned char* Data, unsigned char length)
{
	unsigned char i;
	iicStart();
	iicSendByte(AT24C02_ADDRESS);
	iicRecvAck();
	iicSendByte(addr);
	iicRecvAck();
	for(i = 0; i < length; i++)
	{
		iicSendByte(Data[i]);
		iicRecvAck();
	}
	iicStop();
}

void eepromWriteOneData(unsigned char WordAddress, unsigned char Data)
{
	iicStart();
	iicSendByte(AT24C02_ADDRESS);
	iicRecvAck();
	iicSendByte(WordAddress);
	iicRecvAck();
	iicSendByte(Data);
	iicRecvAck();
	iicStop();
}

unsigned char eepromRead(unsigned char WordAddress)
{
	unsigned char Data;
	iicStart();
	iicSendByte(AT24C02_ADDRESS);
	iicRecvAck();
	iicSendByte(WordAddress);
	iicRecvAck();
	iicStart();
	iicSendByte(AT24C02_ADDRESS | 0x01);
	iicRecvAck();
	Data = iicRecvByte();
	iicSendAck(1);
	iicStop();
	return Data;
}