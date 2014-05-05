/*
 * TARGET EMBEDDED SOFTWARE - TYLER KUSKE
 * This is for the target boards in order to recognize that its been hit and
 *  to act accordingly when that happens
 */

#include <msp430.h>
#include "RF_Toggle_LED_Demo.h"
#include "validate.h"

#define  PACKET_LEN         (0x05)			// PACKET_LEN <= 61
#define  RSSI_IDX           (PACKET_LEN)    // Index of appended RSSI
#define  CRC_LQI_IDX        (PACKET_LEN+1)  // Index of appended LQI, checksum
#define  CRC_OK             (BIT7)          // CRC_OK bit
#define  PATABLE_VAL        (0x51)          // 0 dBm output
#define  TRIGGER 			26700			// Marks one second on the trigger interrupt
#define  BLUE				(0x20)
#define  GREEN 				(0x10)
#define  RED				(0x08)
#define TARGET					1			//used for type of board
#define  ID						1			//target id
#define TARGET_CTL             12
#define STOP 					5
#define START 					4
#define QUERY					 2
extern RF_SETTINGS rfSettings;

//volatile int pattern;
//volatile int intCount;
volatile unsigned int active;			//Set to 1 if target is currently the one to be shot at otherwise its 0
unsigned int counter;			//Counter for the sensors being high - could also use a timer?

unsigned char TxBuffer[PACKET_LEN]= {0xAA, 0xBB, 0xCC, 0xDD, 0x00};
unsigned char RxBuffer[PACKET_LEN+2];
volatile unsigned int gunCode[8];
unsigned char RxBufferLength = 0;
uint8_t playerID;// used to in SHOT message
uint8_t j;
int8_t receiveLaser = 7;
uint32_t stamp;//used by target
uint16_t respTo;//
unsigned char transmitting = 0;
unsigned char receiving = 0;
unsigned char targetID;
uint16_t timeStamp;
unsigned char count;
volatile bool photo;
volatile unsigned char color;
unsigned char cont;
bool running;//used for game start/stop
uint32_t ledcount;
uint8_t inactive;
uint8_t coun;




void shot(void){
	uart_putc('f');
	uart_putc('u');
	uart_putc(0x0a);//response
	uint8_t byte = currentID >> 8;
	uart_putc(byte);
	byte = currentID && 0xFF;
	uart_putc(byte);
	uart_putc(0x00);
	uart_putc(0x06);//size of payload
	uart_putc(playerID);
	uart_putc(ID);
	byte = stamp >> 24;
	uart_putc(byte);
	byte = (stamp >> 16) & 0xFF;
	uart_putc(byte);
	byte = (stamp >> 8) & 0xFF;
	uart_putc(byte);
	byte = stamp & 0xFF;
	uart_putc(byte);
	uart_putc('y');
	uart_putc('y');
}

volatile int acount;//used for debug, turn target off after hit for 3 seconds.  Then turn target back to active.

//Interrupt vector for timer1 A0
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
	if(coun <=2)
	{
		P2OUT |= color;
		coun++;
	}
	else
	{
		P2OUT &= 0x00;	//Turn off the LEDs - the target has been hit
		inactive = 1;
		coun = 0;
	}


}

//Interrupt vector for timer1 A1
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
	if(ledcount <= 8)
	{
		P2OUT &= 0x00;		//Turn off LEDs
	}
	else
	{
		P2OUT |= color;		//Turn on desired leds showing target has been hit
	}
	ledcount++;
	if(ledcount == 10)
	{
		ledcount = 0;
	}
}

//ADC CONFIGURATION
void adc_config(void)
{
	ADC12CTL0 = ADC12ON + ADC12MSC + ADC12SHT0_10;
	ADC12CTL1 = ADC12SHP + ADC12CONSEQ_3;
	ADC12MCTL0 = ADC12INCH_0;
	ADC12MCTL1 = ADC12INCH_1;
	ADC12MCTL2 = ADC12INCH_2 + ADC12EOS;
	ADC12IE = 0x4;
	ADC12CTL0 |= ADC12ENC;
}



//Interrupt for Port 2 bit that relates to the sensor,
// We want to interrupt when the sensors detect light
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
  switch(__even_in_range(P2IV, 16))
  {
    case  0: break;
    case  2: break;                         // P2.0 IFG
    case  4: break;                         // P2.1 IFG
    case  6:
    	break;                         		// P2.2 IFG
    case  8: break;                         // P2.3 IFG
    case 10: break;                         // P2.4 IFG
    case 12: break;                         // P2.5 IFG
    case 14:
    	P2IE = 0;		//gun 1 hit

    	TA1CTL = TASSEL_1 + MC_1 + TACLR;//start timer to turn color on for a second
    	color = BLUE;
    	LEDs();
    	__bic_SR_register_on_exit(LPM3_bits); // Exit active
    	break;                         // P2.6 IFG
    case 16:
    	P1IFG &= 0xBF;
    	P2IE = 0;		//gun2 hit
    	color = GREEN;
    	TA1CTL = TASSEL_1 + MC_1 + TACLR;//start timer to turn color on for a second
    	LEDs();
    	__bic_SR_register_on_exit(LPM3_bits); // Exit active
    	break;                         // P2.7 IFG
    default: break;
  }
}

