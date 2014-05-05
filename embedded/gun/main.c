///////////////////////////////////////////////////////////////////////////////
/*GUN MODULE
 * This module is the software loaded onto the guns. It will take care of the
 * GPIO functions for the laser, buzzer and button. The button will operate
 * through an interrupt and modify the buzzer & laser accordingly. The gun
 * module also needs to transmit & receive messages with the daughter board.
 *
 */
#include <msp430.h> 
#include <stdlib.h>
#include "RF_Toggle_LED_Demo.h"

#define  PACKET_LEN         (0x05)			// PACKET_LEN <= 61
#define  RSSI_IDX           (PACKET_LEN)    // Index of appended RSSI
#define  CRC_LQI_IDX        (PACKET_LEN+1)  // Index of appended LQI, checksum
#define  CRC_OK             (BIT7)          // CRC_OK bit
#define  PATABLE_VAL        (0x51)          // 0 dBm output
#define  GUN_CODE			(0xAA)
extern RF_SETTINGS rfSettings;

volatile unsigned char info;

unsigned int buzz;
unsigned char RxBuffer[PACKET_LEN+2];
unsigned char RxBufferLength = 0;

unsigned char transmitting = 0;
unsigned char receiving = 0;
unsigned char gunID;				//Hard coded gun ID to distinguish between the guns when sending messages
unsigned char laser;				//boolean for laser on or off
unsigned char rounds;				//Duration of game being played
unsigned laserDuration;				//How long the laser stays on after a trigger pull
unsigned char r;					//The character received from the RX uart buffer
uint16_t timeStamp;				//Time of each shot that is taken -> Change to 16 bit int
volatile acount;
unsigned char *time_stamp;
uint32_t numShots = 0;			//Total shots this gun has taken

