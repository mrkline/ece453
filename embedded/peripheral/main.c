/*
 * Daughter Card Embedded Software
 * Written by Tyler Kuske & James Gordon
 * This is the software for the card that plugs into the Zedboard. It is meant
 * to get messages from the GUI through the Zed. It then broadcasts the messages
 * out to the targets and the guns in packets. It is also able to receive
 * messages back to relay to the Zedboard.
 */
#include <msp430.h>
#include "RF_Toggle_LED_Demo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "validate.h"

#define  PACKET_LEN         (0x05)			// PACKET_LEN <= 61
#define  RSSI_IDX           (PACKET_LEN)    // Index of appended RSSI
#define  CRC_LQI_IDX        (PACKET_LEN+1)  // Index of appended LQI, checksum
#define  CRC_OK             (BIT7)          // CRC_OK bit
#define  PATABLE_VAL        (0x51)          // 0 dBm output
#define  ID						1			//target id

/////////message types//////////
#define RESPONSE 1
#define QUERY	2

typedef enum {   //type int8_t
	 OK,
     INTERNAL_ERROR,
     UNKNOWN_REQUEST,
     INVALID_REQUEST,
     UNSUPPORTED_REQUEST
};

extern RF_SETTINGS rfSettings;									//Radio settings set in external file

volatile unsigned int info[6];

const unsigned char TxBuffer[PACKET_LEN]
                         = {0xAA, 0xBB, 0xCC, 0xDD, 0x00};		//Transmit buffer for radio
unsigned char RxBuffer[PACKET_LEN+2];							//Receive buffer for radio
unsigned char RxBufferLength 	= 0;							//Length of info in the receive buffer

unsigned char transmitting 		= 0;							//Asserted if radio is transmitting
unsigned char receiving 		= 0;							//Asserted if radio is receiving
unsigned char targets;											//Number of targets in current game
unsigned char rounds;											//Number of rounds in current games
unsigned char gunIDs[2];										//Unique IDs for each gun
unsigned char targetIDs[3];										//Unique IDs for each target

//Transmit character function for UART
void uart_putc(unsigned char c)
{
	while (!(UCA0IFG & UCTXIFG));             // USCI_A0 TX buffer ready?
	    UCA0TXBUF = c;                  // TX -> RXed character
}

//Transmit string function for UART
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

  //Set radio settings
  WriteRfSettings(&rfSettings);

  WriteSinglePATable(PATABLE_VAL);
}

//Transmit function for the wireless radio
void Transmit(unsigned char *buffer, unsigned char length)
{
	//Transmit on the antenna here when ready to send data
	RF1AIES |= BIT9;
	RF1AIFG &= ~BIT9;                         // Clear pending interrupts
	RF1AIE |= BIT9;                           // Enable TX end-of-packet interrupt
	P2OUT |= 0x02;							//Turn on the diode

	WriteBurstReg(RF_TXFIFOWR, buffer, length);

	Strobe( RF_STX );                         // Strobe STX
}

//Set the wireless radio up to receive messages
void ReceiveOn(void)
{
	//Receive signals from the peripheral
	RF1AIES |= BIT9;                          // Falling edge of RFIFG9
	RF1AIFG &= ~BIT9;                         // Clear a pending interrupt
	RF1AIE  |= BIT9;                          // Enable the interrupt

	// Radio is in IDLE following a TX, so strobe SRX to enter Receive Mode
	Strobe( RF_SRX );
}

