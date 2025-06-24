#include "UART.h"
#include <REGX52.H>


void Uart1_Init(void)	//28800bps@11.0592MHz
{
	PCON |= 0x80;		//使能波特率倍速位SMOD
	SCON = 0x50;		//8位数据,可变波特率
	TMOD &= 0x0F;		//设置定时器模式
	TMOD |= 0x20;		//设置定时器模式
	TL1 = 0xFE;			//设置定时1初始值
	TH1 = 0xFE;			//设置定时1重载值
	ET1 = 0;			//禁止定时器1中断
	TR1 = 1;			//定时器1开始计时
	EA = 1;
}

void UART_S(unsigned char x)
{
	SBUF = x;
	while(TI == 0);
	TI = 0;
}

