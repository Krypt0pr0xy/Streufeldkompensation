/*
 * Titel: Haubt Programm für Streufeldkompensation
 * Cyrill Wyler 10.11.2020
 */
//#################################################################
//______Header Files
#include <msp430.h> 
#include "Streufeldkompensation_function.h"
//#################################################################



//#################################################################
//______Haupt-Funktionen

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	config_CLK_1MHZ();
	config_HW_UART();

	while(1)
	{

	    delay_ms(100);
	 }
}


