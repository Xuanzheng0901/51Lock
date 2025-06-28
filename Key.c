#include <REGX52.H>
#include <Delay.h>
#include <Key.h>

static unsigned char key = 0;
static unsigned char timer0Count = 0;

static void beep()
{
	P2_3 = 0;
	Delay100ms(1);
	P2_3 = 1;
}

static void timer0Init(void)		//50ms@11.0592MHz
{
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x01;			//设置定时器模式
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x4C;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;			    //定时器0开始计时
	EA = 1;
	ET0 = 1;
}
static void timer0Reset()
{
	TR0 = 0;
	TF0 = 0;
	ET0 = 0;
}

static void __keyboardScan(unsigned char line)
{
	unsigned char col = 0;
	P3 = ~(1 << line);
	for(col = 0; col < 4; col++)
	{
		if((P3 & (0x10 << col)) == 0)
		{
			beep();
			TR1 = 0;
			timer0Init(); //启动定时器
			while((P3 & (0x10 << (col))) == 0 && timer0Count < 20); //定时器计满或松手时打断
			timer0Reset();
			
			if(timer0Count >= 20) //判断是否为长按
			{
				beep();
				
				key = 4 * line + col + 101;
				while((P3 & (0x10 << col)) == 0);
				TR1 = 1;
				return;
			}
			timer0Count = 0;
			Delay1ms(20);
			TR1 = 1;
			key = 4 * line + col + 1;
		}
	}
}

static void __Matrix(void)
{
	unsigned char line;
	for(line = 0; line < 4; line++)
	{
		__keyboardScan(line);
		if(key >= 100)
			return;
	}
}

unsigned char Matrix(void)
{
	static uchar keynum;
	__Matrix();
	if(key)
	{
		keynum = key;
		key = 0;
		return keynum;
	}
	else return 0;
}

static void __timer0Interrupt() interrupt 1 //定时器0中断回调
{
	timer0Count += 1;
	TL0 = 0x00;
	TH0 = 0x4C;
}