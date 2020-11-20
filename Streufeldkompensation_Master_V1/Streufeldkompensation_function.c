/*
 * Streufeldkompensation_function.c
 *
 * ETH Zürich Hönggerberg 10.11.2020
 */


#include <msp430.h>
#include "Streufeldkompensation_function.h"
int interrupt_flag;
char input_data[] = " ";//dummy buffer


char cmd_1[20] = "";
char cmd_2[20] = "";
char cmd_3[20] = "";
char cmd_4[20] = "";


//#################################################################
//______Config_Function

//______Konfigurier Funktionen
void config_CLK_1MHZ(void)
{
    //Select Internal 1Mhz
    DCOCTL = 0;
    BCSCTL1 = 13;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
}

void delay_ms(int ms)
{
    //Dynamic Delay for 1Mhz an 8Mhz same Time
    unsigned int time = 0;
    for(time = ms; time >= 1; time--)
    {
        __delay_cycles(1000);//NOP
    }
}


void config_standart_Ports(void)
{
    //chipselect P1.4
    P1OUT |= BIT4;
    P1DIR |= BIT4;
}

void config_HW_UART(void)
{
    //USA0
    /*
     * P1.1 --> RXD
     * P1.2 --> TXD
     *
     */

    P1SEL |= (BIT1 + BIT2); // P1.1 = RXD, P1.2=TXD
    P1SEL2 |= (BIT1 + BIT2); // P1.1 = RXD, P1.2=TXD

    //UART
    UCA0CTL1 |= UCSSEL_2; // Use SMCLK
    UCA0BR0 |= 104; // Set baud rate to 9600 with 1MHz clock (Data Sheet 15.3.13 family Guide page 424)
    UCA0BR1 = 0; // Set baud rate to 9600 with 1MHz clock
    UCA0MCTL |= UCBRS0; // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine
    IE2 |= UCA0RXIE; // Enable USCI_A0 RX interrupt
    __bis_SR_register(GIE); //interrupts enabled
}





void config_SPI(void)
{
    //UCB0:
    /*
     * P1.6 = UCB0SOMI --> MISO
     * P1.7 = UCB0SIMO --> MOSI
     * P1.5 = USBCLK --> CLK
     */
    P1SEL |= (BIT5 + BIT6 + BIT7);//selecting ports
    P1SEL2 |= (BIT5 + BIT6 + BIT7);//selecting ports


    //SPI Slau147 page 444
    UCB0CTL1 |= UCSWRST;//Software Reset Enable
    UCB0CTL0 |= (UCCKPH + UCMSB + UCMST + UCSYNC); //Clock phase select, MSB first, Master MOde, 3-pin, 8-bit SPI master
    UCB0CTL1 |= UCSSEL_2; // SMCLK
    UCB0BR0 |= 0x02; // SMCLK / 2
    UCB0BR1 = 0; // no prescaler
    UCB0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
}




void check_interruptflag(void)
{
    switch(interrupt_flag)
    {
        case interruptUARTRX:
            UARTSendArray(input_data);//send input back
            UARTSendArray("\r\n");//add a Carriage Return to the end
            CommandDecoder(input_data);
            break;
        default:
            break;
    }

    interrupt_flag = 0;
}


void config__MAX7301(void)
{
    SPISendData_2(0x04,0x01);//Start Config
    SPISendData_2(0x06,0xFF);//Input Mask Transition Register


    //set Pin Output
    SPISendData_2(0x09, 0x55);//P4-P7
    SPISendData_2(0x0A, 0x55);//P8-P11
    SPISendData_2(0x0B, 0x55);//P12-P15
    SPISendData_2(0x0C, 0x55);//P16-P19
    SPISendData_2(0x0D, 0x55);//P20-P23
    SPISendData_2(0x0E, 0x55);//P24-P27
    SPISendData_2(0x0F, 0x55);//P28-P31

    SPISendData_2(0x04, 0x01);//Normal operation
    SPISendData_2(0x04,0x81);//Configregister
}


