#include <REGX52.H>
#include <Delay.h>
#include <Key.h>

static unsigned char key = 0;
static unsigned char timer0Count = 0;
static unsigned char timer1Status;
// extern uchar timer1Count;

void beep()
{
	P2_3 = 0;
	Delay100ms(1);
	P2_3 = 1;
}

void timer0Init(void)		//50ms@11.0592MHz
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
void timer0Reset()
{
	TR0=0;
	TF0=0;
	ET0=0;
}

static void __keyboardScan(unsigned char line, unsigned char needLongPress)
{
	unsigned char col = 0;
	//P3 = 0xFF;
	P3 = ~(1 << line);
	for(col = 0; col < 4; col++)
	{
		if((P3 & (0x10 << col)) == 0)
		{
			beep();
			// timer1Count = 0;
			if((1 << col) & needLongPress)
			{
				timer1Status = TR1;
				TR1 = 0;
				timer0Init();//启动定时器
				while((P3 & (0x10 << (col))) == 0 && timer0Count < 20);//定时器计满或松手时打断
				timer0Reset();
				TR1 = timer1Status;
			}
			else
				while((P3 & (0x10 << (col))) == 0);
			
			if(timer0Count >= 20)
			{
				beep();
				timer0Count = 0;
				key = 4 * line + col + 101;
				while((P3 & (0x10 << col)) == 0);
				return;
			}
			Delay1ms(20);
			key = 4 * line + col + 1;
		}
	}
}

static void __Matrix(void)
{
	unsigned char line, key = 0;
	for(line = 0; line < 4; line++)
	{
		if(line == 3)
			__keyboardScan(line, 0xF);
		else if(line == 2)
			__keyboardScan(line, 0xF);
		else
			__keyboardScan(line, 0xF4);
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

void __timer0Interrupt() interrupt 1 //定时器0中断回调
{
	timer0Count += 1;
	TL0 = 0x00;
	TH0 = 0x4C;
}