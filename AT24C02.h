#ifndef __AT24C02_H__
#define __AT24C02_H__

void eepromWriteOneData(unsigned char WordAddress, unsigned char Data);
unsigned char eepromRead(unsigned char WordAddress);
void eepromWritePage(unsigned char addr, unsigned char* Data, unsigned char length);

#endif