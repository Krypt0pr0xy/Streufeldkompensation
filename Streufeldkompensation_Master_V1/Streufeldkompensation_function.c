/*
 * Streufeldkompensation_function.c
 *
 * ETH Zürich Hönggerberg 10.11.2020
 */


#include <msp430.h>
#include "Streufeldkompensation_function.h"
#include <string.h>

char interrupt_flag = 0;//Flag for interupts
char flag_LED_DATA = 0;
char status[buflen_status] = "";//Status buffer
char input_data[buflen_input_data] = " ";//buffer for input
char cmd_1[buflen_cmd] = "";//command 1 buffer
char cmd_2[buflen_cmd] = "";//command 2 buffer
char cmd_3[buflen_cmd] = "";//command 3 buffer
char cmd_4[buflen_cmd] = "";//command 4 buffer
char cmd_5[buflen_cmd] = "";//command 5 buffer
unsigned char pin28_setting = GPIO_INPUT;
unsigned char pin29_setting = GPIO_INPUT;
unsigned char pin30_setting = GPIO_INPUT;
unsigned char pin31_setting = GPIO_INPUT;
unsigned int LED_DATA_timer = 1000;
//#########################################################################################################################################################
//______________________________________________________________________Config_Function

//______Konfigurier Funktionen
void config_CLK_1MHZ(void)
{
    //Select Internal 1Mhz
    DCOCTL = 0;//DCO Clock Frequency Control
    BCSCTL1 = 13;//Basic Clock System Control 1
    BCSCTL1 = CALBC1_1MHZ;//BCSCTL1 Calibration Data for 1MHz
    DCOCTL = CALDCO_1MHZ;//DCOCTL  Calibration Data for 1MHz
}

void delay_ms(int ms)
{
    //Dynamic Delay for 1Mhz
    unsigned int time = 0;
    for(time = ms; time >= 1; time--)
    {
        __delay_cycles(1000);//NOP
    }
}


