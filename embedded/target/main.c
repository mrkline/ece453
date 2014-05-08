/*
 * TARGET EMBEDDED SOFTWARE - TYLER KUSKE
 * This is for the target boards in order to recognize that its been hit and
 *  to act accordingly when that happens
 */

#include <msp430.h>
#include "RF_Toggle_LED_Demo.h"
#include "validate.h"
#include <stdlib.h>

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
#define QUERY					2

extern RF_SETTINGS rfSettings;				//Settings for the wireless radio

volatile unsigned int active;				//Set to 1 if target is currently the one to be shot at otherwise its 0
unsigned int counter;						//Counter for the sensors being high - could also use a timer?

unsigned char TxBuffer[PACKET_LEN]= {0xAA, 0xBB, 0xCC, 0xDD, 0x00};	//Wireless transmit buffer
unsigned char RxBuffer[PACKET_LEN+2];		//Receive buffer for the wireless radio
volatile unsigned int gunCode[8];			//Strobed pattern from the laser to be read in
unsigned char RxBufferLength 		= 0;	//Receive buffer length from the wireless radio
uint8_t playerID;							// used to in SHOT message
uint8_t j;
int8_t receiveLaser 				= 7;	//Used to time the receiving of the pattern of the laser
uint32_t stamp;								//used by target
uint16_t respTo;							//
unsigned char transmitting 			= 0;	//Asserted when the wireless radio is transmitting
unsigned char receiving 			= 0;	//Asserted when the wireless radio is receiving
unsigned char targetID;						//Unique ID for each target (Hard coded in)
uint16_t timeStamp;							//Tracks time for certain events
unsigned char count;						//Counter
volatile bool hitInt;
volatile unsigned char color;				//Color of LEDs to be displayed
unsigned char cont;
bool running;								//used for game start/stop
uint32_t ledcount;
volatile int acount;						//used for debug, turn target off after hit for 3 seconds.  Then turn target back to active.

/* THIS METHOD CONSTRUCTS A SHOT MESSAGE TO SEND TO DAUGHTER CARD WIRELESSLY
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
	uart_putc(playerID);//always gonna be 1, only one gun :(
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
*/


//Interrupt vector for timer1 A0
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
	if(rand()%5 == 0 && !active)
	{
		active = true;
	}

	acount++;
	color 	= 0;			//Clear the color
	P2OUT 	= 0x00;			//Turn off the LEDs - the target has been hit
	cont 	= 1;

	if(acount > 3)
	{
		active 	= 1;							//turn target back on, for debug testing
		hitInt 	= 0;							//
		color	= 0;							//Clear current color
		LEDs();									//Call method to set LEDs color
		acount 	= 0;							//Reset counter
		TA1CTL 	= TASSEL_1 + MC_1 + TACLR;		//reset timer
		P2IFG &= 0x3F;							//clears any "bounce" interrupts for gun hits
	}
}

//Interrupt vector for timer0 A0
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
	//Pulse width modulating LEDs for easier viewing
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
	ADC12CTL0 	= ADC12ON + ADC12MSC + ADC12SHT0_10;
	ADC12CTL1 	= ADC12SHP + ADC12CONSEQ_3;
	ADC12MCTL0 	= ADC12INCH_0;
	ADC12MCTL1 	= ADC12INCH_1;
	ADC12MCTL2 	= ADC12INCH_2 + ADC12EOS;
	ADC12IE 	= 0x4;
	ADC12CTL0  |= ADC12ENC;
}

//Interrupt for Port 2 bit that relates to the sensor,
// We want to interrupt when the sensors detect light
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
  switch(__even_in_range(P2IV, 16))
  {
    case  0: break;
    case  2: break;                         	// P2.0 IFG
    case  4: break;                         	// P2.1 IFG
    case  6:

    	__bic_SR_register_on_exit(LPM3_bits); 	// Exit active
    	break;                         			// P2.2 IFG
    case  8: break;                         	// P2.3 IFG
    case 10: break;                         	// P2.4 IFG
    case 12: break;                         	// P2.5 IFG
    case 14:
    	//For testing/debugging, shows how target reacts to a
    	// different gun pattern being read in
    	P2IE 	= 0;							//gun 1 hit

    	TA1CTL 	= TASSEL_1 + MC_1 + TACLR;		//start timer to turn color on for a second
    	color 	= BLUE;							//Set color of LEDs to BLUE
    	LEDs();									//Go change the color of the LEDs
    	__bic_SR_register_on_exit(LPM3_bits); 	// Exit active
    	break;                         			// P2.6 IFG
    case 16:

    	P1IFG  &= 0xBF;							//Reset the interrupt flag
    	P2IE 	= 0;							//gun2 hit
    	color 	= GREEN;						//Set the color
    	TA1CTL 	= TASSEL_1 + MC_1 + TACLR;		//start timer to turn color on for a second
    	LEDs();									//Set color of LEDs
    	__bic_SR_register_on_exit(LPM3_bits); 	// Exit active
    	break;                         			// P2.7 IFG
    default: break;
  }
}