void setup_MAX7301pins(void)
{
    MAX7301_setPIN(CH1_CS,ON);//CS on
    MAX7301_setPIN(CH2_CS,ON);//CS on
    MAX7301_setPIN(CH3_CS,ON);//CS on
    MAX7301_setPIN(CH4_CS,ON);//CS on
    MAX7301_setPIN(CH5_CS,ON);//CS on
    MAX7301_setPIN(CH6_CS,ON);//CS on
    MAX7301_setPIN(CH7_CS,ON);//CS on
    MAX7301_setPIN(CH8_CS,ON);//CS on

    //Set all A0 /A1 to OFF
    MAX7301_setPIN(CH1_A0,OFF);
    MAX7301_setPIN(CH1_A1,OFF);

    MAX7301_setPIN(CH2_A0,OFF);
    MAX7301_setPIN(CH2_A1,OFF);

    MAX7301_setPIN(CH3_A0,OFF);
    MAX7301_setPIN(CH3_A1,OFF);

    MAX7301_setPIN(CH4_A0,OFF);
    MAX7301_setPIN(CH4_A1,OFF);

    MAX7301_setPIN(CH5_A0,OFF);
    MAX7301_setPIN(CH5_A1,OFF);

    MAX7301_setPIN(CH6_A0,OFF);
    MAX7301_setPIN(CH6_A1,OFF);

    MAX7301_setPIN(CH7_A0,OFF);
    MAX7301_setPIN(CH7_A1,OFF);

    MAX7301_setPIN(CH8_A0,OFF);
    MAX7301_setPIN(CH8_A1,OFF);

}

//#################################################################
//______UART_Function

void UARTSendArray(char array_to_send[])//send char array
{
    unsigned int array_length = strlen(array_to_send);//size of char
    unsigned int counter = 0;
    for(counter = 0; counter <= array_length; counter++)//for loop with length
    {
        while(!(IFG2 & UCA0TXIFG));//Unload all data first from Buffer
        UCA0TXBUF = array_to_send[counter];//Take array at specific place
    }
}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    interrupt_flag = interruptUARTRX;
    UARTreceiveArray();
}


void UARTreceiveArray(void)
{
    unsigned char counter = 0;

    while((input_data[counter] = UCA0RXBUF) != '\r')//while fill buffer at specific place until Carriage return
    {
        while(!(UC0IFG & UCA0RXIFG));//Wait for buffer
        counter++;//Increase counter
    }
    input_data[counter+1] = '\0';//add end of array because c "string standard"
}

//#################################################################
//______SPI_Function

