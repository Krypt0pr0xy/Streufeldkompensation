/*
 * Titel: Haubt Programm f�r Streufeldkompensation
 * Cyrill Wyler 10.11.2020
 */
//#################################################################
//______Header Files
#include <msp430.h> 
#include "Streufeldkompensation_function.h"
//#################################################################



//#################################################################
//______Main_Function

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	config_CLK_1MHZ();
	config_HW_UART();
	config_standart_Ports();
	config_SPI();
	config__MAX7301();
	while(1)
	{
	    delay_ms(100);
	    check_interruptflag();
	}
}

