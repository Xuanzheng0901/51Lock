#include <REGX52.H>
#include <Delay.h>
#include <LCD1602.h>
#include <Key.h>
#include <UART.h>
#include <string.h>
#include <AT24C02.h>

#define PWD_SIZE  6

static const char str1[] = "Long Press Key13 To Reset Password ";
unsigned char isUnlocked = 0, isSettingMode = 1;
char pwdSet[PWD_SIZE+1] = {0};
char pwdInput[PWD_SIZE+1] = {0};

void flowingLED(void)
{
	unsigned char i;
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
}

void breathingLEDWhileWaiting(void)
{
	uchar i, j;
	uchar direct = 0;
	uchar secondsToWait = 30;

	while(secondsToWait--)//呼吸灯
	{
		LCD_ShowNum(2, 13, secondsToWait, 2);
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
}

void blinkingLED(void)
{
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
}

void timer1ISR(void) interrupt 3
{
	static unsigned char displayIndex = 0;
	static unsigned char timer1Count;
	unsigned char i;
	timer1Count++;
	if(isUnlocked && timer1Count >= 10)
	{
		for(i = 1; i <= 16; i++)
			LCD_ShowChar(2, i, str1[(displayIndex+i-1) % 35]);
		displayIndex++;
		displayIndex %= 35; 
		
		TL1 = 0x00;
		TH1 = 0x4F;
		timer1Count = 0;
	}
}

void Timer1_Init(void)		//50毫秒@11.0592MHz
{
	TMOD &= 0x0F;			//设置定时器模式
	TMOD |= 0x10;			//设置定时器模式
	TL1 = 0x00;				//设置定时初始值
	TH1 = 0x4F;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
	ET1 = 1;				//使能定时器1中断
	EA = 1;
}

void setPassword(void)
{
	unsigned char pwdInputCount = 0;
	unsigned char inputKey = 0;

	LCD_Clear();
	if(pwdSet[0] == 0 && pwdSet[1] == 0 && pwdSet[2] == 0 && pwdSet[3] == 0 && pwdSet[4] == 0 && pwdSet[5] == 0)//如果密码没被设置过,就进入设置密码模式
	{
		isSettingMode = 1;
		pwdSet[0] = '_';
	}
	LCD_ShowString(1, 1, "Setting Password");
	LCD_ShowString(2, 1, pwdSet);
	
	while(isSettingMode)//进入设置密码模式
	{
		P1 = (0xFF >> pwdInputCount) & 0xFE;//有几位就亮几个, 最后一个灯亮标志设置密码环节
		inputKey = Matrix();
		if(inputKey)
		{
			if(inputKey >= 1 && inputKey <= 14 && pwdInputCount < 6)
			{
				pwdSet[pwdInputCount] = inputKey < 11 ? (inputKey - 1 + '0') : (inputKey - 11 + 'A');
				pwdInputCount++;
				if(pwdInputCount < 6)
					pwdSet[pwdInputCount] = '_';
				LCD_ClearLine(2);
				LCD_ShowString(2, 1, pwdSet);
			}
			else if(inputKey == 15 && pwdInputCount > 0)//退格
			{
				pwdSet[pwdInputCount] = 0;
				pwdInputCount--;
				pwdSet[pwdInputCount] = '_';
				LCD_ClearLine(2);
				LCD_ShowString(2, 1, pwdSet);
			}
			else if(inputKey == 16 &&  pwdInputCount == 6)//确认
			{
				LCD_Clear();
				LCD_ShowString(1, 1, "OK!");
				// Uart1_Init();
				// for(i = 0; i < 6; i++)
				// 	UART_S(pwdSet[i]);//串口发送设置的密码
				// TR1 = 0;
				eepromWriteOneData(0x00, 0xcc);
				Delay100ms(1);
				eepromWritePage(0x10, pwdSet, 6);

				flowingLED();

				// TODO: 存入eeprom 

				isSettingMode = 0;//标志退出设置模式
			}
			else if(inputKey == 16 && pwdInputCount != 6)//如果没输够六位
			{
				LCD_Clear();
				LCD_ShowString(1, 1, "Too short!");
				Delay100ms(20);
				LCD_Clear();
				LCD_ShowString(1, 1, "Setting Password:");
				LCD_ShowString(2, 1, pwdSet);
			}
			else if(inputKey == 115 && pwdInputCount != 0)//长按清除
			{
				memset(pwdSet, 0, PWD_SIZE);
				pwdSet[0] = '_';
				pwdInputCount = 0;
				LCD_ClearLine(2);
				LCD_ShowString(2, 1, pwdSet);
			}
		}
	}
	LCD_Clear();
}

void lockMain(void)
{
	unsigned char pwdInputCount = 0;
	unsigned char inputKey = 0;
	unsigned char errorTime = 0;

	LCD_Clear();
	pwdInput[0] = '_';
	LCD_ShowString(1, 1, "Input Password:");
	LCD_ShowString(2, 1, pwdInput);
	
	while(!isSettingMode)//进入设置密码模式
	{
		P1 = (0xFF >> pwdInputCount);//有几位就亮几个, 最后一个灯亮标志设置密码环节
		inputKey = Matrix();
		if(inputKey)
		{
			if(inputKey >= 1 && inputKey <= 14 && pwdInputCount < 6)
			{
				pwdInput[pwdInputCount] = inputKey < 11 ? (inputKey - 1 + '0') : (inputKey - 11 + 'A');
				pwdInputCount++;
				if(pwdInputCount < 6)
					pwdInput[pwdInputCount] = '_';
				LCD_ShowString(2, 1, pwdInput);
			}
			else if(inputKey == 15 && pwdInputCount > 0)//退格
			{
				pwdInput[pwdInputCount] = 0;
				pwdInputCount--;
				pwdInput[pwdInputCount] = '_';
				LCD_ClearLine(2);
				LCD_ShowString(2, 1, pwdInput);
			}
			else if(inputKey == 16)//确认
			{
				if(pwdInputCount < 6)
				{
					LCD_Clear();
					LCD_ShowString(1, 1, "Too short!");
					Delay100ms(20);
					LCD_Clear();
					LCD_ShowString(1, 1, "Input Password:");
					LCD_ShowString(2, 1, pwdInput);
				}
				else if(pwdInputCount == 6)
				{
					if(strncmp(pwdInput, pwdSet, 6) == 0)
					{
						memset(pwdInput, 0, PWD_SIZE);
						pwdInput[0] = '_';
						pwdInputCount = 0;
						isUnlocked = 1;
						return;
					}
					else
					{
						errorTime++;
						LCD_Clear();
						if(errorTime >= 3)
						{
							LCD_ShowString(1, 1, "Too Many Errors!");
							LCD_ShowString(2, 1, "Retry After   s!");
							breathingLEDWhileWaiting();
							LCD_Clear();
							errorTime--;
						}
						else
						{
							LCD_ShowString(1 ,1, "Password Error!");
							LCD_ShowString(2, 1, "You Can Retry:");
							LCD_ShowNum(2, 15, 3 - errorTime, 1);
							blinkingLED();
							
						}
					}
				}
				memset(pwdInput, 0, PWD_SIZE);
				pwdInput[0] = '_';
				pwdInputCount = 0;
				LCD_ShowString(1, 1, "Input Password:");
				LCD_ClearLine(2);
				LCD_ShowString(2, 1, pwdInput);
			}
			else if(inputKey == 115 && pwdInputCount != 0)//长按清除
			{
				memset(pwdInput, 0, PWD_SIZE);
				pwdInput[0] = '_';
				pwdInputCount = 0;
				LCD_ClearLine(2);
				LCD_ShowString(2, 1, pwdInput);
			}
		}
	}
	LCD_Clear();
}

unsigned char unlocked(void)
{
	unsigned char keyInput;
	LCD_Clear();
	
	LCD_ShowString(1, 1, "Password right!");
	LCD_ShowString(1, 1, "Password right!");

	while(1)
	{
		
		keyInput = Matrix();
		if(keyInput == 116)
		{
			memset(pwdSet, 0, PWD_SIZE);
			isUnlocked = 0;
			return 1; //chongshemima
		}
		if(keyInput == 115)//长按退格键关锁
		{
			isUnlocked = 0;
			return 0;
		}
	}
}

void main()
{
	unsigned char i;
	Timer1_Init();
	LCD_Init();
	if(eepromRead(0x00) == 0xcc)
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
		LCD_Clear();
		setPassword();
		lockMain();
		isSettingMode = unlocked();
	}
}