void config_standart_Ports(void)
{
    //chipselect P1.4 GPIO Expander
    P1OUT |= BIT4;
    P1DIR |= BIT4;

    //LED_DATA
    P1OUT |= BIT0;
    P1DIR |= BIT0;

    //P2.7 als normal GPIO
    P2SEL &= ~(BIT7);
    P2OUT |= BIT7;
    P2DIR |= BIT7;

    P1OUT &= ~(BIT3);
    P1DIR |= BIT3;

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

    while(UCA0TXBUF > 0);//Clearing TXBUFFER
    while(UCA0RXBUF > 0);//Clearing RXBUFFER

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


void config_ADC10()
{
    P1DIR &= ~(BIT3);
    //Datasheet slau144 page 553
    ADC10CTL1 |= INCH_3 + ADC10DIV_3;//Input channel select --> A3
    ADC10CTL0 |= ADC10SHT_2 + ADC10ON;//Select Vref --> VR+ = VCC and VR- = VSS


}

void check_interruptflag(void)
{
    //select the interrupt
    switch(interrupt_flag)
    {
        case interruptUARTRX:
            UARTSendArray("\r\nReceived: \t");//add a Carriage Return to the end
            UARTSendArray(input_data);//send input back
            UARTSendArray("\r\n\r\n");//add a Carriage Return to the end
            delay_ms(100);
            CommandDecoder(input_data);
            break;
        default:
            break;
    }

    if(flag_LED_DATA == 1)//check if LED_DATA flag is On
    {
        LED_DATA_timer--;//decreas Timer
        if(LED_DATA_timer <= 1)// if Timer is under 10
        {//Reset LED_DATA
            LED_DATA_Low;//setting LED_DATA Low
            LED_DATA_timer = 1000;//Reset LED Timer
            flag_LED_DATA = 0;//Reseting flag
        }
    }


    ADC10CTL0 |= ENC + ADC10SC;//Start ADC10 mesure
    while(ADC10CTL1 & ADC10BUSY);//check if ADC10 is Busy

    if(ADC10MEM < undervoltagelevel)//check voltage at ADC PIN
    {//If a under voltage has detected the processor will do nothing
        //all Chanels off
        MAX7301_setPIN(MUX_EN, OFF);
        while(1)
        {
            delay_ms(300);//Blinking TIME
            LED_FAIL_High;
            delay_ms(300);//Blinking TIME
            LED_FAIL_Low;
        }
    }
    interrupt_flag = 0;//reseting the flag
}

void config_Timer(void)
{
    //Set MCLK = SMCLK = 1MHz
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    //Timer Configuration
    TACCR0 = 0; //Initially, Stop the Timer
    TACCTL0 |= CCIE; //Enable interrupt for CCR0.
    TACTL = TASSEL_2 + ID_0 + MC_1; //Select SMCLK, SMCLK/1 , Up Mode
    TACCR0 = 1000-1; //Start Timer, Compare value for Up Mode to get 1ms delay per loop
}


#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A_CCR0_ISR(void)//Timer A Interupt every 1ms
{
    check_interruptflag();//check if an interupt was done
}


void config_MAX7301(void)
{
    SPISendData_Max7301_2(0x04,0x81);//Start Config
    SPISendData_Max7301_2(0x06,0x00);//Input Mask Transition Register
  //Datasheet Page 6
//    REGISTER                        |ADDRESS CODE (HEX) |       REGISTER DATA
//-----------------------------------------------------------------------------------------------------------
//                                                                |D7|D6| |D5|D4| |D3|D2| |D1|D0|
//Port Configuration for P7, P6, P5, P4       |   0x09    |       |  P7   |  P6   |  P5   |  P4 |
//Port Configuration for P11, P10, P9, P8     |   0x0A    |       |  P11  |  P10  |  P9   |  P8 |
//Port Configuration for P15, P14, P13, P12   |   0x0B    |       |  P15  |  P14  |  P13  |  P12|
//Port Configuration for P19, P18, P17, P16   |   0x0C    |       |  P19  |  P18  |  P17  |  P16|
//Port Configuration for P23, P22, P21, P20   |   0x0D    |       |  P23  |  P22  |  P21  |  P20|
//Port Configuration for P27, P26, P25, P24   |   0x0E    |       |  P27  |  P26  |  P25  |  P24|
//Port Configuration for P31, P30, P29, P28   |   0x0F    |       |  P31  |  P30  |  P29  |  P28|
//
//
//MODE    FUNCTION                        PORT    REGISTER    PIN BEHAVIOR            ADDRESSCODE (HEX)   PORT CONFIGURATION BIT PAIR
//                                                                                                UPPER LOWER
//-----------------------------------------------------------------------------------------------------------------------
//Output  GPIO Output                     Register bit = 0    Active-low logicoutput  0x09 to 0x0F        0       1
//                                        Register bit = 1    Active-high logicoutput 0x09 to 0x0F        0       1
//-----------------------------------------------------------------------------------------------------------------------
//Input   GPIO Input Without Pullup       Register bit input logic level logic input  0x09 to 0x0F        1       0
//-----------------------------------------------------------------------------------------------------------------------
//Input GPIO Input with Pullup            Register bit input logic level logic input  0x09 to 0x0F        1       1

    //set Pin Output
    SPISendData_Max7301_2(0x09, 0x55);//P4-P7 as Output
    SPISendData_Max7301_2(0x0A, 0x55);//P8-P11 as Output
    SPISendData_Max7301_2(0x0B, 0x55);//P12-P15 as Output
    SPISendData_Max7301_2(0x0C, 0x55);//P16-P19 as Output
    SPISendData_Max7301_2(0x0D, 0x55);//P20-P23 as Output
    SPISendData_Max7301_2(0x0E, 0x55);//P24-P27 as Output


    SPISendData_Max7301_2(0x0F, 0x55);//P28-P31 as Input
    //SPISendData_Max7301_2(0x0F, 0xAA);//P28-P31 as Input

    SPISendData_Max7301_2(0x04, 0x01);//Normal operation
}
void config_specialPins(unsigned char pin28_status, unsigned char pin29_status, unsigned char pin30_status, unsigned char pin31_status)
{

    unsigned char config = 0x00;//buffer for register

    config = pin28_status;//adding the setting for pin 28
    config |= (pin29_status << 2);//shifting setting for pin 29
    config |= (pin30_status << 4);//shifting setting for pin 30
    config |= (pin31_status << 6);//shifting setting for pin 31

    SPISendData_Max7301_2(0x04,0x81);//Start Config
    SPISendData_Max7301_2(0x06,0x00);//Input Mask Transition Register

    SPISendData_Max7301_2(0x0F, config);//P28-P31 as config

    SPISendData_Max7301_2(0x04, 0x01);//Normal operation
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

    //Set all A0 / A1 to OFF
    //CH1----------------------
    MAX7301_setPIN(CH1_A0,OFF);
    MAX7301_setPIN(CH1_A1,OFF);
    //CH2----------------------
    MAX7301_setPIN(CH2_A0,OFF);
    MAX7301_setPIN(CH2_A1,OFF);
    //CH3----------------------
    MAX7301_setPIN(CH3_A0,OFF);
    MAX7301_setPIN(CH3_A1,OFF);
    //CH4----------------------
    MAX7301_setPIN(CH4_A0,OFF);
    MAX7301_setPIN(CH4_A1,OFF);
    //CH5----------------------
    MAX7301_setPIN(CH5_A0,OFF);
    MAX7301_setPIN(CH5_A1,OFF);
    //CH6----------------------
    MAX7301_setPIN(CH6_A0,OFF);
    MAX7301_setPIN(CH6_A1,OFF);
    //CH7----------------------
    MAX7301_setPIN(CH7_A0,OFF);
    MAX7301_setPIN(CH7_A1,OFF);
    //CH8----------------------
    MAX7301_setPIN(CH8_A0,OFF);
    MAX7301_setPIN(CH8_A1,OFF);

    //MUX----------------------
    MAX7301_setPIN(MUX_EN, OFF);
}

//#########################################################################################################################################################
//______________________________________________________________________UART_Function

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
    interrupt_flag = interruptUARTRX;//Seting the Interrupt flag for UART RX
    while(!(UC0IFG & UCA0RXIFG));//Wait for buffer
    UARTreceiveArray();
}


void UARTreceiveArray(void)
{
    unsigned char counter = 0;
    char dummybuffer = 0;
    while((input_data[counter] = UCA0RXBUF) != '\r')//while fill buffer at specific place until Carriage return
    {
        while(!(UC0IFG & UCA0RXIFG));//Wait for buffer
        counter++;//Increase counter
    }
    input_data[counter+1] = '\0';//add end of array because c "string standard"

    //Clearing the rest of the UART Buffer
    while(UCA0RXBUF != 0x0D)
    {
        dummybuffer = UCA0RXBUF;
        while(!(UC0IFG & UCA0RXIFG));//Wait for buffer
    }
    if(dummybuffer){dummybuffer = 0;}//usles for function but this isnt there it will get outoptimized

}




//#########################################################################################################################################################
//______________________________________________________________________SPI_Function

