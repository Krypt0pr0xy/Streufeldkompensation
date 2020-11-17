/*
 * Streufeldkompensation_function.h
 *
 *  Created on: 10.11.2020
 *      Author: Cyrill Wyller
 */

#ifndef STREUFELDKOMPENSATION_FUNCTION_H_
#define STREUFELDKOMPENSATION_FUNCTION_H_

#endif /* STREUFELDKOMPENSATION_FUNCTION_H_ */

#define CS_High (P1OUT |= (BIT4))
#define CS_Low (P1OUT &= (~BIT4))

#define COMMANDCHAR '='
#define interruptUARTRX 2

//#################################################################
//______Pin Defines For Max7301
#define Port4 0x24
#define Port5 0x25
#define Port6 0x26
#define Port7 0x27
#define Port8 0x28
#define Port9 0x29
#define Port10 0x2A
#define Port11 0x2B
#define Port12 0x2C
#define Port13 0x2D
#define Port14 0x2E
#define Port15 0x2F
#define Port16 0x30
#define Port17 0x31
#define Port18 0x32
#define Port19 0x33
#define Port20 0x34
#define Port21 0x35
#define Port22 0x36
#define Port23 0x37
#define Port24 0x38
#define Port25 0x39
#define Port26 0x3A
#define Port27 0x3B
#define Port28 0x3C
#define Port29 0x3D
#define Port30 0x3E
#define Port31 0x3F


//#################################################################
//______Config_Function
void config_CLK_1MHZ(void);
void delay_ms(int ms);
void config_standart_Ports();
void config_HW_UART(void);
void config_SPI(void);


void check_interruptflag(void);


void config__MAX7301(void);
//#################################################################
//______UART_Function
void UARTSendArray(char array_to_send[]);
__interrupt void USCI0RX_ISR(void);
void UARTreceiveArray(void);
//#################################################################
//______SPI_Function
void SPISendByte(unsigned char input_char);
void SPISendData_1(unsigned char input_Byte);
void SPISendData_2(unsigned char input_Byte1, unsigned char input_Byte2);
void SPISendData_3(unsigned char input_Byte1, unsigned char input_Byte2, unsigned char input_Byte3);
unsigned char SPIReceiveByte();


void CommandDecoder(char input_command[]);
void command_SET(char channel[], char value[], char out[]);

void MAX7301_setPIN(unsigned char port_pin, unsigned char state);