//Turn off the wireless radio to receive messages
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
    	 P2OUT &= 0xF7;		//Turn off the red LEDs
    	 P2OUT |= 0x20;		//Turn on blue LEDs
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
    UCA0BR1 = 0x00;
    UCA0MCTL = UCBRS_3+UCBRF_0;               // Modulation UCBRSx=3, UCBRFx=0
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
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
      InitRadio();								//Sets up the antenna radio to be active

      PMAPPWD = 0x02D52;                        // Get write-access to port mapping regs
      P1MAP5 = PM_UCA0RXD;                      // Map UCA0RXD output to P1.5
      P1MAP6 = PM_UCA0TXD;                      // Map UCA0TXD output to P1.6
      PMAPPWD = 0;                              // Lock port mapping registers

      P1DIR |= BIT6;                            // Set P1.6 as TX output
      P1SEL |= BIT5 + BIT6;                     // Select P1.5 & P1.6 to UART function

      P2DIR |= 0x02;							//Configure the buffer
      P2SEL &= 0xFD;

      UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
      UCA0CTL1 |= UCSSEL_1;                     // CLK = ACLK
      UCA0BR0 = 0x03;                           // 32kHz/9600=3.41 (see User's Guide)
      UCA0BR1 = 0x00;                           //
      UCA0MCTL = UCBRS_3+UCBRF_0;               // Modulation UCBRSx=3, UCBRFx=0
      UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
      UCA0IE |= UCRXIE;                			// Enable USCI_A0 RX interrupt

      uart_config();							//Configure the UART

      //Code for testing the UART communication
      unsigned char string6[] = {0x66, 0x75, 0x0c, 0x00, 0x00, 0x00, 0x04, 0x01, 0x01, 0x03,0x00, 0x79, 0x79};
      int i;
      while(1)
      {
    	  for(i = 0; i <=12; i++)
    	  {
    		  uart_putc(string6[i]);
    	  }
      }
  	/*uint8_t recChar = UCA0RXBUF; //grab next byte in uart.
  	while(!onChar(recChar)){
  		recChar = UCA0RXBUF;
  	}
  	i = 0;
  	      while(!onChar(buffer[i]))
  	    	  i++;
  	      uart_puts("\n\rSUCCESS2\n\r");
  	      char IDstring2 [20];
  	  	  sprintf(IDstring2, "id: %d\n\r", currentID);
  		  uart_puts(IDstring2);
  	  	  if(type==1)
  	  		  uart_puts("typeNEW: RESPONSE\n\r");
  	      char PLstring2 [50];
  	  	  sprintf(PLstring2, "payloadLength: %d\n\r", payloadLength);
  		  uart_puts(PLstring2);
  	      char boardIDstring2 [20];
  	      uint16_t respTo = extractUInt16(buffer + 7);
  	      char respString [20];
  	  	  sprintf(respString, "responding to: %d\n\r", respTo);
  		  uart_puts(respString);
  	      if(buffer[9] == OK)
  	    	  uart_puts("CODE: OK\n\r");
  	      if(buffer[10] == 'y' && buffer[11] == 'y'){
  	    	  uart_puts("END\n\r");
  	      }*/
  	      //*NOTE: Response message does not need a board id or board type, it is implied to be daughter card's, don't implement in target and gun.
  		  //RESPONSE:  //type == 1
  		  //if(message_id == waiting_id)//make sure id of response message is one that has been issued. have an array indexed by sent message id count.  Decrement when entryID has beend recieved
  		  //use:payload length, payload->respTo, code

