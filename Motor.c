#include <REGX52.H>
#include <Delay.h>

sbit MOTOR_A=P2^2;
sbit MOTOR_B=P2^5;
sbit MOTOR_C=P4^4;
sbit MOTOR_D=P4^6;

void setMotorPos(unsigned char position)
{
    MOTOR_A = !(0x01 & position);
    MOTOR_B = !(0x02 & position);
    MOTOR_C = !(0x04 & position);
    MOTOR_D = !(0x08 & position);
}

void motorForward90()
{
    unsigned char i = 1;
    unsigned int j = 0;
    for(j = 0; j < 512; j++)
    {
        setMotorPos(i);
        i = ((i == 8) ? 1 : (i << 1));
        Delay10us(150);
    }
    setMotorPos(0xFF);
}

void motorBackward90()
{
    unsigned char i = 0x08;
    unsigned int j = 0;
    for(j = 0; j < 512; j++)
    {
        setMotorPos(i);
        i = ((i == 1) ? 8 : (i >> 1));
        Delay10us(150);
    }
    setMotorPos(0xFF);
}
void motorInit()
{
    MOTOR_A = 0;
    MOTOR_B = 0;
    MOTOR_C = 0;
    MOTOR_D = 0;
}