void Sensors(void)
{
/*	counter = 100;		//Just a guess
	while((P2IN & 0x04) && counter)
	{
		counter = counter - 1;	// When counter equals a certain amount of time indicate target has been hit
	}

	if(!counter)
	{
		LEDs();					//Go to LED method
		//active = 0;				//Make target inactive
	}

	return;						//Return back to main method*/

}

void LEDs(void)
{
	//When this is reached, we know the target is "hit"

	P2OUT &= 0x00;		//Turn off LEDs
	P2OUT |= color;		//Turn on desired leds showing target has been hit
	return;
}


void uart_putc(unsigned char c)
{
	while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
	    UCA0TXBUF = c;                  // TX -> RXed character
}
void uart_puts(const char *str)
{
     while(*str) uart_putc(*str++);
}

// Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
  switch(__even_in_range(UCA0IV,4))
  {
  case 0:break;                             // Vector 0 - no interrupt
  case 2:                                   // Vector 2 - RXIFG
    while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
    	UCA0TXBUF = UCA0RXBUF;                  // TX -> RXed character
    break;
  case 4:break;                             // Vector 4 - TXIFG
  default: break;
  }
}

//Interrupt for the ADC
#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
  switch(__even_in_range(ADC12IV,34))
  {
  case  0: break;                           // Vector  0:  No interrupt
  case  2: break;                           // Vector  2:  ADC overflow
  case  4: break;                           // Vector  4:  ADC timing overflow
  case  6:                                  // Vector  6:  ADC12IFG0
    break;
  case  8:
	    break;                           // Vector  8:  ADC12IFG1
  case 10:      if (ADC12MEM0 >= 0x100 || ADC12MEM1 >= 0x100 || ADC12MEM2 >= 0x100){                 // ADC12MEM = A0 > 0.5AVcc?
	  	  color = GREEN;
	  	  TA0CTL = TASSEL_1 + MC_1 + TACLR;
	  	  LEDs();
	  	ADC12CTL0 &= !ADC12SC;                   // Stop sampling/conversion
	  	TA1CTL |= 0x0010;				//Start the timer
    }
    break;                           // Vector 10:  ADC12IFG2
  case 12: break;                           // Vector 12:  ADC12IFG3
  case 14: break;                           // Vector 14:  ADC12IFG4
  case 16: break;                           // Vector 16:  ADC12IFG5
  case 18: break;                           // Vector 18:  ADC12IFG6
  case 20: break;                           // Vector 20:  ADC12IFG7
  case 22: break;                           // Vector 22:  ADC12IFG8
  case 24: break;                           // Vector 24:  ADC12IFG9
  case 26: break;                           // Vector 26:  ADC12IFG10
  case 28: break;                           // Vector 28:  ADC12IFG11
  case 30: break;                           // Vector 30:  ADC12IFG12
  case 32: break;                           // Vector 32:  ADC12IFG13
  case 34: break;                           // Vector 34:  ADC12IFG14
  default: break;
  }
  __bic_SR_register_on_exit(LPM0_bits);   // Exit active CPU
}


//GPIO CONFIGURATION
void gpio_config(void)
{
    P2OUT &= 0x00;		//Clear the output register for Port 2
    //Set up GPIO pins for LEDs and Sensors (0 for input, 1 for output)
    P2DIR &= 0x00;				//Set Sensors GPIO to an input
   // P2DIR |= 0x3A;				//Set LEDs GPIO to outputs
    P2DIR |= 0x38;			//FOR TESTING

   // P2SEL &= 0xC1;				//Set all to be GPIO pins - 0 for GPIO
    P2SEL &= 0x01;		//FOR TESTING
    P2SEL |= 0x07;

    P2IE |= 0xC0;		//FOR TESTING
    P2IES &= 0x3F;		//FOR TESTING

    count = 0;
    P2OUT = 0x00;		//Turn LEDs red


    P2SEL |= BIT0;                            // P2.0 ADC option select
    P1DIR |= BIT0;                            // P1.0 output
}

