#include <msp430.h>
#include "RF_Toggle_LED_Demo.h"
#include <stdio.h>
#include <stdlib.h>

// Demonstrating a change for Git.

#define  PACKET_LEN         (0x05)			// PACKET_LEN <= 61
#define  RSSI_IDX           (PACKET_LEN)    // Index of appended RSSI
#define  CRC_LQI_IDX        (PACKET_LEN+1)  // Index of appended LQI, checksum
#define  CRC_OK             (BIT7)          // CRC_OK bit
#define  PATABLE_VAL        (0x51)          // 0 dBm output

extern RF_SETTINGS rfSettings;

volatile unsigned int info[6];
//volatile unsigned int transmit_flag;

unsigned char TxBuffer[PACKET_LEN]= {0xAA, 0xBB, 0xCC, 0xDD, 0x00};
unsigned char RxBuffer[PACKET_LEN+2];
unsigned char RxBufferLength = 0;

unsigned char transmitting = 0;
unsigned char receiving = 0;
unsigned char targets;
unsigned char rounds;
unsigned char gunIDs[2];
unsigned char targetIDs[3];


void uart_putc(unsigned char c)
{
	while (!(UCA0IFG & UCTXIFG));             // USCI_A0 TX buffer ready?
	    UCA0TXBUF = c;                  // TX -> RXed character
}
void uart_puts(const char *str)
{
     while(*str) uart_putc(*str++);
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


// Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	unsigned char tx_char;
  switch(__even_in_range(UCA0IV,4))
  {
  case 0:break;                             // Vector 0 - no interrupt
  case 2:                                   // Vector 2 - RXIFG

    while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
    UCA0TXBUF = UCA0RXBUF;                  // TX -> RXed character
    while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
    UCA0TXBUF = 'T';                  // TX -> RXed character
    break;
  case 4:
      __delay_cycles(5000);                 // Add small gap between TX'ed bytes
      tx_char = P1IN;
      tx_char = tx_char >> 4;
      UCA0TXBUF = tx_char;                  // Transmit character
	  break;                             // Vector 4 - TXIFG
  default: break;
  }
}


//Interrupt Vector for the antenna for both receiving and transmitting
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
        // Read the length byte from the FIFO
        RxBufferLength = ReadSingleReg( RXBYTES );
        ReadBurstReg(RF_RXFIFORD, RxBuffer, RxBufferLength);

        // Stop here to see contents of RxBuffer
        __no_operation();

        // Check the CRC results
        if(RxBuffer[CRC_LQI_IDX] & CRC_OK)
          P2OUT ^= BIT6;                    // Toggle LED1
      }
      else if(transmitting)		    // TX end of packet
      {
        RF1AIE &= ~BIT9;                    // Disable TX end-of-packet interrupt
        P2OUT &= ~BIT7;                     // Turn off LED after Transmit
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

    //Peripheral is going to send and receive information from other modules so need to set it up to receive information
    // and send it along the PM_UCA0SOMI pin to the Zedboard or the Cypress

    //P5SEL |= 0x03;                            // Enable XT1 pins
    // Increase PMMCOREV level to 2 in order to avoid low voltage error
    // when the RF core is enabled
      SetVCore(2);
      ResetRadioCore();
      InitRadio();		//Sets up the antenna radio to be active

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
      UCA0IE |= UCRXIE;                // Enable USCI_A0 RX interrupt

      uart_puts((char *)"\n\r**************\n\rPeripheral\n\r");
      uart_puts((char *)"***************\n\r\n\r");

      //Strobe sends commands to our antenna
      Strobe( RF_SIDLE );				//Exit receive mode on the antenna
      Strobe(RF_SRX);					//Enable receive mode on the antenna

      __bis_SR_register(GIE);
      __no_operation();                         // For debugger


      while(1)
      {
    	  //Check field for # of guns & targets - get each id and store into seperate array
    	  //Send a packet of game information to peripheral - number of targets used, rounds
    	  	  //Comes from zed which will be on port PM_UCA0SOMI
    	  //Initialize start game state for all of them
    	  //Send a start game message to guns
    	  while(rounds != 0)
    	  {
    	  	 //Set number of targets
    	  	 receiving = 1;
    	  	 	 while(targets != 0)//Set receiving to 1 so we can receive messages from targets and guns
    	  	 	 {
    	  	 		 //Each target & gun have their own unique id which will be sent in the packet
    	  	 		 //Target will send the guns id and a timestamp as well - this info will be sent along to the zedboard to process
    	  	 		 	 //Data will be sent to Zed through PM_UCA0SIMO
    	  	 		 //Subtract 1 from number of targets after one data packet received
    	  	 		 //When the number of targets in the game have sent information, leave the while loop because round is over

    	  	 	 }
    	  	 receiving = 0;			//Need to transmit message to guns
    	  	 	 //Tell gun to send its data to peripheral
    	  	 receiving = 1;			//Ready to get data from gun
    	  	 //Receive packets of data from each gun with number of shots - Match with ID
    	  	 //Leave while loop when game is completed
    	  }
    	  //Tell guns & targets game is over
      }

   }

