#include <REGX52.H>
#include <Delay.h>
//#include <LCD1602.h>
#include <Nixie.h>
#include <Key.h>
#include <UART.h>
#include <string.h>
#include <AT24C02.h>
#include <Motor.h>

#define PWD_SIZE  8

static const char str1[] = "Long Press Key16 To Reset Password ";
unsigned char isUnlocked = 0, isSettingMode = 1;
char pwdSet[PWD_SIZE+1] = {0};
char pwdInput[PWD_SIZE+1] = {0};
extern unsigned char nixieBuffer[8];


void flowingLED(void)
{
	unsigned char i;
	ET1 = 0;
	P1 = 0x7f;
	for(i = 0; i < 7; i++)//流水灯
	{
		P1 = ~P1;
		P1 = P1 >> 1;
		P1 = ~P1;
		Delay100ms(1);
	}
	for(i = 0; i < 7; i++)//流水灯
	{
		P1 = ~P1;
		P1 = P1 << 1;
		P1 = ~P1;
		Delay100ms(1);
	}
	P1 = 0x7f;
	Delay100ms(3);//流水灯
	ET1 = 1;
}

void breathingLEDWhileWaiting(void)
{
	uchar i, j;
	uchar direct = 0;
	uchar secondsToWait = 30;

	ET1 = 0;
	while(secondsToWait--)//呼吸灯
	{
		Nixie(secondsToWait + '0', 1);
		for(i = 0; i < 100; i++)
		{
			for(j = 0; j < 6; j++)
			{
				P1 = 0x00;
				Delay10us(direct ? i : 100 - i);
				P1 = 0xff;
				Delay10us(direct ? 100 - i : i);
			}
		}
		direct = !direct;
	}
	ET1 = 1;
}

void blinkingLED(void)
{
	ET1 = 0;
	P0 = 0x07;
	P2_6 = 1;
	P2_6 = 0;
	P0 = 0x0;
	P2_7 = 1;
	P2_7 = 0;
	P1=0;
	Delay100ms(3);
	P1=~P1;
	Delay100ms(3);
	P1=~P1;
	Delay100ms(3);
	P1=~P1;
	Delay100ms(3);
	P1=~P1;
	Delay100ms(3);
	P1=~P1;
	Delay100ms(3);
	ET1 = 0;
}

void timer1Init(void)		//50毫秒@11.0592MHz
{
	TMOD &= 0x0F;			//设置定时器模式
	TMOD |= 0x10;			//设置定时器模式
	TL1 = 0xCD;				//设置定时初始值
	TH1 = 0xF8;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
	ET1 = 1;				//使能定时器1中断
	EA = 1;
}

unsigned char isPwdSet()
{
	unsigned char i;
	for(i = 0; i < PWD_SIZE; i++)
	{
		if(pwdSet[i] != 0)
		return 1;
	}
	return 0;
}

void setPassword(void)
{
	unsigned char pwdInputCount = 0;
	unsigned char inputKey = 0;
	static unsigned char i;
	if(!isPwdSet())//如果密码没被设置过,就进入设置密码模式
	{
		isSettingMode = 1;
		pwdSet[0] = '_';
	}
	else
		return;
	nixieShowString(pwdSet);
	//nixieShowString(pwdSet);
	while(isSettingMode)//进入设置密码模式
	{
		
		P1 = (0xFF >> pwdInputCount) & 0xFE;//有几位就亮几个, 最后一个灯亮标志设置密码环节
		inputKey = Matrix();
		if(inputKey)
		{
			if(inputKey >= 1 && inputKey <= 14 && pwdInputCount < PWD_SIZE)
			{
				pwdSet[pwdInputCount] = inputKey < 11 ? (inputKey - 1 + '0') : (inputKey - 11 + 'A');
				pwdInputCount++;
				if(pwdInputCount < PWD_SIZE)
					pwdSet[pwdInputCount] = '_';
				nixieShowString(pwdSet);
			}
			else if(inputKey == 15 && pwdInputCount > 0)//退格
			{
				pwdSet[pwdInputCount] = 0;
				pwdInputCount--;
				pwdSet[pwdInputCount] = '_';
				nixieShowString(pwdSet);

			}
			else if(inputKey == 16 &&  pwdInputCount == PWD_SIZE)//确认
			{
				Uart1_Init();
				ET1 = 0;
				for(i = 0; i < PWD_SIZE; i++)
					UART_S(pwdSet[i]);//串口发送设置的密码
				
				eepromWriteOneData(0x00, 0xcc);
				Delay100ms(1);
				eepromWritePage(0x10, pwdSet, PWD_SIZE);

				flowingLED();

				// TODO: 存入eeprom 

				isSettingMode = 0;//标志退出设置模式
				ET1 = 1;timer1Init();
			}
			else if(inputKey == 16 && pwdInputCount != PWD_SIZE)//如果没输够六位
			{
				//Delay100ms(20);
				nixieShowString(pwdSet);
			}
			else if(inputKey == 115 && pwdInputCount != 0)//长按清除
			{
				memset(pwdSet, 0, PWD_SIZE);
				pwdSet[0] = '_';
				pwdInputCount = 0;

				nixieShowString(pwdSet);
			}
		}
	}
}

