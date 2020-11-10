/*
 * Streufeldkompensation_function.c
 *
 *  Created on: 10.11.2020
 *      Author: Cyrill Wyller
 */


#include <msp430.h>
#include "Streufeldkompensation_function.h"



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


void config_HW_UART(void)
{
    P1SEL = BIT1 + BIT2 ; // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2 ; // P1.1 = RXD, P1.2=TXD
    UCA0CTL1 |= UCSSEL_2; // Use SMCLK
    UCA0BR0 = 104; // Set baud rate to 9600 with 1MHz clock (Data Sheet 15.3.13 family Guide page 424)
    UCA0BR1 = 0; // Set baud rate to 9600 with 1MHz clock
    UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST; // Initialize USCI state maschine
    IE2 |= UCA0RXIE; // Enable USCI_A0 RX interrupt

     __bis_SR_register(GIE); //interrupts enabled
}

void UARTSendArray(unsigned char array_to_send[])//send char array
{
    unsigned int array_length = strlen(array_to_send);//size of char
    int counter = 0;
    for(counter = 0; counter <= array_length; counter++)//for loop with length
    {
        while(!(IFG2 & UCA0TXIFG));//Unload all data first from Buffer
        UCA0TXBUF = array_to_send[counter];//Take array at specific place
    }
}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    unsigned char input_data[] = " ";//dummy buffer
    unsigned char counter = 0;

    while((input_data[counter] = UCA0RXBUF) != '\r')//while fill buffer at specific place until Carriage return
    {
        while(!(UC0IFG & UCA0RXIFG));//Wait for buffer
        counter++;//Increase counter
    }
    input_data[counter+1] = '\0';//add end of array because c "string standard"
    UARTSendArray(input_data);//send input back
    UARTSendArray("\r");//ad a Carriage Return to the end
}




