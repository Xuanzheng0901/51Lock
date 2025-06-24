#ifndef __I2C_H__
#define __I2C_H__

void iicStart(void);
void iicStop(void);
void iicSendByte(unsigned char Byte);
unsigned char iicRecvByte(void);
void iicSendAck(unsigned char AckBit);
unsigned char iicRecvAck(void);


#endif
