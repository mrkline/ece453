///////////////////////////////////////////////////////////////////////////////
/*GUN MODULE
 * This module is the software loaded onto the guns. It will take care of the
 * GPIO functions for the laser, buzzer and button. The button will operate
 * through an interrupt and modify the buzzer & laser accordingly. The gun
 * module also needs to transmit & receive messages with the daughter board.
 *
 */
#include <msp430.h> 
#include "RF_Toggle_LED_Demo.h"

#define  PACKET_LEN         (0x05)			// PACKET_LEN <= 61
#define  RSSI_IDX           (PACKET_LEN)    // Index of appended RSSI
#define  CRC_LQI_IDX        (PACKET_LEN+1)  // Index of appended LQI, checksum
#define  CRC_OK             (BIT7)          // CRC_OK bit
#define  PATABLE_VAL        (0x51)          // 0 dBm output

extern RF_SETTINGS rfSettings;

volatile unsigned char info;
//volatile unsigned int transmit_flag;

unsigned int buzz;
//unsigned char TxBuffer[PACKET_LEN]= {0xAA, 0xBB, 0xCC, 0xDD, 0x00};
unsigned char RxBuffer[PACKET_LEN+2];
unsigned char RxBufferLength = 0;

unsigned char transmitting = 0;
unsigned char receiving = 0;
unsigned char gunID;
unsigned char numShots = 0;
unsigned char rounds;

const unsigned char TxBuffer[PACKET_LEN]= {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};

void uart_putc(unsigned char c);
void uart_puts(const char *str);

void uart_putc(unsigned char c)
{
	while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
	    UCA0TXBUF = c;                  // TX -> RXed character
}
void uart_puts(const char *str)
{
     while(*str) uart_putc(*str++);
}

//If the trigger is pulled, we will go to this method to take care of the
// buzzer and laser functionality
void TriggerPull(void)
{
	buzz = 50;
    P2OUT |= 0x01; //When the button is pushed down(trigger pull) the laser should go on (output bit goes high)

    //When the button is pushed, the buzzer should make a noise for certain amount of time
    while(buzz != 0)
    {
    	P2OUT |= 0x04; //Buzzer goes on (output bit goes high)
    	buzz = buzz - 1;
    }
}


//Function to have high power mode enabled for the radio, write the correct settings
// to the radio and set up the PA table values
void InitRadio()
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

//Interrupt for when the trigger is pulled
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
  switch(__even_in_range(P2IV, 16))
  {
    case  0: break;
    case  2: break;                         // P2.0 IFG

    case  4:                          		// P2.1 IFG
    P2IFG &= 0xFD;
    //buttonPressed = 1;
    TriggerPull();
    P2OUT &= 0xFA;		//Turn the buzzer and the laser off
    numShots = numShots + 1;			//NOT SURE ABOUT THIS LOGIC
    __bic_SR_register_on_exit(LPM3_bits); // Exit active
    break;

    case  6: break;                         // P2.2 IFG
    case  8: break;                         // P2.3 IFG
    case 10: break;                         // P2.4 IFG
    case 12: break;                         // P2.5 IFG
    case 14: break;                         // P2.6 IFG
    case 16: break;                         // P2.7 IFG
    default: break;
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
    	  P2OUT &= 0xF7;		//Light the diode
        // Read the length byte from the FIFO
        RxBufferLength = ReadSingleReg( RXBYTES );
        ReadBurstReg(RF_RXFIFORD, RxBuffer, RxBufferLength);

        // Stop here to see contents of RxBuffer
        __no_operation();

        // Check the CRC results
        /*if(RxBuffer[CRC_LQI_IDX] & CRC_OK)
          P2OUT ^= BIT6;                    // Toggle LED1*/
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

/*
 * main.c
 */
void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    // Increase PMMCOREV level to 2 in order to avoid low voltage error
    // when the RF core is enabled
    SetVCore(2);

    ResetRadioCore();
    InitRadio();

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

    __bis_SR_register(GIE);       // Enter LPM0, Enable interrupts
      __no_operation();                         // For debugger
    //Gun needs to transmit its data to the peripheral and receive info back from peripheral
    //Configure all GPIO pins and antenna pins

    //Make pins 2.2(buzzer) & 2.0(laser) outputs and pin 2.1(button) an input
    P2DIR &= 0xFD;
    P2DIR |= 0x0D;

    //Pins 2.0, 2.1, and 2.2 selected to be GPIOs
    P2SEL &= 0xF0;

    //Set up an interrupt on the button for the trigger pull
    P2IES &= 0xFD;    	//P2IES -> Select interrupt edge: 0 = L to H, 1 = H to L -> Check this with schematic tomorrow
    P2IE |= 0x02;		//P2IE  -> Enable/Disable Interrupt: 0 = disabled, 1 = enabled

   	//P2OUT &= 0xF7;		//Light the diode
   	P2OUT |= 0x08;		//Darken the diode
   	ReceiveOff();
   	receiving = 0;

   	transmitting = 1;
   	Transmit( (unsigned char*)TxBuffer, sizeof TxBuffer);  			//In order to transmit


   	P2OUT |= 0x08;		//Darken the diode
   //while(1);
  /*  while(1)
    {
    	numShots = 0;			//Reset value of number of shots
    	while(rounds != 0)
    	{
    		//Gun will be receiving a message to tell it to send its data
    		//Gets message from peripheral board

    		info = gunID;	//Send peripheral confirmation that this gun is in the game - send its unique id
    		TxBuffer[PACKET_LEN - 2] = info;
    		Transmit( (unsigned char*)TxBuffer, sizeof TxBuffer);
    		transmitting = 1;	//Ready gun board to send info to peripheral

    		//Gun then will wait for next message from peripheral - End of Game
    		info = numShots;//When message received, transmit packet with number of shots fired with unique gun ID
    		info = gunID;		//LOOK AT THIS TONIGHT
    		TxBuffer[PACKET_LEN - 2] = info;
    		Transmit((unsigned char*)TxBuffer, sizeof TxBuffer);
    		transmitting = 1;
    	}
    }*/
}
