#include <REGX52.H>
#include <Delay.h>
#include <string.h>

unsigned char nixieBuffer[8] = {0};
static unsigned char nixieCount = 0;

void Nixie(char value, unsigned char pos)
{
    unsigned char i;
    static const unsigned char xdata nixieTable[16] = 
    {
        0x3F, 0x06, 0x5B, 0x4F, 0x66,
        0x6D, 0x7D, 0x07, 0x7F, 0x6F,
        0x77, 0x7C, 0x39, 0x5E, 0x08, 0x00
    };

    unsigned char valueIndex;

    switch(value)
    {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            valueIndex = value - '0';
            break;
        case 'A': case 'B': case 'C': case 'D':
            valueIndex = value - 'A' + 10;
            break;
        case '_':
            valueIndex = 14;
            break;
        default:
            valueIndex = 15;
    }

    nixieBuffer[pos] = nixieTable[valueIndex];
    for(i = 0; i < 8; i++)
    {
        if(nixieBuffer[i] != 0)
        {
            nixieCount++;
        }
        else
        {
            break;
        }
    }
}

void nixieShowString(const char* String)
{
    unsigned char i = 0;
    memset(nixieBuffer, 0, 8);
    while(String[i])
    {
        Nixie(String[i], i);
        i++;
    }
}

void nixieDisplayLoop(void) interrupt 3
{
    unsigned char i;
    for(i = 0; i < 8; i++)
    {
		P0 = 0xFF;
		P2_7 = 1;
        P2_7 = 0;
		P0 = nixieBuffer[i];
		P2_6 = 1;
        P2_6 = 0;
		P0 = ~(1 << i);
        P2_7 = 1;
        P2_7 = 0;
		Delay10us(50);
	}
	P0 = 0xFF;
	P2_7 = 1;
	P2_7 = 0;
	TL1 = 0x00;
	TH1 = 0xDC;	
}