void SPISendByte(unsigned char input_Byte)//send SPI Byte Directly
{
    while(!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
    UCB0TXBUF = input_Byte;//Sending SPI
}





void SPISendData_Max7301_1(unsigned char input_Byte)//Send SPI Data with Chip Select an timing for 1 Byte
{
    CS_Max7301_Low;
    SPISendByte(input_Byte);//Send first Byte
    __delay_cycles(20);//NOP
    CS_Max7301_High;
    //delay_ms(100);
}

void SPISendData_Max7301_2(unsigned char input_Byte1, unsigned char input_Byte2)//Send SPI Data with Chip Select an timing for 2 Byte
{
    CS_Max7301_Low;
    SPISendByte(input_Byte1);//Send first Byte
    //__delay_cycles(8);//NOP
    SPISendByte(input_Byte2);//Send second Byte
    __delay_cycles(20);//NOP
    CS_Max7301_High;
    //delay_ms(100);
}

void SPISendData_Max7301_3(unsigned char input_Byte1, unsigned char input_Byte2, unsigned char input_Byte3)//Send SPI Data with Chip Select an timing for 3 Byte
{
    CS_Max7301_Low;
    SPISendByte(input_Byte1);//Send first Byte
    //__delay_cycles(8);//NOP
    SPISendByte(input_Byte2);//Send second Byte
    //__delay_cycles(20);//NOP
    SPISendByte(input_Byte3);//Send third Byte
    __delay_cycles(20);//NOP
    CS_Max7301_High;
    //delay_ms(100);
}

void SPISendData_Max5719_3(unsigned char input_Byte1, unsigned char input_Byte2, unsigned char input_Byte3)
{
    SPISendByte(input_Byte1);//Send first Byte
    //__delay_cycles(8);//NOP
    SPISendByte(input_Byte2);//Send second Byte
    //__delay_cycles(8);//NOP
    SPISendByte(input_Byte3);//Send third Byte
    __delay_cycles(20);//NOP
    //delay_ms(100);
}

unsigned char SPIReceiveByte()
{
    while (!(IFG2 & UCB0RXIFG)); // USCI_B0 RX Received?
    return(UCB0RXBUF); // Store received data
}

char SPIReceive_Input_Max7301(unsigned char Pin)
{
    unsigned char temp = 0;
    unsigned char temp2 = 0;
    Pin += 0x20;
    Pin |= 0x80;
    CS_Max7301_Low;
    SPISendByte(Pin);//Send first Byte
    __delay_cycles(8);//NOP
    SPISendByte(0x00);//Send second Byte
    __delay_cycles(20);//NOP
    CS_Max7301_High;
    __delay_cycles(5);//NOP
    CS_Max7301_Low;
    SPISendByte(0x00);//Send second Byte
    __delay_cycles(8);//NOP
    while (!(IFG2 & UCB0RXIFG)); // USCI_B0 RX Received?
    temp = UCB0RXBUF; // Store received data
    SPISendByte(0x00);//Send second Byte
    while (!(IFG2 & UCB0RXIFG)); // USCI_B0 RX Received?
    temp2 = UCB0RXBUF; // Store received data
    __delay_cycles(20);//NOP
    CS_Max7301_High;
    delay_ms(100);


    if(temp == Pin && temp2 == 0x01)
    {
#ifdef DEBUGG//if debugg is defined putout the message
        UARTSendArray("ON\r\n");
#endif
        return (0x01);
    }
    if(temp == Pin && temp2 == 0x00)
    {
#ifdef DEBUGG//if debugg is defined putout the message
        UARTSendArray("OFF\r\n");
#endif
        return (0x00);
    }
}
//#########################################################################################################################################################
//____________________________________________Command Decoder SET=CH1=4353424=OUT1->SET, CH1, 4353424, OUT1
/*
 * First command can only be set
 * Second commadn sets the channel from CH1-CH8
 * Third command gives the Voltage value from -1 to 1 or -10 to 10
 */
void CommandDecoder(char input_command[])
{
#ifdef DEBUGG//if debugg is defined putout the message
        UARTSendArray("CommandDecoder: Start");
        UARTSendArray("\r\n");
#endif
    strcpy(status,"");//status clear

    unsigned char clearCounter=0;
    //Clearing all Char arrays
    for(clearCounter = 0; clearCounter <=19;clearCounter++)
    {
        cmd_1[clearCounter] = '\0';
        cmd_2[clearCounter] = '\0';
        cmd_3[clearCounter] = '\0';
        cmd_4[clearCounter] = '\0';
        cmd_5[clearCounter] = '\0';
    }
    unsigned char length = strlen(input_command);
    unsigned char counter = 0;
    unsigned char cmd_counter = 0;

    //SET=CH1=4.5=OUT10

    cmd_counter=0;//Reseting counter for out but array
    for(counter = 0; (input_command[counter] != COMMANDCHAR) && (counter <= length); counter++)//Searching to Special Symbol
    {
        cmd_1[cmd_counter] = input_command[counter];//Copy char to comand char
        cmd_counter++;
    }

    cmd_counter=0;//Reseting counter for out but array
    for(counter = counter+1; (input_command[counter] != COMMANDCHAR) && (counter <= length); counter++)//Searching to Special Symbol
    {
        cmd_2[cmd_counter] = input_command[counter];//Copy char to comand char
        cmd_counter++;
    }

    cmd_counter=0;//Reseting counter for out but array
    for(counter = counter+1; (input_command[counter] != COMMANDCHAR) && (counter <= length); counter++)//Searching to Special Symbol
    {
        cmd_3[cmd_counter] = input_command[counter];//Copy char to comand char
        cmd_counter++;
    }

    cmd_counter=0;//Reseting counter for out but array
    for(counter = counter+1; (input_command[counter] != COMMANDCHAR) && (counter <= length); counter++)//Searching to Special Symbol
    {
        cmd_4[cmd_counter] = input_command[counter];//Copy char to comand char
        cmd_counter++;
    }

    cmd_counter=0;//Reseting counter for out but array
    for(counter = counter+1; (input_command[counter] != '\r') && (counter <= length); counter++)//Searching to Special Symbol
    {
        cmd_5[cmd_counter] = input_command[counter];//Copy char to comand char
        cmd_counter++;
    }

    //ending the char array with a end Symbol
    cmd_1[strlen(cmd_1)+1] = '\0';
    cmd_2[strlen(cmd_2)+1] = '\0';
    cmd_3[strlen(cmd_3)+1] = '\0';
    cmd_4[strlen(cmd_4)+1] = '\0';
    cmd_5[strlen(cmd_4)+1] = '\0';
    //Sending the Information over UART back


#ifdef DEBUGG//if debugg is defined putout the message
        UARTSendArray("Command Decoder:\r\n");
        UARTSendArray(" * Command 1: \t");
        UARTSendArray(cmd_1);
        UARTSendArray("\r\n");
        UARTSendArray(" * Command 2: \t");
        UARTSendArray(cmd_2);
        UARTSendArray("\r\n");
        UARTSendArray(" * Command 3: \t");
        UARTSendArray(cmd_3);
        UARTSendArray("\r\n");
        UARTSendArray(" * Command 4: \t");
        UARTSendArray(cmd_4);
        UARTSendArray("\r\n");
        UARTSendArray(" * Command 5: \t");
        UARTSendArray(cmd_5);

        UARTSendArray("\r\n\r\n");
        UARTSendArray("\r\n");
#endif

    //Check the SET Command and when true start command_set function
    if(strcmp(cmd_1, "SET\0") == 0)
    {
        LED_DATA_High;
        flag_LED_DATA = 1;
        command_SET(cmd_2, cmd_3, cmd_4, cmd_5);
    }

    //Check the Help Command
    if(strcmp(cmd_1, "Help\0") == 0)
    {
        LED_DATA_High;
        flag_LED_DATA = 1;
        command_Help();
    }


    //------------------------------------------------------------------------------------------------------------
    //Command for configure the ports
    if(strcmp(cmd_1, "PORTCONFIGURE\0") == 0)//Check if first command is PORTCONFIGURE
    {
        LED_DATA_High;
        flag_LED_DATA = 1;
        command_PORTCONFIGURE(cmd_2, cmd_3, cmd_4);
    }
//______________________________________________________________________________________________________
        //Command for setting the Port
    if(strcmp(cmd_1, "PORTSET\0") == 0)//Check if command 1 is PORTSET
    {
        LED_DATA_High;
        flag_LED_DATA = 1;
        command_PORTSET(cmd_2, cmd_3);
    }


    if(strcmp(cmd_1, "PORTREAD\0") == 0)//Check if command 1 is PORTREAD
    {
        LED_DATA_High;
        flag_LED_DATA = 1;
        command_PORTREAD(cmd_2);
    }
//-------------------------------------------------------------------------------------------------------
#ifdef DEBUGG//if debugg is defined putout the message
        UARTSendArray("CommandDecoder: End");
        UARTSendArray("\r\n");
#endif
}

//#########################################################################################################################################################
//____________________________________________________________________________command functions
void command_SET(char channel[buflen_cmd], char value[buflen_cmd], char out[buflen_cmd], char out_res[buflen_cmd])
{
    /*Example for CH1 +/-10V 2.7k
     *
     * 1. MAX7301 P4 Einschalten für CS von DAC
     * 2. MAX5719 (DAC) Wert einstellen
     * 3. MAX7301 P4 Ausschalten für CS von DAC
     * 4. ADG1204 Decoder Einstellen und Pin Halten
     */
#ifdef DEBUGG//if debugg is defined putout the message
        UARTSendArray("command_SET: Start");
        UARTSendArray("\r\n");
#endif
//_____________________________________________________________________________
    //Extracting the Voltage

    char input[20] = "";//input buffer
    strcpy(input, value);//copy it from value
    input[19] = '\0';//securing the last buffer as end symbol
    int firstdigit = 0;
    unsigned long long commavalue = 0;
    unsigned long long Divider = 1E18;
    firstdigit = atoi(strtok(value, ','));//take the first digit only
    if(firstdigit < 0){firstdigit *= (-1);}//be sure it is positiv
    char offset = 0;
    if(input[0] == '-')//check if first buffer has an minus symbol
    {
        offset = 1;
        input[2] = 0;//clearing buffer
#ifdef DEBUGG//if debugg is defined putout the message
        UARTSendArray("command_SET: with '-'");
        UARTSendArray("\r\n");
#endif
    }else{
#ifdef DEBUGG//if debugg is defined putout the message
        UARTSendArray("command_SET: without '-'");
        UARTSendArray("\r\n");
#endif
    }
    input[0] = 0;//clearing buffer
    input[1] = 0;//clearing buffer

    char counter = 0;
    for(counter = (2+offset); counter <=19; counter++)//counting through the array
    {
        char temp_buf[2] = {input[counter], '\0' };//filling an temporary buffer ech time
        commavalue+=(atoi(temp_buf)*Divider);//

        Divider /= 10;//setting divider one place down
    }


    double vout = 0.0;

    vout+=firstdigit;//adding the first digit to Vout
    vout+=(commavalue/1E19);//adding the comma value to Vout

    if(offset > 0){vout *= (-1);}//multiplying with -1 when the value should be negativ

//_____________________________________________________________________________________________

    //Getting the string Value to an float Variable
    unsigned char CH = 0;
    //Extracting the channel nummber of the char array
    if(channel[3] == '\0')//check if its only one digit
    {
        char buffer[1] = channel[2];//Reading the number ov CH1 --> 1
        CH = atoi(buffer);// extracting nummer from "3" --> 3;
        if((CH > 8) || (CH == 0))//check if channel nummer between 1 and 8
        {
            strcat(status,"-->Wrong Channel set to CH1\r\n");//Safing error
            CH = 1;
        }
    }
    else{strcat(status,"-->Wrong Channel set to CH1\r\n");CH = 1;}//Safing error

    unsigned char out_mode = 1;//Variable for Out1 or OUT10
    if(strcmp(out, "OUT1") == 0)
    {out_mode = 1;}
    else{out_mode = 10;}

    set_Voltage_MAX5719(CH, vout, out_mode);

    unsigned char A0_OUT10 = OFF;
    unsigned char A1_OUT10 = OFF;

    unsigned char A0_OUT1 = OFF;
    unsigned char A1_OUT1 = ON;


    if(strcmp(out_res, "Low") == 0)
    {
        A0_OUT10 = ON;
        A1_OUT10 = OFF;
        A0_OUT1 = ON;
        A1_OUT1 = ON;
#ifdef DEBUGG//if debugg is defined putout the message
        UARTSendArray("OUT RES Low");
        UARTSendArray("\r\n");
#endif
    }
    else if(strcmp(out_res, "High") == 0)
    {
        A0_OUT10 = OFF;
        A1_OUT10 = OFF;
        A0_OUT1 = OFF;
        A1_OUT1 = ON;
#ifdef DEBUGG//if debugg is defined putout the message
        UARTSendArray("OUT RES High");
        UARTSendArray("\r\n");
#endif
    }else
    {
        A0_OUT10 = OFF;
        A1_OUT10 = OFF;
        A0_OUT1 = OFF;
        A1_OUT1 = ON;
        UARTSendArray("Command 5 wrong, set Output Resistor to High");
        UARTSendArray("\r\n");
    }
    //setting A0 and A1
    switch(CH)
    {
    case 1:
        if(out_mode == 1)//check if mode is +/- 1V
        {
            MAX7301_setPIN(CH1_A0,A0_OUT1);
            MAX7301_setPIN(CH1_A1,A1_OUT1);
        }
        else if(out_mode == 10)//check if mode is +/- 10V
        {
            MAX7301_setPIN(CH1_A0,A0_OUT10);
            MAX7301_setPIN(CH1_A1,A1_OUT10);
        }
        break;

    case 2:
        if(out_mode == 1)//check if mode is +/- 1V
        {
            MAX7301_setPIN(CH2_A0,A0_OUT1);
            MAX7301_setPIN(CH2_A1,A1_OUT1);
        }
        else if(out_mode == 10)//check if mode is +/- 10V
        {
            MAX7301_setPIN(CH2_A0,A0_OUT10);
            MAX7301_setPIN(CH2_A1,A1_OUT10);
        }
        break;

    case 3:
        if(out_mode == 1)//check if mode is +/- 1V
        {
            MAX7301_setPIN(CH3_A0,A0_OUT1);
            MAX7301_setPIN(CH3_A1,A1_OUT1);
        }
        else if(out_mode == 10)//check if mode is +/- 10V
        {
            MAX7301_setPIN(CH3_A0,A0_OUT10);
            MAX7301_setPIN(CH3_A1,A1_OUT10);
        }
        break;

    case 4:
        if(out_mode == 1)//check if mode is +/- 1V
        {
            MAX7301_setPIN(CH4_A0,A0_OUT1);
            MAX7301_setPIN(CH4_A1,A1_OUT1);
        }
        else if(out_mode == 10)//check if mode is +/- 10V
        {
            MAX7301_setPIN(CH4_A0,A0_OUT10);
            MAX7301_setPIN(CH4_A1,A1_OUT10);
        }
        break;

    case 5:
        if(out_mode == 1)//check if mode is +/- 1V
        {
            MAX7301_setPIN(CH5_A0,A0_OUT1);
            MAX7301_setPIN(CH5_A1,A1_OUT1);
        }
        else if(out_mode == 10)//check if mode is +/- 10V
        {
            MAX7301_setPIN(CH5_A0,A0_OUT10);
            MAX7301_setPIN(CH5_A1,A1_OUT10);
        }
        break;

    case 6:
        if(out_mode == 1)//check if mode is +/- 1V
        {
            MAX7301_setPIN(CH6_A0,A0_OUT1);
            MAX7301_setPIN(CH6_A1,A1_OUT1);
        }
        else if(out_mode == 10)//check if mode is +/- 10V
        {
            MAX7301_setPIN(CH6_A0,A0_OUT10);
            MAX7301_setPIN(CH6_A1,A1_OUT10);
        }
        break;

    case 7:
        if(out_mode == 1)//check if mode is +/- 1V
        {
            MAX7301_setPIN(CH7_A0,A0_OUT1);
            MAX7301_setPIN(CH7_A1,A1_OUT1);
        }
        else if(out_mode == 10)//check if mode is +/- 10V
        {
            MAX7301_setPIN(CH7_A0,A0_OUT10);
            MAX7301_setPIN(CH7_A1,A1_OUT10);
        }
        break;

    case 8:
        if(out_mode == 1)//check if mode is +/- 1V
        {
            MAX7301_setPIN(CH8_A0,A0_OUT1);
            MAX7301_setPIN(CH8_A1,A1_OUT1);
        }
        else if(out_mode == 10)//check if mode is +/- 10V
        {
            MAX7301_setPIN(CH8_A0,A0_OUT10);
            MAX7301_setPIN(CH8_A1,A1_OUT10);
        }
        break;
    }
#ifdef DEBUGG//if debugg is defined putout the message
        UARTSendArray("command_SET: End");
        UARTSendArray("\r\n");
#endif

    if(!strcmp(status,""))//check does it contain an ERROR?
    {
        UARTSendArray("OK \r\n");//ifnot OK
        //Sending Finished
        UARTSendArray("Sending -------------------------> \r\n");
    }
    else
    {
        UARTSendArray("Fail\r\n");//else Fail
        UARTSendArray("see Help for more Information\r\n");
#ifdef DEBUGG//if debugg is defined putout the message
        UARTSendArray(status);
        UARTSendArray("\r\n");
#endif
    }

}

//______________________________________________________________________
//sending Help
void command_Help()
{
    //Sending Example
    UARTSendArray("--------For Example to set Voltage--------------\r\n");
    UARTSendArray("SET_CH1_2.5_OUT10_Low\r\n");
    UARTSendArray("-------------------------------\r\n");
    UARTSendArray("SET\r\n");
    UARTSendArray("^^^_Command SET\r\n");
    UARTSendArray("     CH1\r\n");
    UARTSendArray("     ^^^_Select Channel\r\n");
    UARTSendArray("         2.5\r\n");
    UARTSendArray("         ^^^_Select Output Voltage(-1v <-> +1V) or (-10v <-> +10V)\r\n");
    UARTSendArray("             OUT10\r\n");
    UARTSendArray("             ^^^_Select Output Stage 1 or 10 -> +/-1V or +/-10V \r\n");
    UARTSendArray("                   Low \r\n");
    UARTSendArray("                   ^^^_Select Output Resistor Low = 270 or High = 2.7k\r\n");
    UARTSendArray("#######################################################\r\n\r\n\r\n\r\n");

    UARTSendArray("--------For Example to Configure Ports--------------\r\n");
    UARTSendArray("PORTCONFIGURE_WRITE_28_OUTPUT\r\n");
    UARTSendArray("     ^^^Setting Pin 28  to Output\r\n");

    UARTSendArray("PORTCONFIGURE_WRITE_29_OUTPUT\r\n");
    UARTSendArray("     ^^^Setting Pin 29  to Output\r\n");

    UARTSendArray("PORTCONFIGURE_WRITE_30_OUTPUT\r\n");
    UARTSendArray("     ^^^Setting Pin 30  to Output\r\n");

    UARTSendArray("PORTCONFIGURE_WRITE_31_INPUT\r\n");
    UARTSendArray("     ^^^Setting Pin 31  to Input\r\n");

    UARTSendArray("PORTCONFIGURE_SET_\r\n");
    UARTSendArray("     ^^^Finish Settings\r\n");
    UARTSendArray("#######################################################\r\n\r\n\r\n\r\n");

    UARTSendArray("--------For Example to Setting Output Ports--------------\r\n");

    UARTSendArray("PORTSET_31_HIGH_\r\n");
    UARTSendArray("     ^^^Setting Pin 31 High\r\n");

    UARTSendArray("PORTSET_31_LOW_\r\n");
    UARTSendArray("     ^^^Setting Pin 31 Low\r\n");
    UARTSendArray("#######################################################\r\n\r\n\r\n\r\n");
    UARTSendArray("--------For Example to Setting Output Ports--------------\r\n");

    UARTSendArray("PORTREAD_31_\r\n");
    UARTSendArray("     ^^^Reading Pin 31\r\n");
    UARTSendArray("     ^^^Answer possibility:\r\n");
    UARTSendArray("                 ^^^P31=OFF\r\n");
    UARTSendArray("                 ^^^P31=ON\r\n");
}

//#################################################################
//_____________________Configure Ports as Input or Output
void command_PORTCONFIGURE(char commad2[buflen_cmd], char commad3[buflen_cmd], char commad4[buflen_cmd])
{
#ifdef DEBUGG//if debugg is defined putout the message
        UARTSendArray("Entering Port Conf\r\n");
#endif
        if(strcmp(commad2, "WRITE\0") == 0)//Check if command 2 is WRITE
        {
#ifdef DEBUGG//if debugg is defined putout the message
            UARTSendArray("Entering Port Conf Write\r\n");
#endif
            unsigned char pinnumber = 0;
            pinnumber = atoi(commad3);//safing pin number (28-31)
            if(pinnumber >= 28 && pinnumber <= 31)//check if Pin number is in the range
            {
#ifdef DEBUGG//if debugg is defined putout the message
                UARTSendArray("Entering Port Conf Write Pinnumber\r\n");
#endif
                unsigned char setting = 0;
                if(strcmp(commad4, "OUTPUT\0") == 0)//Check configuration type
                {
#ifdef DEBUGG//if debugg is defined putout the message
                    UARTSendArray("Entering Port Conf Write Pinnumber OUTPUT\r\n");
#endif
                    setting = GPIO_OUTPUT;//safing Configuration type in setting
                }
                else if(strcmp(commad4, "INPUT\0") == 0)//Check configuration type
                {
#ifdef DEBUGG//if debugg is defined putout the message
                    UARTSendArray("Entering Port Conf Write Pinnumber INPUT\r\n");
#endif
                    setting = GPIO_INPUT;//safing Configuration type in setting
                }
                else if(strcmp(commad4, "INPUTWITHPULLUP\0") == 0)//Check configuration type
                {
#ifdef DEBUGG//if debugg is defined putout the message
                    UARTSendArray("Entering Port Conf Write Pinnumber INPUT With PULLUP\r\n");
#endif
                    setting = GPIO_INPUT_with_PULLUP;//safing Configuration type in setting
                }
                else
                {
                    setting = GPIO_INPUT;//if error set to Input
                }
                switch(pinnumber)//switch for each pinnumber
                {
                    case 28://case 28
                        pin28_setting = setting;//safe settings in pin settings
                        break;
                    case 29://case 29
                        pin29_setting = setting;//safe settings in pin settings
                        break;
                    case 30://case 30
                        pin30_setting = setting;//safe settings in pin settings
                        break;
                    case 31://case 31
                        pin31_setting = setting;//safe settings in pin settings
                        break;
                }
            }
        }
        else if(strcmp(commad2, "SET\0") == 0)//Check if command 2 is SET
        {
#ifdef DEBUGG//if debugg is defined putout the message
            UARTSendArray("Entering Port Conf SET\r\n");
#endif
            config_specialPins(pin28_setting, pin29_setting, pin30_setting, pin31_setting);//configure specialpins
            UARTSendArray("Configuration Finished\r\n");
        }
}


//#################################################################
//Setting Outputpins High or Low
void command_PORTSET(char commad2[buflen_cmd], char commad3[buflen_cmd])
{
#ifdef DEBUGG//if debugg is defined putout the message
        UARTSendArray("Entering PORT SET\r\n");
#endif
        unsigned char pinnumber = 0;
        unsigned char pinckeck = 0;
        pinnumber = atoi(commad2);//coping number from command 2
        if(pinnumber >= 28 && pinnumber <= 31)//check if number is between 28 and 31
        {
            switch(pinnumber)
            {
                case 28:
                    pinckeck = pin28_setting;//safe settings in pin settings
                    break;
                case 29:
                    pinckeck = pin29_setting;//safe settings in pin settings
                    break;
                case 30:
                    pinckeck = pin30_setting;//safe settings in pin settings
                    break;
                case 31:
                    pinckeck = pin31_setting;//safe settings in pin settings
                    break;
            }
            if(pinckeck == GPIO_OUTPUT)//check configuration of Pin
            {
                if(strcmp(commad3, "HIGH\0") == 0)//check if command 3 is HIGH
                {
                    UARTSendArray("Setting Pin HIGH\r\n");
                    MAX7301_setPIN(pinnumber,ON);//set pin ON
                }
                if(strcmp(commad3, "LOW\0") == 0)//check if command 3 is LOW
                {
                    UARTSendArray("Setting Pin LOW\r\n");
                    MAX7301_setPIN(pinnumber,OFF);//set pin OFF
                }

            }
        }
}

//#################################################################
//_________________________________Reading Input pin status
void command_PORTREAD(char commad2[buflen_cmd])
{
#ifdef DEBUGG//if debugg is defined putout the message
        UARTSendArray("Entering PORT READ\r\n");
#endif
    unsigned char pinnumber = 0;
    pinnumber = atoi(commad2);//coping number from command 2
    if(pinnumber >= 28 && pinnumber <= 31)//check if number is between 28 and 31
    {
        switch(pinnumber)//Switch between 28 and 31
        {
            case 28:
                if((pin28_setting == GPIO_INPUT) || (pin28_setting == GPIO_INPUT_with_PULLUP))//check configuration of Pin
                {//If pin is active send information fo an active pin back if notsend information for an inactive pin back
                    if(SPIReceive_Input_Max7301(pinnumber)){UARTSendArray("P28=ON\r\n");}else{UARTSendArray("P28=OFF\r\n");}
                }
                break;
            case 29:
                if((pin29_setting == GPIO_INPUT) || (pin29_setting == GPIO_INPUT_with_PULLUP))//check configuration of Pin
                {//If pin is active send information for an active pin back if notsend information for an inactive pin back
                   if(SPIReceive_Input_Max7301(pinnumber)){UARTSendArray("P29=ON\r\n");}else{UARTSendArray("P29=OFF\r\n");}
                }
                break;
            case 30:
                if((pin30_setting == GPIO_INPUT) || (pin30_setting == GPIO_INPUT_with_PULLUP))//check configuration of Pin
                {//If pin is active send information fo an active pin back if notsend information for an inactive pin back
                    if(SPIReceive_Input_Max7301(pinnumber)){UARTSendArray("P30=ON\r\n");}else{UARTSendArray("P30=OFF\r\n");}
                }
                break;
            case 31:
                if((pin31_setting == GPIO_INPUT) || (pin31_setting == GPIO_INPUT_with_PULLUP))//check configuration of Pin
                {//If pin is active send information fo an active pin back if notsend information for an inactive pin back
                    if(SPIReceive_Input_Max7301(pinnumber)){UARTSendArray("P31=ON\r\n");}else{UARTSendArray("P31=OFF\r\n");}
                }
                break;
        }
    }
}
//#################################################################
//______MAX7301_setPin sets pins for GPIO exbander
void MAX7301_setPIN(unsigned char port_pin, unsigned char state)//e.g. MAX7301_setIO(5,1)
{
    char state_checked = 0x00;

    if(state == 0){state_checked = 0x00;}//check if state is bigger than 1
     else{state_checked = 0x01;}

    port_pin = port_pin + 0x20;//add offset (for more information datasheet)
    if((port_pin >= 0x20)&&(port_pin <=0x3F))//check if pin is avaible
    {
        SPISendData_Max7301_2(port_pin,state_checked);//Send two Bytes over SPI
    }


}



//#################################################################
//______set_VOltage_MAX5719 sets Voltage off DAC
void set_Voltage_MAX5719(unsigned char channel, double set_voltage, unsigned char out_mode)//Only for symetric
{

    if(out_mode == 1)
    {
        if((set_voltage > 1) || (set_voltage < -1))//check if Voltage input is in the range of vref
            {
                set_voltage = 0.0;
                //UARTSendArray("-->Wrong Voltage set to 0.0 \r\n");
                strcat(status,"-->Wrong Voltage set to 0.0 \r\n");
            }
        set_voltage *= 4;
    }
    else
    {
        if((set_voltage > 10) || (set_voltage < -10))//check if Voltage input is in the range of vref
            {
                set_voltage = 0.0;
                //UARTSendArray("-->Wrong Voltage set to 0.0 \r\n");
                strcat(status,"-->Wrong Voltage set to 0.0 \r\n");
            }
        set_voltage = set_voltage / 2.5;
    }
    unsigned long out = 0;
    out = (unsigned long)(((set_voltage)+4.096)*2048000); // calulating the Bit value
//-----------------------------------------------------------------------------------------------------------------------------------------------
    unsigned char byte0 = 0;//last byte from the long value
    unsigned char byte1 = 0;//middle byte from the long value
    unsigned char byte2 = 0;//first byte from the long value
    //Sending MSP First last 4 Bits are ignored
    switch(channel)//Sending to the correct Channel
    {
    case 1:
        MAX7301_setPIN(CH1_CS,OFF);//CS off
#ifdef CORRECTION
        if(out_mode == 1)
        {
            set_voltage = ((set_voltage - CH1_AlphaB_1) / CH1_AlphaA_1);
        }
        else
        {
            set_voltage = ((set_voltage - CH1_AlphaB_10) / CH1_AlphaA_10);
        }
#endif
        out = (unsigned long)(((set_voltage)+4.096)*2048000); // calulating the Bit value
        byte0 = (out >> 16);//last byte from the long value
        byte1 = (out >> 8);//middle byte from the long value
        byte2 = (out);//first byte from the long value
        SPISendData_Max5719_3(byte0, byte1, byte2);//send Data over SPI
        MAX7301_setPIN(CH1_CS,ON);//CS on
        break;

    case 2:
        MAX7301_setPIN(CH2_CS,OFF);//CS off
#ifdef CORRECTION
        if(out_mode == 1)

        {
            set_voltage = ((set_voltage - CH2_AlphaB_1) / CH2_AlphaA_1);
        }
        else
        {
            set_voltage = ((set_voltage - CH2_AlphaB_10) / CH2_AlphaA_10);
        }
#endif
        out = (unsigned long)(((set_voltage)+4.096)*2048000); // calulating the Bit value
        byte0 = (out >> 16);//last byte from the long value
        byte1 = (out >> 8);//middle byte from the long value
        byte2 = (out);//first byte from the long value
        SPISendData_Max5719_3(byte0, byte1, byte2);//send Data over SPI
        MAX7301_setPIN(CH2_CS,ON);//CS on
        break;

    case 3:
        MAX7301_setPIN(CH3_CS,OFF);//CS off
#ifdef CORRECTION
        if(out_mode == 1)
        {
            set_voltage = ((set_voltage - CH3_AlphaB_1) / CH3_AlphaA_1);
        }
        else
        {
            set_voltage = ((set_voltage - CH3_AlphaB_10) / CH3_AlphaA_10);
        }
#endif
        out = (unsigned long)(((set_voltage)+4.096)*2048000); // calulating the Bit value
        byte0 = (out >> 16);//last byte from the long value
        byte1 = (out >> 8);//middle byte from the long value
        byte2 = (out);//first byte from the long value
        SPISendData_Max5719_3(byte0, byte1, byte2);//send Data over SPI
        MAX7301_setPIN(CH3_CS,ON);//CS on
        break;

    case 4:
        MAX7301_setPIN(CH4_CS,OFF);//CS off
#ifdef CORRECTION
        if(out_mode == 1)
        {
            set_voltage = ((set_voltage - CH4_AlphaB_1) / CH4_AlphaA_1);
        }
        else
        {
            set_voltage = ((set_voltage - CH4_AlphaB_10) / CH4_AlphaA_10);
        }
#endif
        out = (unsigned long)(((set_voltage)+4.096)*2048000); // calulating the Bit value
        byte0 = (out >> 16);//last byte from the long value
        byte1 = (out >> 8);//middle byte from the long value
        byte2 = (out);//first byte from the long value
        SPISendData_Max5719_3(byte0, byte1, byte2);//send Data over SPI
        MAX7301_setPIN(CH4_CS,ON);//CS on
        break;

    case 5:
        MAX7301_setPIN(CH5_CS,OFF);//CS off
#ifdef CORRECTION
        if(out_mode == 1)
        {
            set_voltage = ((set_voltage - CH5_AlphaB_1) / CH5_AlphaA_1);
        }
        else
        {
            set_voltage = ((set_voltage - CH5_AlphaB_10) / CH5_AlphaA_10);
        }
#endif
        out = (unsigned long)(((set_voltage)+4.096)*2048000); // calulating the Bit value
        byte0 = (out >> 16);//last byte from the long value
        byte1 = (out >> 8);//middle byte from the long value
        byte2 = (out);//first byte from the long value
        SPISendData_Max5719_3(byte0, byte1, byte2);//send Data over SPI
        MAX7301_setPIN(CH5_CS,ON);//CS on
        break;

    case 6:
        MAX7301_setPIN(CH6_CS,OFF);//CS off
#ifdef CORRECTION
        if(out_mode == 1)
        {
            set_voltage = ((set_voltage - CH6_AlphaB_1) / CH6_AlphaA_1);
        }
        else
        {
            set_voltage = ((set_voltage - CH6_AlphaB_10) / CH6_AlphaA_10);
        }
#endif
        out = (unsigned long)(((set_voltage)+4.096)*2048000); // calulating the Bit value
        byte0 = (out >> 16);//last byte from the long value
        byte1 = (out >> 8);//middle byte from the long value
        byte2 = (out);//first byte from the long value
        SPISendData_Max5719_3(byte0, byte1, byte2);//send Data over SPI
        MAX7301_setPIN(CH6_CS,ON);//CS on
        break;

    case 7:
        MAX7301_setPIN(CH7_CS,OFF);//CS off
#ifdef CORRECTION
        if(out_mode == 1)
        {
            set_voltage = ((set_voltage - CH7_AlphaB_1) / CH7_AlphaA_1);
        }
        else
        {
            set_voltage = ((set_voltage - CH7_AlphaB_10) / CH7_AlphaA_10);
        }
#endif
        out = (unsigned long)(((set_voltage)+4.096)*2048000); // calulating the Bit value
        byte0 = (out >> 16);//last byte from the long value
        byte1 = (out >> 8);//middle byte from the long value
        byte2 = (out);//first byte from the long value
        SPISendData_Max5719_3(byte0, byte1, byte2);//send Data over SPI
        MAX7301_setPIN(CH7_CS,ON);//CS on
        break;

    case 8:
        MAX7301_setPIN(CH8_CS,OFF);//CS off
#ifdef CORRECTION
        if(out_mode == 1)
        {
            set_voltage = ((set_voltage - CH8_AlphaB_1) / CH8_AlphaA_1);
        }
        else
        {
            set_voltage = ((set_voltage - CH8_AlphaB_10) / CH8_AlphaA_10);
        }
#endif
        out = (unsigned long)(((set_voltage)+4.096)*2048000); // calulating the Bit value
        byte0 = (out >> 16);//last byte from the long value
        byte1 = (out >> 8);//middle byte from the long value
        byte2 = (out);//first byte from the long value
        SPISendData_Max5719_3(byte0, byte1, byte2);//send Data over SPI
        MAX7301_setPIN(CH8_CS,ON);//CS on
        break;
    }

}



