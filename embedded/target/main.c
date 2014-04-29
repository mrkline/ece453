/*
 * TARGET EMBEDDED SOFTWARE - TYLER KUSKE
 * This is for the target boards in order to recognize that its been hit and
 *  to act accordingly when that happens
 */

#include <msp430.h>
#include "RF_Toggle_LED_Demo.h"

#define  PACKET_LEN         (0x05)			// PACKET_LEN <= 61
#define  RSSI_IDX           (PACKET_LEN)    // Index of appended RSSI
#define  CRC_LQI_IDX        (PACKET_LEN+1)  // Index of appended LQI, checksum
#define  CRC_OK             (BIT7)          // CRC_OK bit
#define  PATABLE_VAL        (0x51)          // 0 dBm output
#define  TRIGGER 			26700			// Marks one second on the trigger interrupt
#define  BLUE				(0x20)
#define  GREEN 				(0x10)
#define  RED				(0x08)
extern RF_SETTINGS rfSettings;

volatile unsigned int active;			//Set to 1 if target is currently the one to be shot at otherwise its 0
unsigned int counter;			//Counter for the sensors being high - could also use a timer?

unsigned char TxBuffer[PACKET_LEN]= {0xAA, 0xBB, 0xCC, 0xDD, 0x00};
unsigned char RxBuffer[PACKET_LEN+2];
unsigned char RxBufferLength = 0;

unsigned char transmitting = 0;
unsigned char receiving = 0;
unsigned char targetID;
uint16_t timeStamp;
unsigned char count;
volatile unsigned char color;
unsigned char cont;
uint32_t ledcount;
<<<<<<< HEAD

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
    if (ADC12MEM0 >= 0x7ff)                 // ADC12MEM = A0 > 0.5AVcc?
      //P1OUT |= BIT0;                        // P1.0 = 1
    	LEDs();
    else
      //P1OUT &= ~BIT0;                       // P1.0 = 0

    __bic_SR_register_on_exit(LPM0_bits);   // Exit active CPU
  case  8: break;                           // Vector  8:  ADC12IFG1
  case 10: break;                           // Vector 10:  ADC12IFG2
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
}

=======
volatile acount;//used for debug, turn target off after hit for 3 seconds.  Then turn target back to active.
>>>>>>> origin/master
//Interrupt vector for timer1 A0
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
	acount++;
	P2OUT = 0x00;	//Turn off the LEDs - the target has been hit
	cont = 1;
	//P2OUT ^= 0x20;
	if(acount > 3){
		active = 1;//turn target back on, for debug testing
		acount = 0;
		TA1CTL = TASSEL_1 + MC_0 + TACLR;//turn off timer
		P2IFG &= 0x3F;//clears any "bounce" interrupts for gun hits
	}
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
    	//P2IE = 0;							//Clear the interrupt
    	//P1IFG
    	Sensors();							//Go into the Sensors function
    	__bic_SR_register_on_exit(LPM3_bits); // Exit active
    	break;                         		// P2.2 IFG
    case  8: break;                         // P2.3 IFG
    case 10: break;                         // P2.4 IFG
    case 12: break;                         // P2.5 IFG
    case 14:
    	active = 0;//set target to off, it has been hit and should not read anymore
    	//P1IFG &= 0xDF;
    	P2IE = 0;		//gun 1 hit

    	TA1CTL = TASSEL_1 + MC_1 + TACLR;//start timer to turn color on for a second
    	color = BLUE;
    	LEDs();
    	__bic_SR_register_on_exit(LPM3_bits); // Exit active
    	break;                         // P2.6 IFG
    case 16:
    	active = 0;//set target to off, it has been hit and should not read anymore;
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
	counter = 100;		//Just a guess
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

	ledcount = 0;
	//TA0CTL |= 0x0010;				//Start the timer
	P2OUT &= 0x00;		//Turn off LEDs
	P2OUT |= color;//0x10;		//Turn on desired leds showing target has been hit
	return;
	while(!cont)
	{

	}
	//Send message to daughter board when it gets hit - wait for a second -> if no acknowledgement then send again
	//P2OUT &= 0x00;
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
      if(receiving)			    // RX end of packet
      {
    	  P2OUT &= 0xF7;		//Turn off the red LEDs
    	  P2OUT |= 0x20;		//Turn on blue LEDs
        // Read the length byte from the FIFO
        RxBufferLength = ReadSingleReg( RXBYTES );
        ReadBurstReg(RF_RXFIFORD, RxBuffer, RxBufferLength);

        // Stop here to see contents of RxBuffer
        __no_operation();

        // Check the CRC results
        /*if(RxBuffer[CRC_LQI_IDX] & CRC_OK)
          P2OUT ^= BIT6;                  */  // Toggle LED1
      }
      else if(transmitting)		    // TX end of packet
      {
    	  P2OUT &= 0xF7;		//Light the diode
        RF1AIE &= ~BIT9;                    // Disable TX end-of-packet interrupt
        //P2OUT &= ~BIT7;                     // Turn off LED after Transmit
        transmitting = 0;
      }
      else while(1); 			    // trap
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