void Sensors(void)
{
	counter = 100;		//Just a guess
	while((P2IN & 0x04) && counter)
	{
		counter = counter - 1;	// When counter equals a certain amount of time indicate target has been hit
	}

	if(!counter)
	{
		//LEDs();					//Go to LED method
		//active = 0;				//Make target inactive
	}

	return;						//Return back to main method

}

//Turns LEDs to the correct color
void LEDs(void)
{
	P2OUT &= 0x00;				//Turn off LEDs
	P2OUT |= color;				//Turn on desired leds showing target has been hit
	return;
}

//Function to have high power mode enabled for the radio, write the correct settings
// to the radio and set up the PA table values
void InitRadio(void)
{
  // Set the High-Power Mode Request Enable bit so LPM3 can be entered
  // with active radio enabled
  PMMCTL0_H = 0xA5;
  PMMCTL0_L |= PMMHPMRE_L;
  PMMCTL0_H = 0x00;

  WriteRfSettings(&rfSettings);

  WriteSinglePATable(PATABLE_VAL);
}

//Transmits data across the wireless radio
void Transmit(unsigned char *buffer, unsigned char length)
{
	//Transmit on the antenna here when ready to send data
	RF1AIES |= BIT9;
	RF1AIFG &= ~BIT9;                         // Clear pending interrupts
	RF1AIE |= BIT9;                           // Enable TX end-of-packet interrupt

	WriteBurstReg(RF_TXFIFOWR, buffer, length);

	Strobe( RF_STX );                         // Strobe STX
}

//Turn on the receive function of the wireless radio
void ReceiveOn(void)
{
	//Receive signals from the peripheral
	RF1AIES |= BIT9;                          // Falling edge of RFIFG9
	RF1AIFG &= ~BIT9;                         // Clear a pending interrupt
	RF1AIE  |= BIT9;                          // Enable the interrupt

	// Radio is in IDLE following a TX, so strobe SRX to enter Receive Mode
	Strobe( RF_SRX );
}

//Turn off the receive function of the wireless radio
void ReceiveOff(void)
{
	//Turn off Receive function when it is not needed
	RF1AIE &= ~BIT9;                          // Disable RX interrupts
	RF1AIFG &= ~BIT9;                         // Clear pending IFG

	// It is possible that ReceiveOff is called while radio is receiving a packet.
	// Therefore, it is necessary to flush the RX FIFO after issuing IDLE strobe
	// such that the RXFIFO is empty prior to receiving a packet.
	Strobe( RF_SIDLE );
	Strobe( RF_SFRX  );
}

//Send character via UART
void uart_putc(unsigned char c)
{
	while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
	    UCA0TXBUF = c;                  // TX -> RXed character
}

//Send string via UART
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
    	UCA0TXBUF = UCA0RXBUF;              // TX -> RXed character
    break;
  case 4:break;                             // Vector 4 - TXIFG
  default: break;
  }
}

