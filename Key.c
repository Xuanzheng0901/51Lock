#include <REGX52.H>
#include <Delay.h>
#include <Key.h>

static unsigned char key = 0;
static unsigned char timer0Count = 0;
// extern uchar timer1Count;

// void beep()
// {
// 	// unsigned char _;
// 	// for(_ = 0; _ < 255; _++)
// 	// {
// 	// 	P2_5 = 0;
// 	// 	Delay10us(10);
// 	// 	P2_5 = 1;
// 	// 	Delay10us(10);
// 	// }
// }
void beep()
{
	P2_3 = 0;
	Delay100ms(1);
	P2_3 = 1;
}

void Timer0_Init(void)		//50ms@11.0592MHz
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
void Timer0_Reset()
{
	TR0=0;
	TF0=0;
	ET0=0;
}

static void __KeyboardScan(unsigned char line, unsigned char needLongPress)
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
				Timer0_Init();//启动定时器
				while((P3 & (0x10 << (col))) == 0 && timer0Count < 40);//定时器计满或松手时打断
				Timer0_Reset();
			}
			else
				while((P3 & (0x10 << (col))) == 0);
			
			if(timer0Count >= 40)
			{
				beep();
				timer0Count = 0;
				key = 4 * line + col + 101;
				while((P3 & (0x10 << col)) == 0);
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
			__KeyboardScan(line, 1);
		else if(line == 2)
			__KeyboardScan(line, 4);
		else
			__KeyboardScan(line, 0);
		if(key)
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

// unsigned char Matrix()
// {
// 	s = ET1;
// 	r = TR1; //储存定时器1中断启用状态
// 	ET1 = 0;
// 	TR1 = 0;//将定时器1关闭
// 	key1 = 0;

// 	P1=0xFF;
// 	Keyboard.Line1=0;
// 	if(Keyboard.Colu1==0){Delay1ms(20);beep();while(Keyboard.Colu1==0);Delay1ms(20);key1=1;}
// 	if(Keyboard.Colu2==0){Delay1ms(20);beep();while(Keyboard.Colu2==0);Delay1ms(20);key1=2;}
// 	if(Keyboard.Colu3==0){Delay1ms(20);beep();while(Keyboard.Colu3==0);Delay1ms(20);key1=3;}
// 	if(Keyboard.Colu4==0){Delay1ms(20);beep();while(Keyboard.Colu4==0);Delay1ms(20);key1=4;}

// 	P1=0xFF;
// 	Keyboard.Line2=0;
// 	if(Keyboard.Colu1==0){Delay1ms(20);beep();while(Keyboard.Colu1==0);Delay1ms(20);key1=5;}
// 	if(Keyboard.Colu2==0){Delay1ms(20);beep();while(Keyboard.Colu2==0);Delay1ms(20);key1=6;}
// 	if(Keyboard.Colu3==0){Delay1ms(20);beep();while(Keyboard.Colu3==0);Delay1ms(20);key1=7;}
// 	if(Keyboard.Colu4==0){Delay1ms(20);beep();while(Keyboard.Colu4==0);Delay1ms(20);key1=8;}

// 	P1=0xFF;
// 	Keyboard.Line3=0;
// 	if(Keyboard.Colu1==0){Delay1ms(20);beep();while(Keyboard.Colu1==0);Delay1ms(20);key1=9;}
// 	if(Keyboard.Colu2==0){Delay1ms(20);beep();while(Keyboard.Colu2==0);Delay1ms(20);key1=10;}
// 	if(Keyboard.Colu3==0)
// 	{
// 		Delay1ms(20);
// 		beep();
// 		Timer0_Init();//启动定时器
// 		while(Keyboard.Colu3==0 && count1 < 40);//定时器计满或松手时打断
// 		Timer0_Reset();
// 		if(count1 >= 40)//判断是否为长按
// 		{
// 			Delay1ms(20);
// 			count1=0;
// 			ET1 = s;
// 			TR1 = r;
// 			return 100;
// 		}
// 		else
// 		{
// 			Delay1ms(20);
// 			key1=11;
// 		}
// 	}
// 	if(Keyboard.Colu4==0){Delay1ms(20);beep();while(Keyboard.Colu4==0);Delay1ms(20);key1=12;}

// 	P1=0xFF;
// 	Keyboard.Line4=0;
// 	if(Keyboard.Colu1==0)
// 	{
// 		Delay1ms(20);beep();
// 		Timer0_Init();//启动定时器
// 		while(Keyboard.Colu1==0 && count1 < 40);//定时器计满或松手时打断
// 		Timer0_Reset();
// 		if(count1 >= 40)//判断是否为长按
// 		{
// 			Delay1ms(20);
// 			count1=0;
// 			ET1 = s;
// 			TR1 = r;
// 			return 99;
// 		}
// 		else
// 		{
// 			Delay1ms(20);
// 			count1=0;
// 			key1=13;
// 		}
// 	}
// 	//if(P3_5==0){Delay1ms(20);beep();while(P3_5==0);Delay1ms(20);key1=14;}
// 	//if(P3_6==0){Delay1ms(20);beep();while(P3_6==0);Delay1ms(20);key1=15;}
// 	//if(P3_7==0){Delay1ms(20);beep();while(P3_7==0);Delay1ms(20);key1=16;}
// 	ET1 = s;
// 	TR1 = r;
// 	return key1;
// }

