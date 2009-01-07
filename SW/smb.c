/************ SMB driver ************/

#define  SA          0x00     // Slave Address (0 for single slave / 0x5A<<1 default)
#define  RAM_Access  0x00     // RAM access command
#define  RAM_Tobj1   0x07     // To1 address in the RAM
#define  RAM_Tamb    0x06     // Ta address in the RAM


//*High and Low level of clock
#define HIGHLEV	40      // max. 50us
#define LOWLEV	   100 	  // max. 30ms
#define TBUF	   20

//SMBus control signals
#define SCL    PIN_B0
#define SDA    PIN_B1

#define mSDA_HIGH()	output_float(SDA); // SDA float
#define mSDA_LOW()   output_low(SDA);  // SDA low
#define mSCL_HIGH()	output_float(SCL); //output_high(SCL);  // SCL high
#define mSCL_LOW()   output_low(SCL);  // SCL low

#define  ACK	 0
#define	NACK   1

//**********************************************************************************************
//							START CONDITION ON SMBus
//**********************************************************************************************
//Name:			START_bit
//Function:		Generate START condition on SMBus
//Parameters:	No
//Return:		No
//Comments: 	Refer to "System Managment BUS(SMBus) specification Version 2.0"
//				or AN"SMBus communication with MLX90614" on the website www.melexis.com
//**********************************************************************************************
void SMB_START_bit(void)
{
//   disable_interrupts(GLOBAL);
	mSDA_HIGH();			// Set SDA line
	delay_us( TBUF );	// Wait a few microseconds
	mSCL_HIGH();			// Set SCL line
	delay_us( TBUF );	// Generate bus free time between Stop
							// and Start condition (Tbuf=4.7us min)
	mSDA_LOW();				// Clear SDA line
	delay_us( TBUF );	// Hold time after (Repeated) Start
							// Condition. After this period, the first clock is generated.
							//(Thd:sta=4.0us min)
	mSCL_LOW();				// Clear SCL line
//   enable_interrupts(GLOBAL);
	delay_us( TBUF );	// Wait a few microseconds
   
   toggle_dome();
}
//*********************************************************************************************
//								STOP CONDITION ON SMBus
//*********************************************************************************************
//Name:			STOPbit
//Function:		Generate STOP condition on SMBus
//Parameters:	No
//Return:		No
//Comments: 	Refer to "System Managment BUS(SMBus) specification Version 2.0"
//		    	or AN"SMBus communication with MLX90614" on the website www.melexis.com
//*********************************************************************************************
void SMB_STOP_bit(void)
{
//   disable_interrupts(GLOBAL);
mSDA_HIGH();
	mSCL_LOW();				// Clear SCL line
	delay_us( TBUF );	// Wait a few microseconds
	mSDA_LOW();				// Clear SDA line
	delay_us( TBUF );	// Wait a few microseconds
	mSCL_HIGH();			// Set SCL line
	delay_us( TBUF );	// Stop condition setup time(Tsu:sto=4.0us min)
	mSDA_HIGH();			// Set SDA line
//   enable_interrupts(GLOBAL);

   toggle_dome();
}


void SMB_send_bit(unsigned char bit_out)
{
//   disable_interrupts(GLOBAL);
	if(bit_out==0) {mSDA_LOW();}
	else	 	   {mSDA_HIGH();}
   delay_us(3);
	mSCL_HIGH();					// Set SCL line
	delay_us( HIGHLEV );			// High Level of Clock Pulse
	mSCL_LOW();						// Clear SCL line
	delay_us( LOWLEV );			// Low Level of Clock Pulse
//	mSDA_HIGH();				    // Master release SDA line ,
//   enable_interrupts(GLOBAL);

   toggle_dome();
	return;
}