//UART CONFIGURATION
void uart_config(void)
{
    PMAPPWD = 0x02D52;                        // Get write-access to port mapping regs
    P1MAP5 = PM_UCA0RXD;                      // Map UCA0RXD output to P1.5
    P1MAP6 = PM_UCA0TXD;                      // Map UCA0TXD output to P1.6
    PMAPPWD = 0;                              // Lock port mapping registers

    P1DIR |= BIT6;                            // Set P1.6 as TX output
    P1SEL |= BIT5 + BIT6;                     // Select P1.5 & P1.6 to UART function

    UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCA0CTL1 |= UCSSEL_1;                     // CLK = ACLK
    UCA0BR0 = 0x03;                           // 32kHz/9600=3.41 (see User's Guide)
    UCA0BR1 = 0x00;                           //
    UCA0MCTL = UCBRS_3+UCBRF_0;               // Modulation UCBRSx=3, UCBRFx=0
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}

//TIMER 1 CONFIGURATION
void timer_config(void)
{
    TA1CCTL0 = CCIE;                          // CCR0 interrupt enabled
    TA1CCR0 = 50000;
    TA1CTL = TASSEL_1 + MC_0 + TACLR; //Start with timer halted, turn on at target hit//MC_1;// + TACLR;         // SMCLK, upmode, clear TAR


    TA0CCTL0 = CCIE;                          // CCR0 interrupt enabled
    TA0CCR0 = 50; //1000;//50000;//about once a second
    TA0CTL = TASSEL_1 + MC_0 + TACLR;
}
/*
 * main.c
 */
void response(void){
	uart_putc('f');
	uart_putc('u');
	uart_putc(0x01);//response
	uint8_t byte = currentID >> 8;
	uart_putc(byte);
	byte = currentID && 0xFF;
	uart_putc(byte);
	uart_putc(0x00);
	uart_putc(0x03);
	byte = respTo >> 8;
	uart_putc(byte);
	byte = respTo & 0xff;
	uart_putc(byte);
	uart_putc(0);//Code is OK
	uart_putc('y');
	uart_putc('y');

}
void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer


    // Increase PMMCOREV level to 2 in order to avoid low voltage error
    // when the RF core is enabled
    SetVCore(2);
    ResetRadioCore();

   __bis_SR_register(GIE);       				// Enter LPM0, Enable interrupts
     __no_operation();                         	// For debugger


    count = 0;
    P2OUT = 0;					//Turn LEDs off

    uart_config();				//Configure the UART
    gpio_config();				//Configure the GPIOs
    adc_config();				//Configure the ADC
    timer_config();				//Configure the timer

    //Assign unique target ID to each target
    TA0CTL |= 0x0010;
    stamp = 250;//for debug;
    respTo = 4;//for debug;
    color = RED;
    LEDs();
    active = true;

    while(1)
    {
    	while(active)
    	{
    	uint8_t recChar = UCA0RXBUF; //grab next byte in uart.
    	if(onChar(recChar)){
    		//color = RED;
    		//LEDs();
    	//valid, check if message is start, stop, querey, targetctl.
    		switch (type) {
    			case QUERY: //2
    				if(payloadLength == 2 && buffer[7] == ID && buffer[8] == TARGET){
    					//query is valid.
    					response();//daugther expects a response
    					//continue, don't know how to "respond To"
    				}
    				break;
    			case START://4
    				if(payloadLength == 0){
    					running = true;//daughter does not expect a response
    				}
    				break;
    			case STOP://5
    				if(payloadLength == 0){
    					running = false;//daughter does not expect a response
    				}
    				break;
    			case TARGET_CTL://12
    				  if(payloadLength >= 2 && buffer[7] == ID){//&& string[7] == thisID
    					  active = buffer[8];
    					  if(active){
    						  color = RED;//RED;
    						  LEDs();
    					  }
    				  }
    				  if(payloadLength >= 4 && buffer[9] == ID){//&& string[9] == thisID
    					  active = buffer[10];
    					  if(active){
    						  color = RED;
    					  	LEDs();
    					  }
    				  }
    				  if(payloadLength == 6 && buffer[11] == ID){//&& string[11] == thisID
    					 active = buffer[11];
    					 if(active){
    						 color = RED;
    						 LEDs();
    					 }
    				  }
    				  response();//daughter expects a response
    			}
    	}
    	if(active)
    	{
        	ADC12CTL0 |= ADC12SC;                   // Start sampling/conversion

     //    __bis_SR_register(GIE);     // LPM0, ADC12_ISR will force exit
     //     __no_operation();                       // For debugger
            color = RED;//RED;
    		P2IE |= 0xC0;//turns on shot recieved interrupt
    		LEDs();
    	}
    	//Respond to 'Are you there?' message from daughter board
    		//Respond by matching targets ID to the message ID
    	//Get message telling when to be turned on -> send confirmation back

    }
}
}
