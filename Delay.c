#include <Delay.h>
#include <REGX52.H>
#include <INTRINS.H>

void Delay100ms(unsigned char x)	//@11.0592MHz
{
	while(x--)
	{
		unsigned char data i, j;

		i = 180;
		j = 73;
		do
		{
			while (--j);
		} while (--i);
	}
}

void Delay1ms(unsigned char x)	//@11.0592MHz
{
	while(x--)
	{
		unsigned char data i, j;

		_nop_();
		i = 2;
		j = 199;
		do
		{
			while (--j);
		} while (--i);
	}
}

void Delay10us(unsigned char x)	//@11.0592MHz
{
	while(x--)
	{
		
		unsigned char data i;

		i = 2;
		while (--i);
	}
}