void SPISendByte(unsigned char input_Byte)//send SPI Byte Directly
{
    while(!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
    UCB0TXBUF = input_Byte;
}



void SPISendData_1(unsigned char input_Byte)//Send SPI Data with Chip Select an timing for 1 Byte
{
    CS_Low;
    SPISendByte(input_Byte);//Send first Byte
    __delay_cycles(20);//NOP
    CS_High;
}

void SPISendData_2(unsigned char input_Byte1, unsigned char input_Byte2)//Send SPI Data with Chip Select an timing for 2 Byte
{
    CS_Low;
    SPISendByte(input_Byte1);//Send first Byte
    __delay_cycles(8);//NOP
    SPISendByte(input_Byte2);//Send second Byte
    __delay_cycles(20);//NOP
    CS_High;
}

void SPISendData_3(unsigned char input_Byte1, unsigned char input_Byte2, unsigned char input_Byte3)//Send SPI Data with Chip Select an timing for 3 Byte
{
    CS_Low;
    SPISendByte(input_Byte1);//Send first Byte
    __delay_cycles(8);//NOP
    SPISendByte(input_Byte2);//Send second Byte
    __delay_cycles(8);//NOP
    SPISendByte(input_Byte3);//Send third Byte
    __delay_cycles(20);//NOP
    CS_High;
}
unsigned char SPIReceiveByte()
{
    while (!(IFG2 & UCB0RXIFG)); // USCI_B0 RX Received?
    return(UCB0RXBUF); // Store received data
}


//#################################################################
//______Command Decoder SET=CH1=4353424=OUT1->SET, CH1, 4353424, OUT1
/*
 * First command can only be set
 * Second commadn sets the channel from CH1-CH8
 * Third command gives the Voltage value from -4 to 4
 */
void CommandDecoder(char input_command[])
{
    UARTSendArray("Run Command Decoder\r\n");
    strcat(input_command, "\r\n");
    UARTSendArray(input_command);

    unsigned char counter = 0;
    unsigned char cmd_counter = 0;

    //SET=CH1=4353424=OUT1

    cmd_counter=0;//Reseting counter for out but array
    for(counter = 0; input_command[counter] != COMMANDCHAR; counter++)//Searching to Special Symbol
    {
        cmd_1[cmd_counter] = input_command[counter];//Copy char to comand char
        cmd_counter++;
    }

    cmd_counter=0;//Reseting counter for out but array
    for(counter = counter+1; input_command[counter] != COMMANDCHAR; counter++)//Searching to Special Symbol
    {
        cmd_2[cmd_counter] = input_command[counter];//Copy char to comand char
        cmd_counter++;
    }

    cmd_counter=0;//Reseting counter for out but array
    for(counter = counter+1; input_command[counter] != COMMANDCHAR; counter++)//Searching to Special Symbol
    {
        cmd_3[cmd_counter] = input_command[counter];//Copy char to comand char
        cmd_counter++;
    }

    cmd_counter=0;//Reseting counter for out but array
    for(counter = counter+1; input_command[counter] != COMMANDCHAR; counter++)//Searching to Special Symbol
    {
        cmd_4[cmd_counter] = input_command[counter];//Copy char to comand char
        cmd_counter++;
    }

    //ending the char array with a end Symbol
    cmd_1[strlen(cmd_1)+1] = '\0';
    cmd_2[strlen(cmd_2)+1] = '\0';
    cmd_3[strlen(cmd_3)+1] = '\0';
    cmd_4[strlen(cmd_4)+1] = '\0';
/*
    strcat(cmd_1, "\r\n");
    strcat(cmd_2, "\r\n");
    strcat(cmd_3, "\r\n");
    strcat(cmd_4, "\r\n");
*/
    UARTSendArray(cmd_1);
    UARTSendArray(cmd_2);
    UARTSendArray(cmd_3);
    UARTSendArray(cmd_4);
    UARTSendArray("END \r\n");

    if(strcmp(cmd_1, "SET") == 0)
    {
        command_SET(cmd_2, cmd_3, cmd_4);
    }

}


void command_SET(char channel[], char value[], char out[])
{
    /*Example for CH1 +/-10V 2.7k
     *
     * 1. MAX7301 P4 Einschalten für CS von DAC
     * 2. MAX5719 (DAC) Wert einstellen
     * 3. MAX7301 P4 Ausschalten für CS von DAC
     * 4. ADG1204 Decoder Einstellen und Pin Halten
     */
    unsigned char CH = channel[2];// extracting numer from CH3 --> 3
    float vout = 0.0;
    vout = (float)atoi(value);

    set_Voltage_MAX5719(CH, vout);

    unsigned char out_mode = (unsigned char)atoi(out);


    switch(CH)
    {
    case 1:
        if(out_mode == 1)//check if mode is +/- 1V
        {
            MAX7301_setPIN(CH1_A0,OFF);
            MAX7301_setPIN(CH1_A1,ON);
        }
        else if(out_mode == 10)//check if mode is +/- 10V
        {
            MAX7301_setPIN(CH1_A0,OFF);
            MAX7301_setPIN(CH1_A1,OFF);
        }
        break;

    case 2:
        if(out_mode == 1)//check if mode is +/- 1V
        {
            MAX7301_setPIN(CH2_A0,OFF);
            MAX7301_setPIN(CH2_A1,ON);
        }
        else if(out_mode == 10)//check if mode is +/- 10V
        {
            MAX7301_setPIN(CH2_A0,OFF);
            MAX7301_setPIN(CH2_A1,OFF);
        }
        break;

    case 3:
        if(out_mode == 1)//check if mode is +/- 1V
        {
            MAX7301_setPIN(CH3_A0,OFF);
            MAX7301_setPIN(CH3_A1,ON);
        }
        else if(out_mode == 10)//check if mode is +/- 10V
        {
            MAX7301_setPIN(CH3_A0,OFF);
            MAX7301_setPIN(CH3_A1,OFF);
        }
        break;

    case 4:
        if(out_mode == 1)//check if mode is +/- 1V
        {
            MAX7301_setPIN(CH4_A0,OFF);
            MAX7301_setPIN(CH4_A1,ON);
        }
        else if(out_mode == 10)//check if mode is +/- 10V
        {
            MAX7301_setPIN(CH4_A0,OFF);
            MAX7301_setPIN(CH4_A1,OFF);
        }
        break;

    case 5:
        if(out_mode == 1)//check if mode is +/- 1V
        {
            MAX7301_setPIN(CH5_A0,OFF);
            MAX7301_setPIN(CH5_A1,ON);
        }
        else if(out_mode == 10)//check if mode is +/- 10V
        {
            MAX7301_setPIN(CH5_A0,OFF);
            MAX7301_setPIN(CH5_A1,OFF);
        }
        break;

    case 6:
        if(out_mode == 1)//check if mode is +/- 1V
        {
            MAX7301_setPIN(CH6_A0,OFF);
            MAX7301_setPIN(CH6_A1,ON);
        }
        else if(out_mode == 10)//check if mode is +/- 10V
        {
            MAX7301_setPIN(CH6_A0,OFF);
            MAX7301_setPIN(CH6_A1,OFF);
        }
        break;

    case 7:
        if(out_mode == 1)//check if mode is +/- 1V
        {
            MAX7301_setPIN(CH7_A0,OFF);
            MAX7301_setPIN(CH7_A1,ON);
        }
        else if(out_mode == 10)//check if mode is +/- 10V
        {
            MAX7301_setPIN(CH7_A0,OFF);
            MAX7301_setPIN(CH7_A1,OFF);
        }
        break;

    case 8:
        if(out_mode == 1)//check if mode is +/- 1V
        {
            MAX7301_setPIN(CH8_A0,OFF);
            MAX7301_setPIN(CH8_A1,ON);
        }
        else if(out_mode == 10)//check if mode is +/- 10V
        {
            MAX7301_setPIN(CH8_A0,OFF);
            MAX7301_setPIN(CH8_A1,OFF);
        }
        break;
    }
}

void MAX7301_setPIN(unsigned char port_pin, unsigned char state)//e.g. MAX7301_setIO(5,1)
{
    char state_checked = 0x00;

    if(state == 0){state_checked = 0x00;}//check if state is bigger than 1
     else{state_checked = 0x01;}

    port_pin = port_pin + 0x20;//add offset (for more information datasheet)
    if((port_pin >= 0x20)&&(port_pin <=0x3F))//check if pin is avaible
    {
        SPISendData_2(port_pin,state_checked);//Send two Bytes over SPI
    }


}

void set_Voltage_MAX5719(unsigned char channel, float set_voltage)//Only for symetric
{
    if((set_voltage >= vref) || (set_voltage <= (vref*-1)))//check if Voltage input is in the range of vref
    {
        set_voltage = 0.0;
    }

    set_voltage = set_voltage + (vref / 2);
    unsigned long out = 0;
    out = (unsigned long)(set_voltage*(1048575/vref)); // calulating the Bit value


    //unsigned char byte3 = ((out >> 24) & 0xFF); //byte not used
    unsigned char byte2 = ((out >> 16) & 0xFF);//last byte from the long value
    unsigned char byte1 = ((out >> 8 ) & 0XFF);//middle byte from the long value
    unsigned char byte0 = (out & 0XFF);//first byte from the long value

    //Sending MSP First last 4 Bits are ignored
    switch(channel)//Sending to the correct Channel
    {
    case 1:
        MAX7301_setPIN(CH1_CS,OFF);//CS off
        SPISendData_3(byte2, byte1, byte0);//send Data over SPI
        MAX7301_setPIN(CH1_CS,ON);//CS on
        break;

    case 2:
        MAX7301_setPIN(CH2_CS,OFF);//CS off
        SPISendData_3(byte2, byte1, byte0);//send Data over SPI
        MAX7301_setPIN(CH2_CS,ON);//CS on
        break;

    case 3:
        MAX7301_setPIN(CH3_CS,OFF);//CS off
        SPISendData_3(byte2, byte1, byte0);//send Data over SPI
        MAX7301_setPIN(CH3_CS,ON);//CS on
        break;

    case 4:
        MAX7301_setPIN(CH4_CS,OFF);//CS off
        SPISendData_3(byte2, byte1, byte0);//send Data over SPI
        MAX7301_setPIN(CH4_CS,ON);//CS on
        break;

    case 5:
        MAX7301_setPIN(CH5_CS,OFF);//CS off
        SPISendData_3(byte2, byte1, byte0);//send Data over SPI
        MAX7301_setPIN(CH5_CS,ON);//CS on
        break;

    case 6:
        MAX7301_setPIN(CH6_CS,OFF);//CS off
        SPISendData_3(byte2, byte1, byte0);//send Data over SPI
        MAX7301_setPIN(CH6_CS,ON);//CS on
        break;

    case 7:
        MAX7301_setPIN(CH7_CS,OFF);//CS off
        SPISendData_3(byte2, byte1, byte0);//send Data over SPI
        MAX7301_setPIN(CH7_CS,ON);//CS on
        break;

    case 8:
        MAX7301_setPIN(CH8_CS,OFF);//CS off
        SPISendData_3(byte2, byte1, byte0);//send Data over SPI
        MAX7301_setPIN(CH8_CS,ON);//CS on
        break;
    }
}



