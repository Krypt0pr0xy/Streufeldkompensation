/*
 * Streufeldkompensation_function.c
 *
 *  Created on: 10.11.2020
 *      Author: Cyrill Wyller
 */


#include <msp430.h>
#include "Streufeldkompensation_function.h"
volatile unsigned char interrupt_flag;
//#################################################################
//______Config_Function

//______Konfigurier Funktionen
void config_CLK_1MHZ(void)
{
    //Select Internal 1Mhz
    DCOCTL = 0;
    BCSCTL1 = 13;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
}

void delay_ms(int ms)
{
    //Dynamic Delay for 1Mhz an 8Mhz same Time
    unsigned int time = 0;
    for(time = ms; time >= 1; time--)
    {
        __delay_cycles(1000);//NOP
    }
}


void config_standart_Ports(void)
{
    //chipselect P1.4
    P1OUT |= BIT4;
    P1DIR |= BIT4;
}

void config_HW_UART(void)
{
    //USA0
    /*
     * P1.1 --> RXD
     * P1.2 --> TXD
     */

    P1SEL |= (BIT1 + BIT2); // P1.1 = RXD, P1.2=TXD
    P1SEL2 |= (BIT1 + BIT2); // P1.1 = RXD, P1.2=TXD

    //UART
    UCA0CTL1 |= UCSSEL_2; // Use SMCLK
    UCA0BR0 |= 104; // Set baud rate to 9600 with 1MHz clock (Data Sheet 15.3.13 family Guide page 424)
    UCA0BR1 = 0; // Set baud rate to 9600 with 1MHz clock
    UCA0MCTL |= UCBRS0; // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine
    IE2 |= UCA0RXIE; // Enable USCI_A0 RX interrupt
    __bis_SR_register(GIE); //interrupts enabled
}





void config_SPI(void)
{
    //UCB0:
    /*
     * P1.6 = UCB0SOMI --> MISO
     * P1.7 = UCB0SIMO --> MOSI
     * P1.5 = USBCLK --> CLK
     */
    P1SEL |= (BIT5 + BIT6 + BIT7);//selecting ports
    P1SEL2 |= (BIT5 + BIT6 + BIT7);//selecting ports


    //SPI Slau147 page 444
    UCB0CTL1 |= UCSWRST;//Software Reset Enable
    UCB0CTL0 |= (UCCKPH + UCMSB + UCMST + UCSYNC); //Clock phase select, MSB first, Master MOde, 3-pin, 8-bit SPI master
    UCB0CTL1 |= UCSSEL_2; // SMCLK
    UCB0BR0 |= 0x02; // SMCLK / 2
    UCB0BR1 = 0; // no prescaler
    UCB0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
}




void check_interruptflag(void)
{

    switch(interrupt_flag)
    {
    case 2:
        UARTreceiveArray();
        break;
    default:
        break;
    }

    interrupt_flag = 0;
}


//#################################################################
//______UART_Function

void UARTSendArray(char array_to_send[])//send char array
{
    unsigned int array_length = strlen(array_to_send);//size of char
    unsigned int counter = 0;
    for(counter = 0; counter <= array_length; counter++)//for loop with length
    {
        while(!(IFG2 & UCA0TXIFG));//Unload all data first from Buffer
        UCA0TXBUF = array_to_send[counter];//Take array at specific place
    }
}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    interrupt_flag = 2;
    //UARTreceiveArray();
}


void UARTreceiveArray(void)
{
    char input_data[] = " ";//dummy buffer
    unsigned char counter = 0;

    while((input_data[counter] = UCA0RXBUF) != '\r')//while fill buffer at specific place until Carriage return
    {
        while(!(UC0IFG & UCA0RXIFG));//Wait for buffer
        counter++;//Increase counter
    }
    input_data[counter+1] = '\0';//add end of array because c "string standard"
    //CommandDecoder(input_data);
    UARTSendArray(input_data);//send input back
    UARTSendArray("\r");//ad a Carriage Return to the end
}

//#################################################################
//______SPI_Function

void SPISendByte(unsigned char input_Byte)//send SPI Byte Directly
{
    while(!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
    UCB0TXBUF = input_Byte;
}



void SPISendData_1(unsigned char input_Byte)//Send SPI Data with Chip Select an timing for 1 Byte
{
    CS_Low;
    SPISendByte(input_Byte);
    __delay_cycles(20);//NOP
    CS_High;
}

void SPISendData_2(unsigned char input_Byte1, unsigned char input_Byte2)//Send SPI Data with Chip Select an timing for 2 Byte
{
    CS_Low;
    SPISendByte(input_Byte1);
    __delay_cycles(8);//NOP
    SPISendByte(input_Byte2);
    __delay_cycles(20);//NOP
    CS_High;
}

void SPISendData_3(unsigned char input_Byte1, unsigned char input_Byte2, unsigned char input_Byte3)//Send SPI Data with Chip Select an timing for 3 Byte
{
    CS_Low;
    SPISendByte(input_Byte1);
    __delay_cycles(8);//NOP
    SPISendByte(input_Byte2);
    __delay_cycles(8);//NOP
    SPISendByte(input_Byte3);
    __delay_cycles(20);//NOP
    CS_High;
}
unsigned char SPIReceiveByte()
{
    while (!(IFG2 & UCB0RXIFG)); // USCI_B0 RX Received?
    return(UCB0RXBUF); // Store received data
}


void CommandDecoder(char input_command[])
{
    UARTSendArray("tEST");
    unsigned int array_length = sizeof(input_command);//size of char
    unsigned char counter = 0;
    unsigned char cmd_counter = 0;
    char cmd_1[] = "hallo\r";
    char cmd_2[] = "welt\r";
    char cmd_3[] = "abc\r";
    char cmd_4[] = "hi\r";



    UARTSendArray(cmd_1);
    UARTSendArray(cmd_2);
    UARTSendArray(cmd_3);
    UARTSendArray(cmd_4);
}
