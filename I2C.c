#include <REGX52.H>

sbit I2C_SCL = P2^1;
sbit I2C_SDA = P2^0;

void iicStart(void)
{
	I2C_SDA = 1;
	I2C_SCL = 1;
	I2C_SDA = 0;
	I2C_SCL = 0;
}

void iicStop(void)
{
	I2C_SDA = 0;
	I2C_SCL = 1;
	I2C_SDA = 1;
}

void iicSendByte(unsigned char Byte)
{
	unsigned char i;
	for(i = 0; i < 8; i++)
	{
		I2C_SDA = Byte & (0x80 >> i);
		I2C_SCL = 1;
		I2C_SCL = 0;
	}
}

unsigned char iicRecvByte(void)
{
	unsigned char i, Byte = 0x00;
	I2C_SDA = 1;
	for(i = 0; i < 8; i++)
	{
		I2C_SCL = 1;
		if(I2C_SDA)
		{
			Byte |= (0x80 >> i);
		}
		I2C_SCL = 0;
	}
	return Byte;
}

void iicSendAck(unsigned char AckBit)
{
	I2C_SDA = AckBit;
	I2C_SCL = 1;
	I2C_SCL = 0;
}

unsigned char iicRecvAck(void)
{
	unsigned char AckBit;
	I2C_SDA = 1;
	I2C_SCL = 1;
	AckBit = I2C_SDA;
	I2C_SCL = 0;
	return AckBit;
}