//ADC CONFIGURATION
void adc_config(void)
{
    ADC12CTL0 = ADC12SHT02 + ADC12ON;         // Sampling time, ADC12 on
    ADC12CTL1 = ADC12SHP;                     // Use sampling timer
    ADC12IE = 0x01;                           // Enable interrupt
    ADC12CTL0 |= ADC12ENC;
}

//GPIO CONFIGURATION
void gpio_config(void)
{
    P2OUT &= 0x00;		//Clear the output register for Port 2
    //Set up GPIO pins for LEDs and Sensors (0 for input, 1 for output)
    P2DIR &= 0x00;				//Set Sensors GPIO to an input
   // P2DIR |= 0x3A;				//Set LEDs GPIO to outputs
    P2DIR |= 0x3A;			//FOR TESTING

   // P2SEL &= 0xC1;				//Set all to be GPIO pins - 0 for GPIO
    P2SEL &= 0x01;		//FOR TESTING

    //Set up an interrupt on the sensors when they detect light
   // P2IES &= 0xFB;    	//P2IES -> Select interrupt edge: 0 = L to H, 1 = H to L -> Check this with schematic tomorrow
   // P2IE |= 0x04;		//P2IE  -> Enable/Disable Interrupt: 0 = disabled, 1 = enabled

    P2IE |= 0xC0;		//FOR TESTING
    P2IES &= 0x3F;		//FOR TESTING

    count = 0;
    P2OUT |= 0x08;		//Turn LEDs red


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
<<<<<<< HEAD
    TA1CCR0 = 50000;
    TA1CTL = TASSEL_2 + MC_1 + TACLR;         // SMCLK, upmode, clear TAR
}
/*
 * main.c
 */
void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
=======
    TA1CCR0 = 40000;//50000;
    TA1CTL = TASSEL_1 + MC_0 + TACLR; //Start with timer halted, turn on at target hit//MC_1;// + TACLR;         // SMCLK, upmode, clear TAR
>>>>>>> origin/master

    // Increase PMMCOREV level to 2 in order to avoid low voltage error
    // when the RF core is enabled
    SetVCore(2);
    ResetRadioCore();
    InitRadio();		//Set up the antenna for 915 MHz
    active = 0;			//Target starts as inactive


  //  __bis_SR_register(GIE);       				// Enter LPM0, Enable interrupts
    //  __no_operation();                         	// For debugger


    //TIMER A0 CONFIGURATIONS - For the LEDs on the target to time their color change
    //TA0CTL &= 0x00;			//Clear the control register
   // TA0CTL |= 0x0102;		//Set Timer to use ACLK, in stop mode, has interrupts
    //TA0CCTL0 |= CCIE;		//Set this interrupt as the highest priority
    //TA0CCR0 = TRIGGER;		//Compare/Capture for Timer will interrupt when this value is reached - should be one second

    gpio_config();				//Configure the GPIOs
    adc_config();				//Configure the ADC
    uart_config();				//Configure the UART
    timer_config();				//Configure the timer

    //Assign unique target ID to each target
    while(1)
    {
    	if(active){
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