/*
      uart_puts((char *)"\n\r**************\n\rPeripheral\n\r");
      uart_puts((char *)"***************\n\r\n\r");
      	  	  	  	  	  	  //f	   u    querey   v id v   vpaylengthv vpayloadv  end
      unsigned char string[] = {0x66, 0x75, 0x02, 0x00, 0x00, 0x00, 0x02, 0x01, 0x01, 0x79, 0x79};


      int i = 0;
      while(!onChar(string[i]))
    	  i++;
      uart_puts("\n\rSUCCESS1\n\r");
      char IDstring [20];
  	  sprintf(IDstring, "id: %d\n\r", currentID);
	  uart_puts(IDstring);
  	  if(type==2)
  		  uart_puts("typeNEW: query\n\r");
      char PLstring [50];
  	  sprintf(PLstring, "payloadLength: %d\n\r", payloadLength);
	  uart_puts(PLstring);
      char boardIDstring [20];
  	  sprintf(boardIDstring, "board id: %d\n\r", string[7]);
	  uart_puts(boardIDstring);
	  if(string[8] == 1)
		  uart_puts("boardType: target\n\r");
	  if(string[9] == 'y' && string[10] == 'y')
		  uart_puts("END\n\r");


	  //Main: (used as reader)
	  //while(1){grab character and vvvifvvv
	  //if(valid){case(type) :}
	  //QUEREY:  //type == 2
	  //if(payload length == whatever && boardid == this id && boardtype == this type)
	  //Respond(id, paylength, payload<-payload will be an array of size paylength)

	 uart_puts((char *)"\n\r**************\n\rPeripheral\n\r");
	 uart_puts((char *)"***************\n\r\n\r");
	      	  	  	  	  	   //f	   u   RESPONSE   v id v   vpaylengthv vpayloadv  end
	 unsigned char string2[] = {0X66, 0X75, 0X01, 0X00, 0X00, 0X00, 0X03, 0X00, 0X19, 0X00, 0X79, 0x79};

      i = 0;
      while(!onChar(string2[i]))
    	  i++;
      uart_puts("\n\rSUCCESS2\n\r");
      char IDstring2 [20];
  	  sprintf(IDstring2, "id: %d\n\r", currentID);
	  uart_puts(IDstring2);
  	  if(type==1)
  		  uart_puts("typeNEW: RESPONSE\n\r");
      char PLstring2 [50];
  	  sprintf(PLstring2, "payloadLength: %d\n\r", payloadLength);
	  uart_puts(PLstring2);
      char boardIDstring2 [20];
      uint16_t respTo = extractUInt16(string2 + 7);
      char respString [20];
  	  sprintf(respString, "responding to: %d\n\r", respTo);
	  uart_puts(respString);
      if(string2[9] == OK)
    	  uart_puts("CODE: OK\n\r");
      if(string2[10] == 'y' && string2[11] == 'y'){
    	  uart_puts("END\n\r");
      }
      //*NOTE: Response message does not need a board id or board type, it is implied to be daughter card's, don't implement in target and gun.
	  //RESPONSE:  //type == 1
	  //if(message_id == waiting_id)//make sure id of response message is one that has been issued. have an array indexed by sent message id count.  Decrement when entryID has beend recieved
	  //use:payload length, payload->respTo, code



     // 66 75 0a 00 00 00 06 02 04 00 00 00 f0 79 79

      uart_puts((char *)"\n\r**************\n\rPeripheral\n\r");
      uart_puts((char *)"***************\n\r\n\r");
      	  	  	  	  	  	     //f     u    shot   v id v   vpaylengthv vpayloadv                             end
      unsigned char string3[] = {0x66, 0x75, 0x0a, 0x00, 0x00, 0x00, 0x06, 0x02, 0x04, 0x00, 0x00, 0x00, 0xf0, 0x79, 0x79};
      //uart_puts(string3);


       i = 0;
      while(!onChar(string3[i]))
    	  i++;
      uart_puts("\n\rSUCCESS3\n\r");
      char IDstring3 [20];
  	  sprintf(IDstring3, "id: %d\n\r", currentID);
	  uart_puts(IDstring3);
  	  if(type==10)
  		  uart_puts("typeNEW: SHOT\n\r");
      char PLstring3 [50];
  	  sprintf(PLstring3, "payloadLength: %d\n\r", payloadLength);
	  uart_puts(PLstring3);
	  char playerIDstring [20];
	  sprintf(playerIDstring, "player id: %d\n\r", string3[7]);
	  uart_puts(playerIDstring);
      char targetIDstring3 [20];
  	  sprintf(targetIDstring3, "target id: %d\n\r", string3[8]);//int8
	  uart_puts(targetIDstring3);
	  uint32_t stamp = extractUInt32(string3 + 9);
	  char stampString [20];
	  sprintf(stampString, "time stamp: %d \n\r", stamp);
	  uart_puts(stampString);
      if(string3[13] == 'y' && string3[14] == 'y'){
    	  uart_puts("END\n\r");
      }



	  //Main: (used as reader)
	  //while(1){grab character and vvvifvvv
	  //if(valid){case(type) :}
	  //SHOT:  //type == 10  //SHOT Message is what target sends on hit.
	  //if(payload length == whatever && targetid == activeTargetid && boardtype == this type)
	  //Output playerid and timestamp?



       uart_puts((char *)"\n\r**************\n\rPeripheral\n\r");
       uart_puts((char *)"***************\n\r\n\r");
       	  	  	  	  	  	     //f     u    start   v id v   vpaylengthv   end
       unsigned char string4[] = {0x66, 0x75, 0x04, 0x00, 0x00, 0x00, 0x00, 0x79, 0x79};
       //uart_puts(string3);


       i = 0;
       while(!onChar(string4[i]))
     	  i++;
       uart_puts("\n\rSUCCESS4\n\r");
       char IDstring4 [20];
   	  sprintf(IDstring4, "id: %d\n\r", currentID);
 	  uart_puts(IDstring4);
   	  if(type==4)
   		  uart_puts("typeNEW: START\n\r");
       char PLstring4 [50];
   	  sprintf(PLstring4, "payloadLength: %d\n\r", payloadLength);
 	  uart_puts(PLstring4);
       if(string4[7] == 'y' && string4[8] == 'y'){
     	  uart_puts("END\n\r");
       }
 	  //Main: (used as reader)
 	  //while(1){grab character and vvvifvvv
 	  //if(valid){case(type) :}
 	  //START:  //type == 4  //START Starts a game.
 	  //if(payload length == 0

       // 66 75 04 00 00 00 00 79

       uart_puts((char *)"\n\r**************\n\rPeripheral\n\r");
       uart_puts((char *)"***************\n\r\n\r");
       	  	  	  	  	  	  //f	   u    stop   v id v     vpaylengthv  end
       unsigned char string5[] = {0x66, 0x75, 0x05, 0x00, 0x00, 0x00, 0x00, 0x79, 0x79};


       i = 0;
       while(!onChar(string5[i]))
     	  i++;
       uart_puts("\n\rSUCCESS5\n\r");
       char IDstring5 [20];
   	  sprintf(IDstring5, "id: %d\n\r", currentID);
 	  uart_puts(IDstring5);
   	  if(type==5)
   		  uart_puts("typeNEW: STOP\n\r");
       char PLstring5 [50];
   	  sprintf(PLstring5, "payloadLength: %d\n\r", payloadLength);
 	  uart_puts(PLstring5);
      if(string5[7] == 'y' && string5[8] == 'y'){
    	  uart_puts("END\n\r");
      }
 	  //Main: (used as reader)
 	  //while(1){grab character and vvvifvvv
 	  //if(valid){case(type) :}
 	  //QUEREY:  //type == 2
 	  //if(payload length == whatever && boardid == this id && boardtype == this type)
 	  //Respond(id, paylength, payload<-payload will be an array of size paylength)

 	 //66 75 0c 00 00 00 04 01 01 03 00 79 79

     uart_puts((char *)"\n\r**************\n\rPeripheral\n\r");
     uart_puts((char *)"***************\n\r\n\r");
     	  	  	  	  	  	  //f	   u    targctl  v id v   vpaylengthv
     unsigned char string6[] = {0x66, 0x75, 0x0c, 0x00, 0x00, 0x00, 0x04, 0x01, 0x01, 0x03,0x00, 0x79, 0x79};
     //if(payloadlength ==

     i = 0;
     while(!onChar(string6[i]))
   	  i++;
     uart_puts("\n\rSUCCESS6\n\r");
     char IDstring6 [20];
 	  sprintf(IDstring6, "id: %d\n\r", currentID);
	  uart_puts(IDstring6);
 	  if(type== 12)
 		  uart_puts("typeNEW: TARGET CONTROL\n\r");
     char PLstring6 [50];
 	  sprintf(PLstring6, "payloadLength: %d\n\r", payloadLength);
	  uart_puts(PLstring6);
	  char targetIdString [20];
	  if(payloadLength >= 2){//&& string[7] == thisID
		  sprintf(targetIdString, "targetId: %d\n\r", string6[7]);
		  uart_puts(targetIdString);
		  if(string6[8])
			  uart_puts("target ON\n\r");
		  else
			  uart_puts("target OFF\n\r");
	  }
	  if(payloadLength >= 4){//&& string[9] == thisID
		  sprintf(targetIdString, "targetId: %d\n\r", string6[9]);
		  uart_puts(targetIdString);
		  if(string6[10])
			  uart_puts("target ON\n\r");
		  else
			  uart_puts("target OFF\n\r");
	  }
	  if(payloadLength == 6){//&& string[11] == thisID
		  sprintf(targetIdString, "targetId: %d\n\r", string6[11]);
		  uart_puts(targetIdString);
		  if(string6[12])
			  uart_puts("target ON\n\r");
		  else
			  uart_puts("target OFF\n\r");
	  }
	  if(string6[11] == 'y' && string6[12] == 'y')
		  uart_puts("END\n\r");
	  //Main: (used as reader)
	  //while(1){grab character and vvvifvvv
	  //if(valid){case(type) :}
	  //TARGET CONTROL:  //type == 12
*/
      //Set up GPIO pins for LEDs and Sensors
          P2DIR &= 0xFB;				//Set Sensors GPIO to an input
          P2DIR |= 0x3A;				//Set LEDs GPIO to outputs

          P2SEL &= 0xC1;				//Set all to be GPIO pins

      //Strobe sends commands to our antenna
