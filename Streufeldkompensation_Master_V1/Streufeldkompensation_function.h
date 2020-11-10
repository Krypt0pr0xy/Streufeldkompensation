/*
 * Streufeldkompensation_function.h
 *
 *  Created on: 10.11.2020
 *      Author: Cyrill Wyller
 */

#ifndef STREUFELDKOMPENSATION_FUNCTION_H_
#define STREUFELDKOMPENSATION_FUNCTION_H_


#endif /* STREUFELDKOMPENSATION_FUNCTION_H_ */

void config_CLK_1MHZ(void);
void delay_ms(int ms);
void config_HW_UART(void);
void UARTSendArray(unsigned char array_to_send[]);
__interrupt void USCI0RX_ISR(void);
