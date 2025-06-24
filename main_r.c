#include <REGX52.H>
#include <Delay.h>
#include <LCD1602.h>
#include <Key.h>
#include <UART.h>
#include <string.h>

#define PWD_SIZE  6

uchar isUnlocked = 0, isSettingMode = 0;
uchar pwdInputCount = 0;
uchar pwdSet[PWD_SIZE] = {255, 255, 255, 255, 255, 255};
uchar pwdInput
uchar timer1Count = 0;

void timer1ISR(void) interrupt 3
{
	uchar i;
	timer1Count++;
	if(isUnlocked)//正确时显示滚动字幕
	{
        uchar j, l = 5;
		for(j = 0; j <= 18; j++)
			LCD_ShowChar(2, j+1, str1[(j+(timer1Count/l))%35]);
		
		timer1Count%=35*l-1;
		TL1 = 0x00;				//设置定时初始值
		TH1 = 0x4C;
		return;
	}
	if(timer1Count >= 200)//如果10s未操作
	{
		if(isSettingMode)
		{
			memset(pwdSet, 0xFF, PWD_SIZE);
			pwdInputCount = 0;
		}
			// for(i = 0; i < 6; i++)
			// {
			// 	pwdSet[i] = 255;
			// 	count_s = 0;
			// }
		else if(!setting)
			for(i = 0; i < 6; i++)
			{
				pwd_i[i] = 255;
				count = 0;
			}
		LCD_Init();
		timer1Count = 0;
		TL1 = 0x00;				//设置定时初始值
		TH1 = 0x4C;	
	}			//设置定时初始值
}

void setPassword(void)
{
	LCD_Init();
	if(pwdSet[0] == 255 && pwdSet[1] == 255 && pwdSet[2] == 255 && pwdSet[3] == 255 && pwdSet[4] == 255 && pwdSet[5] == 255)//如果密码没被设置过,就进入设置密码模式
	{
		isSettingMode = 1;
	}
	count_s = 0;
	Timer1_Init();
	while(isSettingMode)//进入设置密码模式
	{
		// P1 = 0xff >> count_s;//有几位就亮几个
		// P1_0 = 0;//最后一个灯亮标志设置密码环节
		LCD_ShowString(1,1,"Setting Password:");
		for(i = 0; i < 6; i++)//xianshimima
		{
			if(pwdSet[i] != 255)
				LCD_ShowNum(2, i+1, pwdSet[i], 1);//显示已输入的密码
			
			else if(i < 6)
			{
				LCD_ShowChar(2, i+1, '_');//显示光标
				break;
			}
			else//输满六位后不显示光标
				break;
			
		}
		key_s = Matrix();
		if(key_s)
		{
			timer = 0;
			if(key_s >= 1 && key_s <= 10 && count_s < 6)//输入数字
				for(j = 0; j < 6; j++)
					if(pwdSet[j] == 255)//从前往后找第一位未被设置的密码
					{
						pwdSet[j] = ((key_s-1) % 10);
						count_s++;//增加计数
						//key_s = 0;//跳出循环
						break;
					}
			else if(key_s == 11 && count_s > 0)//退格
				for(i = 5; i >= 0; i--)
					if(pwdSet[i] != 255)
					{
						pwdSet[i] = 255;
						key_s = 0;
						count_s--;
						LCD_Init();
						break;
					}
			else if(key_s == 12 && count_s == 6)//确认
			{
				LCD_Init();
				LCD_ShowString(1, 1, "OK!");
				Timer1_Reset();
				Uart1_Init();
				for(i = 0; i < 6; i++)
					UART_S(pwdSet[i]+0x30);//串口发送设置的密码
				
				TR1 = 0;
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
				setting = 0;//标志退出设置模式
			}
			else if(key_s == 12 && count_s != 6)//如果没输够六位
			{
				LCD_Init();
				LCD_ShowString(1, 1, "Too short!");
				Delay100ms(20);
				LCD_Init();
				LCD_ShowString(1,1,"Setting Password:");
			}
			else if(key_s == 100 && count_s != 0)//长按清除
			{
				for(i = 0; i < 6; i++)
					pwdSet[i] = 255;
				
				count_s = 0;
				LCD_Init();
			}
		}
	}
}