unsigned char SMB_Receive_bit(void)
{
	unsigned char Ack_bit;

//   disable_interrupts(GLOBAL);
	mSDA_HIGH();  //_SDA_IO=1;						// SDA-input
	mSCL_HIGH();					// Set SCL line
	delay_us( HIGHLEV );			// High Level of Clock Pulse
	if(input(SDA))	Ack_bit=1;			// \ Read acknowledgment bit, save it in Ack_bit
	else		Ack_bit=0;			// /
	mSCL_LOW();						// Clear SCL line
	delay_us( LOWLEV );			// Low Level of Clock Pulse
//   enable_interrupts(GLOBAL);

   toggle_dome();
	return	Ack_bit;
}


//*********************************************************************************************
//								TRANSMIT DATA ON SMBus
//*********************************************************************************************
//Name:			TX_byte
//Function:		Send a byte on SMBus
//Parameters:	TX_buffer ( the byte which will be send on the SMBus )
//Return:		Ack_bit	  ( acknowledgment bit )
//Comments:  	Sends MSbit first
//*********************************************************************************************
unsigned char SMB_TX_byte(unsigned char Tx_buffer)
{
	unsigned char	Bit_counter;
	unsigned char	Ack_bit;
	unsigned char	bit_out;

	for(Bit_counter=8; Bit_counter; Bit_counter--)
	{
		if(Tx_buffer&0x80) bit_out=1; // If the current bit of Tx_buffer is 1 set bit_out
		else			   bit_out=0; // else clear bit_out

		SMB_send_bit(bit_out);			  // Send the current bit on SDA
		Tx_buffer<<=1;				  // Get next bit for checking
	}

	Ack_bit=SMB_Receive_bit();			  // Get acknowledgment bit

	return	Ack_bit;
}

//*********************************************************************************************
//									RECEIVE DATA ON SMBus
//*********************************************************************************************
//Name:			RX_byte
//Function:		Receive a byte on SMBus
//Parameters:	ack_nack (ackowlegment bit)
//Return:		RX_buffer(Received byte)
//Comments:  	MSbit is received first
//*********************************************************************************************
unsigned char SMB_RX_byte(unsigned char ack_nack)
{
	unsigned char 	RX_buffer;
	unsigned char	Bit_Counter;

	for(Bit_Counter=8; Bit_Counter; Bit_Counter--)
	{
		if(SMB_Receive_bit())						// Get a bit from the SDA line
		{
			RX_buffer <<= 1;					// If the bit is HIGH save 1  in RX_buffer
			RX_buffer |=0b00000001;
		}
		else
		{
			RX_buffer <<= 1;					// If the bit is LOW save 0 in RX_buffer
			RX_buffer &=0b11111110;
		}
	}

	SMB_send_bit(ack_nack);							// Sends acknowledgment bit

	return RX_buffer;
}


unsigned char PEC_calculation(unsigned char pec[]) // CRC calculation
{
   unsigned char   crc[6];
   unsigned char   BitPosition=47;
   unsigned char   shift;
   unsigned char   i;
   unsigned char   j;
   unsigned char   temp;

   do
   {
      crc[5]=0;            /* Load CRC value 0x000000000107 */
      crc[4]=0;
      crc[3]=0;
      crc[2]=0;
      crc[1]=0x01;
      crc[0]=0x07;
      BitPosition=47;         /* Set maximum bit position at 47 */
      shift=0;

      //Find first 1 in the transmited message
      i=5;               /* Set highest index */
      j=0;
      while((pec[i]&(0x80>>j))==0 && i>0)
      {
         BitPosition--;
         if(j<7)
         {
            j++;
         }
         else
         {
            j=0x00;
            i--;
         }
      }

      shift=BitPosition-8;   /*Get shift value for crc value*/


      //Shift crc value
      while(shift)
      {
         for(i=5; i<0xFF; i--)
         {
            if((crc[i-1]&0x80) && (i>0))
            {
               temp=1;
            }
            else
            {
               temp=0;
            }
            crc[i]<<=1;
            crc[i]+=temp;
         }
         shift--;
      }

      //Exclusive OR between pec and crc
      for(i=0; i<=5; i++)
      {
         pec[i] ^=crc[i];
      }
   } while(BitPosition>8);/*End of do-while*/

   return pec[0];
}