//      Strobe( RF_SIDLE );				//Exit receive mode on the antenna
     // Strobe(RF_SRX);					//Enable receive mode on the antenna

      __bis_SR_register(GIE);
      __no_operation();                         // For debugger

      transmitting = 1;
      Transmit( (unsigned char*)TxBuffer, sizeof TxBuffer); 			//In order to transmit

     while(1)
     {
    	 uint8_t byte = UCA0RXBUF;//potentially have to poll a different way, check for flag?
    	 if(onChar(byte))
    	 {
    		 switch (type)
    		 {
    		 	 case QUERY:
    		 		 if(buffer[0] == 'f' && buffer[1] == 'u')
    		 		 {
    		 			 //handle Query, look at TX in uart ISR
    		 			 uart_puts("QUERY");							//Send message across UART
    		 		 }
    		 		 break;
    		 }
    	 }
     }
  /* CODE BELOW WAS FOR THE GAME FLOW CONTROL WITH SENDING MESSAGES BETWEEN THE DAUGHTER BOARD & OTHER MODULES
   * while(1)
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
      }*/

   }

/*		ALL CODE BELOW WAS FOR THE PERIPHERAL TO SET UP PACKETS & SEND THEM ACROSS WIRELESS IF OUR ANTENNA/UART WOULD HAVE WORKED
      uart_puts((char *)"\n\r**************\n\rPeripheral\n\r");
      uart_puts((char *)"***************\n\r\n\r");
      	  	  	  	  	  	  //f	   u    querey   v id v   vpaylengthv vpayloadv  end
      unsigned char string[] = {0x66, 0x75, 0x02, 0x00, 0x00, 0x00, 0x02, 0x01, 0x01, 0x79, 0x79};


      int i = 0;
      while(!onChar(string[i]))
    	  i++;
      uart_puts("\n\rSUCCESS1\n\r");
      char IDstring [20];
  	  sprintf(IDstring, "id: %d\n\r", currentID);
	  uart_puts(IDstring);
  	  if(type==2)
  		  uart_puts("typeNEW: query\n\r");
      char PLstring [50];
  	  sprintf(PLstring, "payloadLength: %d\n\r", payloadLength);
	  uart_puts(PLstring);
      char boardIDstring [20];
  	  sprintf(boardIDstring, "board id: %d\n\r", string[7]);
	  uart_puts(boardIDstring);
	  if(string[8] == 1)
		  uart_puts("boardType: target\n\r");
	  if(string[9] == 'y' && string[10] == 'y')
		  uart_puts("END\n\r");


	  //Main: (used as reader)
	  //while(1){grab character and vvvifvvv
	  //if(valid){case(type) :}
	  //QUEREY:  //type == 2
	  //if(payload length == whatever && boardid == this id && boardtype == this type)
	  //Respond(id, paylength, payload<-payload will be an array of size paylength)

	 uart_puts((char *)"\n\r**************\n\rPeripheral\n\r");
	 uart_puts((char *)"***************\n\r\n\r");
	      	  	  	  	  	   //f	   u   RESPONSE   v id v   vpaylengthv vpayloadv  end
	 unsigned char string2[] = {0X66, 0X75, 0X01, 0X00, 0X00, 0X00, 0X03, 0X00, 0X19, 0X00, 0X79, 0x79};

      i = 0;
      while(!onChar(string2[i]))
    	  i++;
      uart_puts("\n\rSUCCESS2\n\r");
      char IDstring2 [20];
  	  sprintf(IDstring2, "id: %d\n\r", currentID);
	  uart_puts(IDstring2);
  	  if(type==1)
  		  uart_puts("typeNEW: RESPONSE\n\r");
      char PLstring2 [50];
  	  sprintf(PLstring2, "payloadLength: %d\n\r", payloadLength);
	  uart_puts(PLstring2);
      char boardIDstring2 [20];
      uint16_t respTo = extractUInt16(string2 + 7);
      char respString [20];
  	  sprintf(respString, "responding to: %d\n\r", respTo);
	  uart_puts(respString);
      if(string2[9] == OK)
    	  uart_puts("CODE: OK\n\r");
      if(string2[10] == 'y' && string2[11] == 'y'){
    	  uart_puts("END\n\r");
      }
      //*NOTE: Response message does not need a board id or board type, it is implied to be daughter card's, don't implement in target and gun.
	  //RESPONSE:  //type == 1
	  //if(message_id == waiting_id)//make sure id of response message is one that has been issued. have an array indexed by sent message id count.  Decrement when entryID has beend recieved
	  //use:payload length, payload->respTo, code



     // 66 75 0a 00 00 00 06 02 04 00 00 00 f0 79 79

      uart_puts((char *)"\n\r**************\n\rPeripheral\n\r");
      uart_puts((char *)"***************\n\r\n\r");
      	  	  	  	  	  	     //f     u    shot   v id v   vpaylengthv vpayloadv                             end
      unsigned char string3[] = {0x66, 0x75, 0x0a, 0x00, 0x00, 0x00, 0x06, 0x02, 0x04, 0x00, 0x00, 0x00, 0xf0, 0x79, 0x79};
      //uart_puts(string3);


       i = 0;
      while(!onChar(string3[i]))
    	  i++;
      uart_puts("\n\rSUCCESS3\n\r");
      char IDstring3 [20];
  	  sprintf(IDstring3, "id: %d\n\r", currentID);
	  uart_puts(IDstring3);
  	  if(type==10)
  		  uart_puts("typeNEW: SHOT\n\r");
      char PLstring3 [50];
  	  sprintf(PLstring3, "payloadLength: %d\n\r", payloadLength);
	  uart_puts(PLstring3);
	  char playerIDstring [20];
	  sprintf(playerIDstring, "player id: %d\n\r", string3[7]);
	  uart_puts(playerIDstring);
      char targetIDstring3 [20];
  	  sprintf(targetIDstring3, "target id: %d\n\r", string3[8]);//int8
	  uart_puts(targetIDstring3);
	  uint32_t stamp = extractUInt32(string3 + 9);
	  char stampString [20];
	  sprintf(stampString, "time stamp: %d \n\r", stamp);
	  uart_puts(stampString);
      if(string3[13] == 'y' && string3[14] == 'y'){
    	  uart_puts("END\n\r");
      }



	  //Main: (used as reader)
	  //while(1){grab character and vvvifvvv
	  //if(valid){case(type) :}
	  //SHOT:  //type == 10  //SHOT Message is what target sends on hit.
	  //if(payload length == whatever && targetid == activeTargetid && boardtype == this type)
	  //Output playerid and timestamp?



       uart_puts((char *)"\n\r**************\n\rPeripheral\n\r");
       uart_puts((char *)"***************\n\r\n\r");
       	  	  	  	  	  	     //f     u    start   v id v   vpaylengthv   end
       unsigned char string4[] = {0x66, 0x75, 0x04, 0x00, 0x00, 0x00, 0x00, 0x79, 0x79};
       //uart_puts(string3);


       i = 0;
       while(!onChar(string4[i]))
     	  i++;
       uart_puts("\n\rSUCCESS4\n\r");
       char IDstring4 [20];
   	  sprintf(IDstring4, "id: %d\n\r", currentID);
 	  uart_puts(IDstring4);
   	  if(type==4)
   		  uart_puts("typeNEW: START\n\r");
       char PLstring4 [50];
   	  sprintf(PLstring4, "payloadLength: %d\n\r", payloadLength);
 	  uart_puts(PLstring4);
       if(string4[7] == 'y' && string4[8] == 'y'){
     	  uart_puts("END\n\r");
       }
 	  //Main: (used as reader)
 	  //while(1){grab character and vvvifvvv
 	  //if(valid){case(type) :}
 	  //START:  //type == 4  //START Starts a game.
 	  //if(payload length == 0

       // 66 75 04 00 00 00 00 79

       uart_puts((char *)"\n\r**************\n\rPeripheral\n\r");
       uart_puts((char *)"***************\n\r\n\r");
       	  	  	  	  	  	  //f	   u    stop   v id v     vpaylengthv  end
       unsigned char string5[] = {0x66, 0x75, 0x05, 0x00, 0x00, 0x00, 0x00, 0x79, 0x79};


       i = 0;
       while(!onChar(string5[i]))
     	  i++;
       uart_puts("\n\rSUCCESS5\n\r");
       char IDstring5 [20];
   	  sprintf(IDstring5, "id: %d\n\r", currentID);
 	  uart_puts(IDstring5);
   	  if(type==5)
   		  uart_puts("typeNEW: STOP\n\r");
       char PLstring5 [50];
   	  sprintf(PLstring5, "payloadLength: %d\n\r", payloadLength);
 	  uart_puts(PLstring5);
      if(string5[7] == 'y' && string5[8] == 'y'){
    	  uart_puts("END\n\r");
      }
 	  //Main: (used as reader)
 	  //while(1){grab character and vvvifvvv
 	  //if(valid){case(type) :}
 	  //QUEREY:  //type == 2
 	  //if(payload length == whatever && boardid == this id && boardtype == this type)
 	  //Respond(id, paylength, payload<-payload will be an array of size paylength)

 	 //66 75 0c 00 00 00 04 01 01 03 00 79 79

     uart_puts((char *)"\n\r**************\n\rPeripheral\n\r");
     uart_puts((char *)"***************\n\r\n\r");
     	  	  	  	  	  	  //f	   u    targctl  v id v   vpaylengthv
     unsigned char string6[] = {0x66, 0x75, 0x0c, 0x00, 0x00, 0x00, 0x04, 0x01, 0x01, 0x03,0x00, 0x79, 0x79};
     //if(payloadlength ==

     i = 0;
     while(!onChar(string6[i]))
   	  i++;
     uart_puts("\n\rSUCCESS6\n\r");
     char IDstring6 [20];
 	  sprintf(IDstring6, "id: %d\n\r", currentID);
	  uart_puts(IDstring6);
 	  if(type== 12)
 		  uart_puts("typeNEW: TARGET CONTROL\n\r");
     char PLstring6 [50];
 	  sprintf(PLstring6, "payloadLength: %d\n\r", payloadLength);
	  uart_puts(PLstring6);
	  char targetIdString [20];
	  if(payloadLength >= 2){//&& string[7] == thisID
		  sprintf(targetIdString, "targetId: %d\n\r", string6[7]);
		  uart_puts(targetIdString);
		  if(string6[8])
			  uart_puts("target ON\n\r");
		  else
			  uart_puts("target OFF\n\r");
	  }
	  if(payloadLength >= 4){//&& string[9] == thisID
		  sprintf(targetIdString, "targetId: %d\n\r", string6[9]);
		  uart_puts(targetIdString);
		  if(string6[10])
			  uart_puts("target ON\n\r");
		  else
			  uart_puts("target OFF\n\r");
	  }
	  if(payloadLength == 6){//&& string[11] == thisID
		  sprintf(targetIdString, "targetId: %d\n\r", string6[11]);
		  uart_puts(targetIdString);
		  if(string6[12])
			  uart_puts("target ON\n\r");
		  else
			  uart_puts("target OFF\n\r");
	  }
	  if(string6[11] == 'y' && string6[12] == 'y')
		  uart_puts("END\n\r");
	  //Main: (used as reader)
	  //while(1){grab character and vvvifvvv
	  //if(valid){case(type) :}
	  //TARGET CONTROL:  //type == 12
*/
