#include <REGX52.H>
#include <Delay.h>
#include <LCD1602.h>
#include <Key.h>
#include <UART.h>
#include <string.h>

int timer = 0, timer_i = 0, i, j, k, l = 5;
unsigned char count0 = 0, isSettingMode = 0, key = 0, key_s = 0, count = 0, count_s = 0, err = 0, isUnlocked = 0, pwd[6] = {255,255,255,255,255,255}, pwd_i[6] = {255,255,255,255,255,255};
char str1[] = "Long Press Key13 To Reset Password ";

void timer1ISR(void) interrupt 3
{
	timer++;
	if(isUnlocked)//正确时显示滚动字幕
	{
		for(j = 0; j <= 18; j++)
			LCD_ShowChar(2, j+1, str1[(j+(timer/l))%35]);
		
		timer%=35*l-1;
		TL1 = 0x00;				//设置定时初始值
		TH1 = 0x4C;
		return;
	}
	if(timer >= 200)//如果10s未操作
	{
		if(isSettingMode)
			for(i = 0; i < 6; i++)
			{
				pwd[i] = 255;
				count_s = 0;
			}
		else if(!isSettingMode)
			for(i = 0; i < 6; i++)
			{
				pwd_i[i] = 255;
				count = 0;
			}
		
		LCD_Init();
		timer = 0;
	}

	TL1 = 0x00;				//设置定时初始值
	TH1 = 0x4C;				//设置定时初始值
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

void Timer1_Reset()
{
	TR1 = 0;				//定时器1停止计时
	ET1 = 0;				//关闭定时器1中断
	timer = 0;
}

void main()
{
	start:
	//P1 = 0xFF;
	LCD_Init();
	if(pwd[0] == 255 && pwd[1] == 255 && pwd[2] == 255 && pwd[3] == 255 && pwd[4] == 255 && pwd[5] == 255)//如果密码没被设置过,就进入设置密码模式
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
			if(pwd[i] != 255)
				LCD_ShowNum(2, i+1, pwd[i], 1);//显示已输入的密码
			
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
			{
				for(j = 0; j < 6; j++)
					if(pwd[j] == 255)//从前往后找第一位未被设置的密码
					{
						pwd[j] = ((key_s - 1) % 10);
						count_s++;//增加计数
						//key_s = 0;//跳出循环
						break;
					}
			}
			else if(key_s == 11 && count_s > 0)//退格
			{
				for(i = 5; i >= 0; i--)
					if(pwd[i] != 255)
					{
						pwd[i] = 255;
						key_s = 0;
						count_s--;
						LCD_Init();
						break;
					}
			}
			else if(key_s == 12 && count_s == 6)//确认
			{
				LCD_Init();
				LCD_ShowString(1, 1, "OK!");
				Timer1_Reset();
				Uart1_Init();
				for(i = 0; i < 6; i++)
					UART_S(pwd[i]+0x30);//串口发送设置的密码
				
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
				isSettingMode = 0;//标志退出设置模式
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
					pwd[i] = 255;
				
				count_s = 0;
				LCD_Init();
			}
		}
	}
	Lock:
	LCD_Init();
	count = 0;
	Timer1_Init();
	timer = 0;
	while(isSettingMode == 0)//开锁环节
	{
		P1 = 0xff >> count;
		LCD_ShowString(1, 1, "Input Password:");
		for(i = 0; i < 6; i++)//xianshimima
		{
			if(pwd_i[i] != 255)
			{
				LCD_ShowNum(2, i+1, pwd_i[i], 1);
			}
			else if(i < 6)
			{
				LCD_ShowChar(2, i+1, '_');
				break;
			}
			else
			{
				break;
			}
		}
		key = Matrix();
		if(key)
		{
			timer = 0;//按下按键重置timer
			if(key >= 1 && key <= 10 && count < 6)
			{
				for(j = 0; j < 6; j++)
				{
					if(pwd_i[j] == 255)
					{
						pwd_i[j] = ((key-1) % 10);
						count++;
						key = 0;
						break;
					}
				}
			}
			else if(key == 11 && count > 0)
			{	
				for(i = 5; i >= 0; i--)
				{
					if(pwd_i[i] != 255)
					{
						pwd_i[i] = 255;
						key = 0;
						count--;
						LCD_Init();
						break;
					}
				}
			}
			else if(key == 12 && count == 6)
			{
				if(pwd_i[0] == pwd[0] && pwd_i[1] == pwd[1] && pwd_i[2] == pwd[2] && pwd_i[3] == pwd[3] && pwd_i[4] == pwd[4] && pwd_i[5] == pwd[5] && pwd_i[0] != 255 && pwd_i[1] != 255 && pwd_i[2] != 255 && pwd_i[3] != 255 && pwd_i[4] != 255 && pwd_i[5] != 255)
				{
					LCD_Init();
					LCD_ShowString(1, 1, "Password right!");
					//Timer1_Reset();
					isUnlocked = 1;
					timer = 0;
					err = 0;
					count = 0;
					P1 = 0;
					while(1 && isSettingMode == 0)//密码正确后进入开锁状态,LED全亮
					{
						key = Matrix();
						if(key == 113)//长按13键进入设置模式(只有在输入正确后才能重设密码)
						{
							for(i = 0; i < 6; i++)
							{
								pwd[i] = 255;
								pwd_i[i] = 255;
							}
							isSettingMode = 1;
							LCD_Init();
							isUnlocked = 0;
							Timer1_Reset();
							timer = 0;
							goto start;
						}
						if(key == 111)//长按退格键关锁
						{
							for(i = 0; i < 6; i++)
							{
								pwd_i[i] = 255;
							}
							isUnlocked = 0;
							goto Lock;
						}
						
					}
				}
				else
				{
					LCD_Init();
					LCD_ShowString(1 ,1, "Password Error!");
					LCD_ShowString(2, 1, "You Can Retry:");
					err++;//增加错误次数
					timer = 0;
					LCD_ShowNum(2, 15, 5-err, 1);//还能试的次数
					for(i = 0; i < 6; i++)
					{
						pwd_i[i] = 255;
					}
					count = 0;
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
					Delay100ms(3);//闪灯提醒
					if(err == 5)//错误五次锁定30s,同时开启呼吸灯
					{
						LCD_Init();
						Timer1_Reset();
						LCD_ShowString(1, 1, "Too Many Errors!");
						LCD_ShowString(2, 1, "Retry After   s!");

						k = 15;
						//timer = 0;
						timer_i = 30;
						LCD_ShowNum(2, 13, timer_i, 2);
						P1 = 0xff;
						while(k--)//呼吸灯
						{
							for(i = 0; i < 100; i++)
							{
								for(j = 0; j < 6; j++)
								{
									P1 = 0x00;
									Delay10us(i);
									P1 = 0xff;
									Delay10us(100-i);
								}
							}
							timer_i--;
							LCD_ShowNum(2, 13, timer_i, 2);
							for(i = 0; i < 100; i++)
							{
								for(j = 0; j < 6; j++)
								{
									P1 = 0x00;
									Delay10us(100-i);
									P1 = 0xff;
									Delay10us(i);
								}
							}
							timer_i--;
							LCD_ShowNum(2, 13, timer_i, 2);
						}
						err--;
					}
					LCD_Init();
					Timer1_Init();
				}
			}
			else if(key == 12 && count != 6)
			{
				LCD_Init();
				LCD_ShowString(1, 1, "Too short!");
				for(i = 0; i < 6; i++)
				{
					pwd_i[i] = 255;
				}
				count = 0;
				Delay100ms(20);
				timer = 0;
				LCD_Init();
			}
			else if(key == 100 && count != 0)//qingchu
			{
				for(i = 0; i < 6; i++)
				{
					pwd_i[i] = 255;
				}
				count = 0;
				LCD_Init();
			}
		}
	}
}
