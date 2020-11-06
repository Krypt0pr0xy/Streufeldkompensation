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

//#################################################################
//______Konfigurier Funktionen
void config_CLK_1MHZ(void)
{
    //Select Internal 1Mhz
    DCOCTL = 0;
    BCSCTL1 = 13;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    clock = 1;
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


void update_all_chanels()
{
    /*
    Simultaneously update all channels of all DAC8564
    X X 1 1 0 0 X X X devices in the system with data stored in each
    channels data buffer
    */
    //Send 0x30

    P1OUT &= (~BIT4); // Select Device
    while(!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
    UCB0TXBUF = 0x30; // Send data over SPI to Slave
    __delay_cycles(8);//NOP
    while(!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
    UCB0TXBUF = 0x00; // Send data over SPI to Slave
    __delay_cycles(8);//NOP
    while(!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
    UCB0TXBUF = 0x00; // Send data over SPI to Slave
    __delay_cycles(20);//NOP
    P1OUT |= (BIT4); // Unselect Device
    delay_ms(5);
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//______MAIN:
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
	UCB0BR1 = 0; // no prescaler
	UCB0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**

    unsigned int value = 655300; //0-65536

    //Vout = ((X/65536)*10)-5
    float vout = -4.99;
    while(1)
    {
        value = ((vout+5)/10)*65536;

        unsigned char Low = value & 0xFF;
        unsigned char High = value >> 8;



        P1OUT &= (~BIT4); // Select Device

        while(!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
        UCB0TXBUF = 0x00; // Send data over SPI to Slave
        __delay_cycles(8);//NOP
        while(!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
        UCB0TXBUF = High; // Send data over SPI to Slave
        __delay_cycles(8);//NOP
        while(!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
        UCB0TXBUF = Low; // Send data over SPI to Slave
        __delay_cycles(20);//NOP

        P1OUT |= (BIT4); // Unselect Device


        update_all_chanels();
        delay_ms(1000);

    }
    /*
    while(!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
    UCB0TXBUF = 0xFF; // Send data over SPI to Slave

    while (!(IFG2 & UCB0RXIFG)); // USCI_B0 RX Received?
    data = UCB0RXBUF; // Store received data
     */

}


