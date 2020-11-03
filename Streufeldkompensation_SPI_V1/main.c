#include <msp430.h> 


/**
 * main.c
 */


unsigned char clock = 0;

void config_CLK_1MHZ(void)
{
    //Select Internal 1Mhz
    DCOCTL = 0;
    BCSCTL1 = 13;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    clock = 1;
}


void delay_ms(int ms)
{
    //Dynamic Delay for 1Mhz an 8Mhz same Time
    unsigned int time = 0;
    for(time = clock*ms; time >= 1; time--)
    {
        __delay_cycles(1000);//NOP
    }
}


char data = 0;
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	config_CLK_1MHZ();

	//chipselect P1.4
	P1OUT |= BIT4;
	P1DIR |= BIT4;

	//UCB0:
	/*
	 * P1.6 = UCB0SOMI --> MISO
	 * P1.7 = UCB0SIMO --> MOSI
	 * P1.5 = USBCLK --> CLK
	 */
	P1SEL = (BIT5 | BIT6 | BIT7);
	P1SEL2 = (BIT5 | BIT6 | BIT7);


	//SPI Slau147 ab Seite 444

	UCB0CTL1 = UCSWRST;//Software Reset Enable
	UCB0CTL0 |= (UCCKPH + UCMSB + UCMST + UCSYNC); //Clock phase select, MSB first, Master MOde, 3-pin, 8-bit SPI master
	UCB0CTL1 |= UCSSEL_2; // SMCLK
	UCB0BR0 |= 0x02; // SMCLK / 2
	//UCB0BR1 = 0; // no prescaler
	UCB0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**

	//00 000 00 0 11111111 11111111




    P1OUT &= (~BIT4); // Select Device
    while(!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
    UCB0TXBUF = 0x99; // Send data over SPI to Slave
    while(!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
    UCB0TXBUF = 0x99; // Send data over SPI to Slave
    while(!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
    UCB0TXBUF = 0x12; // Send data over SPI to Slave
    P1OUT |= (BIT4); // Unselect Device

    delay_ms(1000);


    /*
    while (!(IFG2 & UCB0RXIFG)); // USCI_B0 RX Received?
    data = UCB0RXBUF; // Store received data
    */

    while(1);

    /*
     * CS LOW
     *
     * 0x12
     * senden
     * high
     * senden
     * low
     * senden
     * CS HIGH
     *
     *.
     */






}
