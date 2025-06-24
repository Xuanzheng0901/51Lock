#ifndef __AT24C02_H__
#define __AT24C02_H__

void epromWrite(unsigned char WordAddress, Data);
unsigned char eepromRead(unsigned char WordAddress);

#endif