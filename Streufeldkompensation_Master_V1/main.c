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
	config_CLK_1MHZ();//seting Clock to 1Mhz
	config_HW_UART();//configurate Harware UART set JUmper Correct!
	config_standart_Ports();//configurate standart I/O
	config_SPI();//setup SPI
	config__MAX7301();//configurate Max7301 over SPI
	setup_MAX7301pins();//configurate default Pins Max7301 over SPI
	config_Timer();
	UARTSendArray("\r\n\r\nMSP430 Streufeldkompensation Started\r\n");
	UARTSendArray("****************************************************\r\n");
//Test
	while(1)
	{}

}