const unsigned char TxBuffer[PACKET_LEN]= {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
const unsigned char message[100];

//Interrupt vector for timer1 A0
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{

}

//Interrupt vector for timer1 A0
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{

}

void uart_putc(unsigned char c)
{
	while (!(UCA0IFG & UCTXIFG));             // USCI_A0 TX buffer ready?
	    UCA0TXBUF = c;                  // TX -> RXed character
}
void uart_puts(const char *str)
{
     while(*str) uart_putc(*str++);
}

void uart_getc(void)
{
	while(!(UCA0IFG & UCRXIFG));
		r = UCA0RXBUF;

}

//If the trigger is pulled, we will go to this method to take care of the
// buzzer and laser functionality
void TriggerPull(void)
{

	timeStamp = TA0R;//When trigger is pulled, get the timer value and put into variable timeStamp
	//Append timeStamp to the time stamp string
	laserDuration = 0;
	while(laserDuration != 5000)
	{
		P2OUT |= 0x01;		//Turn on the laser
		P2OUT ^= 0x04;		//Toggle the buzzer
		laserDuration++;
	}
	P2OUT &= 0xFE;
	return;
}


//Interrupt for when the trigger is pulled
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
  switch(__even_in_range(P2IV, 16))
  {
    case  0: break;
    case  2: break;                         // P2.0 IFG

    case  4:                          		// P2.1 IFG
	TA1CTL = TASSEL_1 + MC_1 + TACLR;//start timer to turn color on for a second
    P2IFG &= 0xFD;
    TriggerPull();
    numShots = numShots + 1;			//NOT SURE ABOUT THIS LOGIC
    __bic_SR_register_on_exit(LPM3_bits); // Exit active
    break;

    case  6: break;                         // P2.2 IFG
    case  8: break;                         // P2.3 IFG
    case 10: break;                         // P2.4 IFG
    case 12: break;                         // P2.5 IFG
    case 14: break;                         // P2.6 IFG
    case 16: break;                         // P2.7 IFG
    //default: break;
  }
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


/*
 * main.c
 */
void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    // Increase PMMCOREV level to 2 in order to avoid low voltage error
    // when the RF core is enabled
    SetVCore(2);
    ResetRadioCore();

    //Map registers for UART and wireless
    PMAPPWD = 0x02D52;                        // Get write-access to port mapping regs
    P1MAP5 = PM_UCA0RXD;                      // Map UCA0RXD output to P1.5
    P1MAP6 = PM_UCA0TXD;                      // Map UCA0TXD output to P1.6
    PMAPPWD = 0;                              // Lock port mapping registers

    P1DIR |= BIT6;                            // Set P1.6 as TX output
    P1SEL |= BIT5 + BIT6;                     // Select P1.5 & P1.6 to UART function

    //UART CONFIGURATION
    UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCA0CTL1 |= UCSSEL_1;                     // CLK = ACLK
    UCA0BR0 = 0x03;                           // 32kHz/9600=3.41 (see User's Guide)
    UCA0BR1 = 0x00;                           //
    UCA0MCTL = UCBRS_3+UCBRF_0;               // Modulation UCBRSx=3, UCBRFx=0
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

    TA1CCTL0 = CCIE;                          // CCR0 interrupt enabled
    TA1CCR0 = 500;						//1000;//50000;//about once a second
    TA1CTL = TASSEL_1 + MC_0 + TACLR; //Start with timer halted, turn on at target hit//MC_1;// + TACLR;         // SMCLK, upmode, clear TAR

    TA0CCR0 = 100;                 // Count limit (16 bit)
    TA0CCTL0 = 0x10;                 // Enable Timer A1 interrupts, bit 4=1
    TA0CTL = TASSEL_1 + MC_2 + TACLR;        // Timer A1 with ACLK, count UP  TODO CHANGE to MC_1

   __bis_SR_register(GIE);       				// Enter LPM0, Enable interrupts
   __no_operation();                         	// For debugger

    //Make pins 2.2(buzzer) & 2.0(laser) outputs and pin 2.1(button) an input
    P2DIR &= 0xFD;
    P2DIR |= 0x0D;

    //Pins 2.0, 2.1, and 2.2 selected to be GPIOs
    P2SEL &= 0xF0;

    //INTERRUPT FOR PUSH BUTTON
    P2IES &= 0xFD;    	//P2IES -> Select interrupt edge: 0 = L to H, 1 = H to L
    P2IE |= 0x02;		//P2IE  -> Enable/Disable Interrupt: 0 = disabled, 1 = enabled
    P2OUT &= 0xFE;		//Turn off laser

    while(1)
    {
  /*  	uint8_t recChar = UCA0RXBUF; //grab next byte in uart.
    	if(onChar(recChar)){
    	//valid, check if message is start, stop, querey, targetctl.
    		switch (type) {
    			case QUERY:
    				if(payloadLength == 2 && buffer[7] == ID && buffer[8] == TARGET){
    					//query is valid.
    					uart_putc('f');
    					uart_putc('u');
    					uint8_t byte = currentID >> 8;
    					uart_putc(byte);
    					byte = currentID && 0xFF;
    					uart_putc(byte);
    					uart_putc(0x00);
    					uart_putc(0x03);
    					//continue, don't know how to "respond To"
    				}
    				break;
    			case START:
    				if(payloadLength == 0){
    					running = true;
    				}
    				break;
    			case STOP:
    				if(payloadLength == 0){
    					running = false;
    				}
    				break;
    			case TARGET_CTL:
    				  if(payloadLength >= 2 && buffer[7] == ID){//&& string[7] == thisID
    					  active = buffer[8];
    				  }
    				  if(payloadLength >= 4 && buffer[9] == ID){//&& string[9] == thisID
    					  active = buffer[10];
    				  }
    				  if(payloadLength == 6 && buffer[11] == ID){//&& string[11] == thisID
    					 active = buffer[11];
    				  }
    				  //response;
    			}*/
    	}

    	//P2OUT &= 0xFE;		//Turn off laser


    //Assign a unique gun ID to the gun
  /*  while(1)
    {
    	numShots = 0;			//Reset value of number of shots
	//Gets an 'Are you there?' message
	 //Check ID and match to gun ID -> acknowledgment sent back to confirm

	//Gets a message that round is over
	 //Respond with a shot message for each message
    		info = gunID;	//Send peripheral confirmation that this gun is in the game - send its unique id
    		TxBuffer[PACKET_LEN - 2] = info;

    		//Gun then will wait for next message from peripheral - End of Game
    		info = numShots;//When message received, transmit packet with number of shots fired with unique gun ID
    		TxBuffer[PACKET_LEN - 2] = info;
    		Transmit((unsigned char*)TxBuffer, sizeof TxBuffer);
    		transmitting = 1;

    }*/
}

