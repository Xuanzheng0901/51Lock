#include <REGX52.H>
#include "I2C.h"

#define AT24C02_ADDRESS 0xA0

void epromWrite(unsigned char WordAddress, Data)
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