/*
 * Titel: Haubt Programm f�r Streufeldkompensation
 * ETH Z�rich H�nggerberg 10.11.2020
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

    float set_voltage = 0;
	while(1)
	{
	    /*
	    delay_ms(10);
	    check_interruptflag();
	    */



//_______________________________Test_______________________________

       unsigned long out = 0;
       out = (unsigned long)(((set_voltage)+4.096)*2048000); // calulating the Bit value


       //unsigned char byte3 = ((out >> 24) & 0xFF); //byte not used
       unsigned char byte0 = (out >> 16);//last byte from the long value
       unsigned char byte1 = (out >> 8);//middle byte from the long value
       unsigned char byte2 = (out);//first byte from the long value


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


        delay_ms(1000);
//_______________________________Test_______________________________

	}
}