//Interrupt for sending data across wireless radio
#pragma vector=CC1101_VECTOR
__interrupt void CC1101_ISR(void)
{
  switch(__even_in_range(RF1AIV,32))        // Prioritizing Radio Core Interrupt
  {
    case  0: break;                         // No RF core interrupt pending
    case  2: break;                         // RFIFG0
    case  4: break;                         // RFIFG1
    case  6: break;                         // RFIFG2
    case  8: break;                         // RFIFG3
    case 10: break;                         // RFIFG4
    case 12: break;                         // RFIFG5
    case 14: break;                         // RFIFG6
    case 16: break;                         // RFIFG7
    case 18: break;                         // RFIFG8
    case 20:                                // RFIFG9
      if(receiving)			    			// RX end of packet
      {
    	  P2OUT &= 0xF7;					//Turn off the red LEDs
    	  P2OUT |= 0x20;					//Turn on blue LEDs

    	// Read the length byte from the FIFO
        RxBufferLength = ReadSingleReg( RXBYTES );
        ReadBurstReg(RF_RXFIFORD, RxBuffer, RxBufferLength);

        // Stop here to see contents of RxBuffer
        __no_operation();
      }
      else if(transmitting)		    		// TX end of packet
      {
    	  P2OUT &= 0xF7;					//Light the diode
    	  RF1AIE &= ~BIT9;                  // Disable TX end-of-packet interrupt
    	  transmitting = 0;
      }
      else while(1); 			    		// trap
      break;
    case 22: break;                         // RFIFG10
    case 24: break;                         // RFIFG11
    case 26: break;                         // RFIFG12
    case 28: break;                         // RFIFG13
    case 30: break;                         // RFIFG14
    case 32: break;                         // RFIFG15
  }
  __bic_SR_register_on_exit(LPM3_bits);
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
  case 10:
	  //Check the AC values of all three sensors to see if they are tripped by light
	  if (ADC12MEM0 >= 0x100 || ADC12MEM1 >= 0x100 || ADC12MEM2 >= 0x100)
	  {
		  color 	= GREEN;					//Color for LEDs is green
		  hitInt 	= 1;						//Indicate sensors have been tripped
		  TA1CTL 	= TASSEL_1 + MC_1 + TACLR;	//turns timer active
		  active 	= 0;
		  LEDs();								//Change color of LEDs
    }

    break;                          	 	// Vector 10:  ADC12IFG2
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
	//Set up GPIO pins for LEDs and Sensors (0 for input, 1 for output)
    P2OUT &= 0x00;				//Clear the output register for Port 2
    P2DIR &= 0x00;				//Set Sensors GPIO to an input
    P2DIR |= 0x38;				//Set LEDs GPIO to outputs

    //Set all to be GPIO pins - 0 for GPIO
    P2SEL &= 0x01;
    P2SEL |= 0x07;

    //Set up an interrupt on the sensors when they detect light
    P2IE |= 0xC0;
    P2IES &= 0x3F;

    count = 0;

    P2SEL |= BIT0;              // P2.0 ADC option select
    P1DIR |= BIT0;              // P1.0 output
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
    TA1CCTL0 	= CCIE;                          // CCR0 interrupt enabled
    TA1CCR0 	= 40000;						 //Set value timer counts up to
    TA1CTL 		= TASSEL_1 + MC_0 + TACLR; 		 //Start with timer halted, turn on at target hit

    TA0CCTL0 	= CCIE;                          // CCR0 interrupt enabled
    TA0CCR0 	= 30;							 //Set value timer counts up to
    TA0CTL 		= TASSEL_1 + MC_2 + TACLR;		 //Start with timer halted, continuous counting
}
/* RESPONSE MESSAGE CONSTRUCTION TO SEND BACK TO DAUGHTER BOARD
void response(void){
	uart_putc('f');
	uart_putc('u');
	uart_putc(0x01);//response
	uint8_t byte = currentID >> 8;//currentID is the message id generated by the target
	uart_putc(byte);
	byte = currentID && 0xFF;
	uart_putc(byte);
	uart_putc(0x00);
	uart_putc(0x03);
	byte = respTo >> 8;
	uart_putc(byte);
	byte = respTo & 0xff;//currentID recieved by the zed
	uart_putc(byte);
	uart_putc(0);//Code is OK
	uart_putc('y');
	uart_putc('y');

}
*/

/*
 * main.c
 */
void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    // Increase PMMCOREV level to 2 in order to avoid low voltage error
    // when the RF core is enabled
    SetVCore(2);
    ResetRadioCore();
    InitRadio();		//Set up the antenna for 915 MHz

    count = 0;
    P2OUT = 0;					//Clear the Port 2 outputs (LEDs)

    uart_config();				//Configure the UART
    gpio_config();				//Configure the GPIOs
    adc_config();				//Configure the ADC
    timer_config();				//Configure the timer

    stamp = 250;				//for debug;
    respTo = 4;					//for debug;

    active = true;				//Target is active for this game
    srand(3);
    hitInt 		= 0;			//Target not hit yet
    ledcount 	= 0;			//Reset counter
    TA0CTL 	   |= 0x0010;		//Start the timer
    while(1)
    {
    	if(active){
    		hitInt 		= 0;						//Target has not been hit yet
    		acount 		= 0;						//Reset counter
        	ADC12CTL0  |= ADC12SC;                  // Start sampling/conversion

            __bis_SR_register(LPM0_bits + GIE);     // LPM0, ADC12_ISR will force exit
            __no_operation();                       // For debugger
    		color 		= RED;						//LEDs to be red
    		TA1CTL 		= TASSEL_1 + MC_0 + TACLR;	//turn off timer
    	}
    }
}
    	/*CODE BELOW IS FOR WIRELESS RADIO COMMUNICATION BETWEEN DAUGHTER BOARD & TARGET BOARD
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

            __bis_SR_register(LPM0_bits + GIE);     // LPM0, ADC12_ISR will force exit
            __no_operation();                       // For debugger
            color = RED;//RED;
    		P2IE |= 0xC0;//turns on shot recieved interrupt
    		LEDs();
    	}
    	//Respond to 'Are you there?' message from daughter board
    		//Respond by matching targets ID to the message ID
    	//Get message telling when to be turned on -> send confirmation back

    }
}
*/

    /*
    while(1)
    {
    	if(active)
    	{
    		color = RED;
    		P2IE |= 0xC0;//turns on shot recieved interrupt
    		LEDs();
    	}
    	//Respond to 'Are you there?' message from daughter board
    		//Respond by matching targets ID to the message ID
    	//Get message telling when to be turned on -> send confirmation back
    	ADC12CTL0 |= ADC12SC;                   // Start sampling/conversion

        __bis_SR_register(LPM0_bits + GIE);     // LPM0, ADC12_ISR will force exit
        __no_operation();                       // For debugger

    }
}
*/
