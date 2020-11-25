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

	delay_ms(1000);
	UARTSendArray("\r\n\r\nMSP430 Streufeldkompensation Started\r\n");
	UARTSendArray("*******************************************\r\n");
	while(1)
	{
	    delay_ms(100);
	    //UARTSendArray("Hallo Welt \r\n");//add a Carriage Return to the end
	    check_interruptflag();

	}
}


