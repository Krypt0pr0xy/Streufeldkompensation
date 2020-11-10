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

//#################################################################
//______Config_Function
void config_CLK_1MHZ(void);
void delay_ms(int ms);
void config_standart_Ports();
void config_HW_UART(void);
void config_SPI(void);

//#################################################################
//______UART_Function
void UARTSendArray(unsigned char array_to_send[]);
__interrupt void USCI0RX_ISR(void);

//#################################################################
//______SPI_Function
void SPISendByte(unsigned char input_char);
void SPISendData_1(unsigned char input_Byte);
void SPISendData_2(unsigned char input_Byte1, unsigned char input_Byte2);
void SPISendData_3(unsigned char input_Byte1, unsigned char input_Byte2, unsigned char input_Byte3);
unsigned char SPIReceiveByte();