void lockMain(void)
{
	unsigned char pwdInputCount = 0;
	unsigned char inputKey = 0;
	unsigned char errorTime = 0;

	pwdInput[0] = '_';
	nixieShowString(pwdInput);

	
	while(!isSettingMode)//进入设置密码模式
	{
		P1 = (0xFF >> pwdInputCount);//有几位就亮几个, 最后一个灯亮标志设置密码环节
		inputKey = Matrix();
		if(inputKey)
		{
			if(inputKey >= 1 && inputKey <= 14 && pwdInputCount < PWD_SIZE)
			{
				pwdInput[pwdInputCount] = inputKey < 11 ? (inputKey - 1 + '0') : (inputKey - 11 + 'A');
				pwdInputCount++;
				if(pwdInputCount < PWD_SIZE)
					pwdInput[pwdInputCount] = '_';
				nixieShowString(pwdInput);
			}
			else if(inputKey == 15 && pwdInputCount > 0)//退格
			{
				pwdInput[pwdInputCount] = 0;
				pwdInputCount--;
				pwdInput[pwdInputCount] = '_';
				nixieShowString(pwdInput);
			}
			else if(inputKey == 16)//确认
			{
				ET1 = 0;
				if(pwdInputCount < PWD_SIZE)
				{
					Delay100ms(20);
					nixieShowString(pwdInput);
				}
				else if(pwdInputCount == PWD_SIZE)
				{
					if(strncmp(pwdInput, pwdSet, PWD_SIZE) == 0)
					{
						memset(pwdInput, 0, PWD_SIZE);
						pwdInput[0] = '_';
						pwdInputCount = 0;
						//nixieShowString("66666666");
						isUnlocked = 1;
						ET1 = 1;
						return;
					}
					else
					{
						errorTime++;
						if(errorTime >= 3)
						{
							breathingLEDWhileWaiting();
							errorTime--;
						}
						else
						{
							blinkingLED();
						}
					}
				}
				memset(pwdInput, 0, PWD_SIZE);
				pwdInput[0] = '_';
				pwdInputCount = 0;
				nixieShowString(pwdInput);
				ET1 = 1;
			}
			else if(inputKey == 115 && pwdInputCount != 0)//长按清除
			{
				memset(pwdInput, 0, PWD_SIZE);
				pwdInput[0] = '_';
				pwdInputCount = 0;
				nixieShowString(pwdInput);
			}
		}
	}
}

unsigned char unlocked(void)
{
	unsigned char keyInput;
	ET1 = 1;
	motorBackward90();
	P0 = 0x6D;
	P2_6 = 1;
	P2_6 = 0;
	P0 = 0;
	P2_7 = 1;
	P2_7 = 0;
	while(1)
	{
		keyInput = Matrix();
		if(keyInput == 116)
		{
			memset(pwdSet, 0, PWD_SIZE);
			isUnlocked = 0;
			motorForward90();
			return 1; //chongshemima
		}
		if(keyInput == 115)//长按退格键关锁
		{
			isUnlocked = 0;
			motorForward90();
			return 0;
		}
	}
}

void main()
{
	unsigned char i;
	motorInit();
	timer1Init();
	
	if(eepromRead(0x00) == 0x00)
	{
		for(i = 0; i < 6; i++)
		{
			pwdSet[i] = eepromRead(0x10 + i);
			Delay1ms(1);
		}
		isSettingMode = 0;
	}
	while(1)
	{
		setPassword();
		lockMain();
		isSettingMode = unlocked();
		//ET1 = 1;
	}
}