/*
 * Streufeldkompensation_function.h
 *
 * ETH Zürich Hönggerberg 10.11.2020
 */

#ifndef STREUFELDKOMPENSATION_FUNCTION_H_
#define STREUFELDKOMPENSATION_FUNCTION_H_

#endif /* STREUFELDKOMPENSATION_FUNCTION_H_ */

#define ON 1
#define OFF 0

#define CS_High (P1OUT |= (BIT4))
#define CS_Low (P1OUT &= (~BIT4))

#define COMMANDCHAR '='
#define interruptUARTRX 2

#define vref 4.096
//#################################################################
//______Pin Defines For Max7301
/*
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
*/
//______Name  Port Pin
#define CH1_CS 4
#define CH1_A0 5
#define CH1_A1 6
#define CH2_CS 7
#define CH2_A0 8
#define CH2_A1 9
#define CH3_CS 10
#define CH3_A0 11
#define CH3_A1 12
#define CH4_CS 13
#define CH4_A0 14
#define CH4_A1 15
#define CH5_CS 16
#define CH5_A0 17
#define CH5_A1 18
#define CH6_CS 19
#define CH6_A0 20
#define CH6_A1 21
#define CH7_CS 22
#define CH7_A0 23
#define CH7_A1 24
#define CH8_CS 25
#define CH8_A0 26
#define CH8_A1 27



//#################################################################
//______Config_Function
void config_CLK_1MHZ(void);
void delay_ms(int ms);
void config_standart_Ports();
void config_HW_UART(void);
void config_SPI(void);


void check_interruptflag(void);


void config__MAX7301(void);

void setup_MAX7301pins(void);
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

//#################################################################
//______Command Decoder
void CommandDecoder(char input_command[]);
void command_SET(char channel[], char value[], char out[]);

//#################################################################
//______MAX7301 GPIO expander
void MAX7301_setPIN(unsigned char port_pin, unsigned char state);

//#################################################################
//______MAX5719 DAC
void set_Voltage_MAX5719(unsigned char channel, float set_voltage);
