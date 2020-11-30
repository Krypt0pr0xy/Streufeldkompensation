/*
 * Titel: Haubt Programm für Streufeldkompensation
 * ETH Zürich Hönggerberg 10.11.2020
 */
//#################################################################
//______Header Files
#include <msp430.h> 
#include "Streufeldkompensation_function.h"


//#################################################################
//______Main_Function
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	//all Config functions
	config_CLK_1MHZ();
	config_HW_UART();
	config_standart_Ports();
	config_SPI();
	config__MAX7301();
	setup_MAX7301pins();

	delay_ms(100);

	UARTSendArray("\r\n\r\nMSP430 Streufeldkompensation Started\r\n");
	UARTSendArray("****************************************************\r\n");


	while(1)
	{
	    /*
	    delay_ms(10);
	    check_interruptflag();
	    */



//_______________________________Test_______________________________
	   float set_voltage = 1;
       set_voltage = set_voltage + (vref / 2);
       unsigned long out = 0;
       out = (unsigned long)(set_voltage*(1048575/vref)); // calulating the Bit value

       //unsigned char byte3 = ((out >> 24) & 0xFF); //byte not used
       unsigned char byte2 = ((out >> 16) & 0xFF0000);//last byte from the long value
       unsigned char byte1 = ((out >> 8 ) & 0xFF0000);//middle byte from the long value
       unsigned char byte0 = (out & 0xFF0000);//first byte from the long value


       byte0 = 0x80;
       byte1 = 0x00;
       byte2 = 0x00;
       P1OUT &= (~BIT0);

	    while(!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
        UCB0TXBUF = byte0;//Sending SPI
        __delay_cycles(8);//NOP
        while(!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
        UCB0TXBUF = byte1;//Sending SPI
        __delay_cycles(8);//NOP
        while(!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
        UCB0TXBUF = byte2;//Sending SPI
        __delay_cycles(20);//NOP

        P1OUT |= (BIT0);



        delay_ms(10000);
//_______________________________Test_______________________________

	}
}


