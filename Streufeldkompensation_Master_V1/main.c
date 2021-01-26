/*
 * Titel: Haubt Programm für Streufeldkompensation
 * ETH Zürich Hönggerberg 10.11.2020
 */
//###############################################################################################################
/*                                     PINOUT:
                                                      __________
DVCC                                                1 | MSP430 |20  DVSS
P1.0/TA0CLK/ACLK/A0/CA0                             2 |        |19  XIN/P2.6/TA0.1
P1.1/TA0.0/ A1/CA1                                  3 |        |18  XOUT/P2.7
P1.2/TA0.1/ A2/CA2                                  4 |        |17  TEST/SBWTCK
P1.3/ADC10CLK/CAOUT/VREF-/VEREF-/A3/CA3             5 |        |16  RST/NMI/SBWTDIO
P1.4/SMCLK/UCB0STE/UCA0CLK/VREF+/VEREF+/A4/CA4/TCK  6 |        |15  P1.7/CAOUT /A7/CA7/TDO/TDI
P1.5/TA0.0/ A5/CA5/TMS                              7 |        |14  P1.6/TA0.1/ CA6/TDI/TCLK
P2.0/TA1.0                                          8 |        |13  P2.5/TA1.2
P2.1/TA1.1                                          9 |        |12  P2.4/TA1.2
P2.2/TA1.1                                          10|        |11
                                                      <-------->

          __________
DVCC    1 | MSP430 |20  DVSS
CS ADC  2 |        |19
UART RX 3 |        |18
UART TX 4 |        |17
        5 |        |16
CS GPIO 6 |        |15  SPI MOSI
SPI CLK 7 |        |14  SPI MISO
        8 |        |13
        9 |        |12
        10|        |11
          <-------->

*/
//###############################################################################################################
//_____Header Files
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

	config_specialPins(GPIO_OUTPUT, GPIO_OUTPUT, GPIO_OUTPUT, GPIO_OUTPUT);

	while(1)
	{
//	    UARTSendArray("P28=ON\r\n");
//	    delay_ms(3000);
//	    UARTSendArray("P28=OFF\r\n");
//	    delay_ms(3000);
	}

}


