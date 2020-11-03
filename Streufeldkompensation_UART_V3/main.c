/*
 * Titel: Test Programm für Streufeldkompensation
 * Cyrill Wyler 28.10.2020
 */
//#################################################################
//______Header Files
#include <msp430.h>
//#################################################################
//______Globale Variabeln
unsigned char clock = 0;
unsigned char data;
unsigned char string1[]={""};
//#################################################################
//______Konfigurier Funktionen

void config_CLK_8MHZ(void)
{
    //Select Internal 8Mhz
    DCOCTL = 0;
    BCSCTL1 = 7;
    BCSCTL1 = CALBC1_8MHZ;
    DCOCTL = CALDCO_8MHZ;
    clock = 8;
}


void config_CLK_1MHZ(void)
{
    //Select Internal 1Mhz
    DCOCTL = 0;
    BCSCTL1 = 13;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    clock = 1;
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

//#################################################################
//______Haupt-Funktionen
void delay_ms(int ms)
{
    //Dynamic Delay for 1Mhz an 8Mhz same Time
    unsigned int time = 0;
    for(time = clock*ms; time >= 1; time--)
    {
        __delay_cycles(1000);//NOP
    }
}



void UARTSendArray(unsigned char *Send_Data, unsigned char Input_char_Length)
{
while(Input_char_Length--)//as long Length is repeat
{
 while(!(IFG2 & UCA0TXIFG));//Unload all data first from Buffer
 UCA0TXBUF = *Send_Data;//Take first  of input data pointer
 Send_Data++;//Go one step further of the pointer
 }
}


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//______MAIN:
void main(void)

{
 WDTCTL = WDTPW + WDTHOLD;//Stop WatchDogTImer

 config_CLK_1MHZ();

 config_HW_UART();


 P1DIR |= (BIT0 + BIT6);//P1.0 (Red LED), P1.6 (Green LED) as Outputs
 P1OUT = 0x00; // Set P1.0 to Off


 P1DIR &= ~BIT3;                   // Set Switch pin to Input
 P1REN |= BIT3;                    // Enable Resistor for SW pin
 P1OUT |= BIT3;                    // Select Pull Up for SW pin

 while(1)
 {
     if(!(P1IN & BIT3))            // If Switch is Pressed
     {
         UARTSendArray("Button Pressed", 14);
         UARTSendArray("\n\r", 4);
     }
     P1OUT &= ~BIT6;             //Green LED -> OFF
     P1OUT |= BIT0;              //Red LED -> ON

     delay_ms(500);             //Delay 500ms

     P1OUT &= ~BIT0;             //Red LED -> OFF
     P1OUT |= BIT6;              //Green LED -> ON
     delay_ms(500);             //Delay 500ms
 }

}


//*****************************************************************
//______Interrupts:

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    data = UCA0RXBUF;
    UARTSendArray("Command: ", 9);
    UARTSendArray(&data, 1);
    UARTSendArray("\n\r", 4);
}
