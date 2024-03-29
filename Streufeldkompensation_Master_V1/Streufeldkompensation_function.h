/*
 * Streufeldkompensation_function.h
 *
 * ETH Z�rich H�nggerberg 10.11.2020
 */

#ifndef STREUFELDKOMPENSATION_FUNCTION_H_
#define STREUFELDKOMPENSATION_FUNCTION_H_

#endif /* STREUFELDKOMPENSATION_FUNCTION_H_ */
//#define DEBUGG

#define CORRECTION

#define ON 1
#define OFF 0
#define true 1
#define false 0

#define CS_Max7301_High (P1OUT |= (BIT4))//Setting the pin 1.4 High
#define CS_Max7301_Low (P1OUT &= (~BIT4))//Setting the pin 1.4 Low

#define LED_FAIL_High (P2OUT |= (BIT7))//Setting the pin 2.7 High
#define LED_FAIL_Low (P2OUT &= (~BIT7))//Setting the pin 2.7 Low

#define LED_DATA_High (P1OUT |= (BIT0))//Setting the pin 1.0 High
#define LED_DATA_Low (P1OUT &= (~BIT0))//Setting the pin 1.0 Low

#define GPIO_OUTPUT 0x01//MAX7301 setting for Output Datasheet Page 6
#define GPIO_INPUT  0x02//MAX7301 setting for Input Datasheet Page 6
#define GPIO_INPUT_with_PULLUP 0x03//MAX7301 setting for Input with Pullup resistor Datasheet Page 6

#define COMMANDCHAR '_'//Separator
#define interruptUARTRX 2//Interupt nomber for Uart TX
//#################################################################
//______Important Numbers do not change them !!!
#define buflen_status 60//!!DO not change them unless you discussed it with the creator of this document!!
#define buflen_input_data 90//!!DO not change them unless you discussed it with the creator of this document!!
#define buflen_cmd 20//!!DO not change them unless you discussed it with the creator of this document!!


#define vref 4.096//Ref Voltage

#define undervoltagelevel 0x2bc


//Gain (A) setting -10 bis 10 in 0.5 schritten
//      +/- 10V
#define CH1_AlphaA_10 0.999994758850174
#define CH1_AlphaB_10 -0.000223761229268582

#define CH2_AlphaA_10 0.999903748327526
#define CH2_AlphaB_10 0.0000836120582918874

#define CH3_AlphaA_10 0.999945071567944
#define CH3_AlphaB_10 0.000314956402439566

#define CH4_AlphaA_10 0.999859495783972
#define CH4_AlphaB_10 0.000291157636585122

#define CH5_AlphaA_10 0.999964084041812
#define CH5_AlphaB_10 0.0000955275795108313

#define CH6_AlphaA_10 0.999839055156794
#define CH6_AlphaB_10 0.000193771558536222

#define CH7_AlphaA_10 1.00002232554007
#define CH7_AlphaB_10 -0.0000259281612201269

#define CH8_AlphaA_10 0.999983354599303
#define CH8_AlphaB_10 -0.0000831890451217259

//      +/-1V 0.05 steps
#define CH1_AlphaA_1 0.999561994006969
#define CH1_AlphaB_1 -0.0000237973758537169

#define CH2_AlphaA_1 0.999624212891986
#define CH2_AlphaB_1 0.00000698056697560258

#define CH3_AlphaA_1 0.99973741641115
#define CH3_AlphaB_1 0.0000294150251219819

#define CH4_AlphaA_1 0.99956674076655
#define CH4_AlphaB_1 0.0000267672117072947

#define CH5_AlphaA_1 0.99964253184669
#define CH5_AlphaB_1 0.00000825862446343573

#define CH6_AlphaA_1 0.999671863554007
#define CH6_AlphaB_1 0.0000161272948779418

#define CH7_AlphaA_1 0.999913015574913
#define CH7_AlphaB_1 0.00000578664953669763

#define CH8_AlphaA_1 0.999885240243903
#define CH8_AlphaB_1 -0.0000107195109755916

//#################################################################
//
//______Name  Port Pin (MAX7301)
//  Portname to Pin Name
#define CH1_CS 8
#define CH1_A0 12
#define CH1_A1 9
#define CH2_CS 13
#define CH2_A0 10
#define CH2_A1 14
#define CH3_CS 11
#define CH3_A0 15
#define CH3_A1 16
#define CH4_CS 17
#define CH4_A0 18
#define CH4_A1 19
#define CH5_CS 20
#define CH5_A0 21
#define CH5_A1 22
#define CH6_CS 23
#define CH6_A0 24
#define CH6_A1 25
#define CH7_CS 26
#define CH7_A0 27
#define CH7_A1 28
#define CH8_CS 7
#define CH8_A0 29
#define CH8_A1 6
#define MUX_EN 30
//#################################################################
//______Config_Function
void config_CLK_1MHZ(void);
void delay_ms(int ms);
void config_standart_Ports(void);
void config_HW_UART(void);
void config_SPI(void);
void config_ADC10(void);
void check_interruptflag(void);
void config_Timer(void);
__interrupt void Timer_A_CCR0_ISR(void);

void config_MAX7301(void);
void config_specialPins(unsigned char pin28_status, unsigned char pin29_status, unsigned char pin30_status, unsigned char pin31_status);
void setup_MAX7301pins(void);
//#################################################################
//______UART_Function
void UARTSendArray(char array_to_send[]);
__interrupt void USCI0RX_ISR(void);
void UARTreceiveArray(void);

//#################################################################
//______SPI_Function
void SPISendByte(unsigned char input_char);
void SPISendData_Max7301_1(unsigned char input_Byte);
void SPISendData_Max7301_2(unsigned char input_Byte1, unsigned char input_Byte2);
void SPISendData_Max7301_3(unsigned char input_Byte1, unsigned char input_Byte2, unsigned char input_Byte3);
unsigned char SPIReceiveByte(void);
char SPIReceive_Input_Max7301(unsigned char Pin);

//#################################################################
//______Command Decoder
void CommandDecoder(char input_command[]);
void command_SET(char channel[buflen_cmd], char value[buflen_cmd], char out[buflen_cmd], char out_res[buflen_cmd]);
void command_Help();
void command_PORTCONFIGURE(char commad2[buflen_cmd], char commad3[buflen_cmd], char commad4[buflen_cmd]);
void command_PORTSET(char commad2[buflen_cmd], char commad3[buflen_cmd]);
void command_PORTREAD(char commad2[buflen_cmd]);

//#################################################################
//______MAX7301 GPIO expander
void MAX7301_setPIN(unsigned char port_pin, unsigned char state);

//#################################################################
//______MAX5719 DAC
void set_Voltage_MAX5719(unsigned char channel, double set_voltage, unsigned char out_mode);


























//You have scrolled down to here so far, her is an Easter Egg
//......................................................
//
//
//                   2b | !2b
//            That is the question here
