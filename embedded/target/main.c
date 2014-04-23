#include <msp430.h> 
#include "RF_Toggle_LED_Demo.h"

#define  PACKET_LEN         (0x05)			// PACKET_LEN <= 61
#define  RSSI_IDX           (PACKET_LEN)    // Index of appended RSSI
#define  CRC_LQI_IDX        (PACKET_LEN+1)  // Index of appended LQI, checksum
#define  CRC_OK             (BIT7)          // CRC_OK bit
#define  PATABLE_VAL        (0x51)          // 0 dBm output

extern RF_SETTINGS rfSettings;

unsigned int active;			//Set to 1 if target is currently the one to be shot at otherwise its 0
unsigned int counter;			//Counter for the sensors being high - could also use a timer?

unsigned char TxBuffer[PACKET_LEN]= {0xAA, 0xBB, 0xCC, 0xDD, 0x00};
unsigned char RxBuffer[PACKET_LEN+2];
unsigned char RxBufferLength = 0;

unsigned char transmitting = 0;
unsigned char receiving = 0;
unsigned char targetID;
unsigned char timeStamp;
unsigned char count;

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
    	P2IE = 0;							//Clear the interrupt
    	Sensors();							//Go into the Sensors function
    	__bic_SR_register_on_exit(LPM3_bits); // Exit active
    	break;                         		// P2.2 IFG
    case  8: break;                         // P2.3 IFG
    case 10: break;                         // P2.4 IFG
    case 12: break;                         // P2.5 IFG
    case 14: break;                         // P2.6 IFG
    case 16: break;                         // P2.7 IFG
    default: break;
  }
}

void Sensors(void)
{
	counter = 50;		//Just a guess
	while((P2IN & 0x04) && counter)
	{
		counter = counter - 1;	// When counter equals a certain amount of time indicate target has been hit
	}

	if(!counter)
	{
		LEDs();					//Go to LED method
		active = 0;				//Make target inactive
	}

	return;						//Return back to main method

}

void LEDs(void)
{
	//When this is reached, we know the target is "hit"
	P2OUT &= 0xF8;		//Turn off Red LEDs

	P2OUT |= 0x10;		//Turn on green leds showing target has been hit

	//Indicate hit
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

void Transmit(unsigned char *buffer, unsigned char length)
{
	//Transmit on the antenna here when ready to send data
	RF1AIES |= BIT9;
	RF1AIFG &= ~BIT9;                         // Clear pending interrupts
	RF1AIE |= BIT9;                           // Enable TX end-of-packet interrupt

	WriteBurstReg(RF_TXFIFOWR, buffer, length);

	Strobe( RF_STX );                         // Strobe STX
}

void ReceiveOn(void)
{
	//Receive signals from the peripheral
	RF1AIES |= BIT9;                          // Falling edge of RFIFG9
	RF1AIFG &= ~BIT9;                         // Clear a pending interrupt
	RF1AIE  |= BIT9;                          // Enable the interrupt

	// Radio is in IDLE following a TX, so strobe SRX to enter Receive Mode
	Strobe( RF_SRX );
}

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
    while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
    UCA0TXBUF = 'T';                  // TX -> RXed character
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
    InitRadio();		//Set up the antenna for 915 MHz
    active = 0;			//Target starts as inactive

    //Set up GPIO pins for LEDs and Sensors
    P2DIR &= 0xFB;				//Set Sensors GPIO to an input
    P2DIR |= 0x3A;				//Set LEDs GPIO to outputs

    P2SEL &= 0xC1;				//Set all to be GPIO pins

    //P2OUT &= 0x00;
    //P2OUT |= 0x02;				//Turn on the test LED to see if we made it this far
    //while(1);
    UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCA0CTL1 |= UCSSEL_1;                     // CLK = ACLK
    UCA0BR0 = 0x03;                           // 32kHz/9600=3.41 (see User's Guide)
    UCA0BR1 = 0x00;                           //
    UCA0MCTL = UCBRS_3+UCBRF_0;               // Modulation UCBRSx=3, UCBRFx=0
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

   // uart_puts((char *)"TEST STRING");

    //Set up an interrupt on the sensors when they detect light
    P2IES &= 0xFB;    	//P2IES -> Select interrupt edge: 0 = L to H, 1 = H to L -> Check this with schematic tomorrow
    P2IE |= 0x04;		//P2IE  -> Enable/Disable Interrupt: 0 = disabled, 1 = enabled
    count = 0;
    while(1)
    {
    	while(count < 10)
    	{
    		count++;
    		P2OUT &= 0xC7;

    	}
    	P2OUT |= 0x10;
    	count = 0;

    }
 /*   while(1)
    {
    	//Receive message from peripheral confirming this target is in the game
    	//Send back a confirmation with unique target ID
    	//Receive message about which game is being played and that it is starting
    	//When sensors confirm a hit, send packet of info to peripheral with target ID and timestamp
    	//Then target is inactive until it gets message from peripheral that it is active again
    }*/
}
