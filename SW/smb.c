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
   disable_interrupts(GLOBAL);
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
   enable_interrupts(GLOBAL);
	delay_us( TBUF );	// Wait a few microseconds
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
   disable_interrupts(GLOBAL);
mSDA_HIGH();
	mSCL_LOW();				// Clear SCL line
	delay_us( TBUF );	// Wait a few microseconds
	mSDA_LOW();				// Clear SDA line
	delay_us( TBUF );	// Wait a few microseconds
	mSCL_HIGH();			// Set SCL line
	delay_us( TBUF );	// Stop condition setup time(Tsu:sto=4.0us min)
	mSDA_HIGH();			// Set SDA line
   enable_interrupts(GLOBAL);
}


void SMB_send_bit(unsigned char bit_out)
{
   disable_interrupts(GLOBAL);
	if(bit_out==0) {mSDA_LOW();}
	else	 	   {mSDA_HIGH();}
   delay_us(3);
	mSCL_HIGH();					// Set SCL line
	delay_us( HIGHLEV );			// High Level of Clock Pulse
	mSCL_LOW();						// Clear SCL line
	delay_us( LOWLEV );			// Low Level of Clock Pulse
//	mSDA_HIGH();				    // Master release SDA line ,
   enable_interrupts(GLOBAL);
	return;
}

unsigned char SMB_Receive_bit(void)
{
	unsigned char Ack_bit;

   disable_interrupts(GLOBAL);
	mSDA_HIGH();  //_SDA_IO=1;						// SDA-input
	mSCL_HIGH();					// Set SCL line
	delay_us( HIGHLEV );			// High Level of Clock Pulse
	if(input(SDA))	Ack_bit=1;			// \ Read acknowledgment bit, save it in Ack_bit
	else		Ack_bit=0;			// /
	mSCL_LOW();						// Clear SCL line
	delay_us( LOWLEV );			// Low Level of Clock Pulse
   enable_interrupts(GLOBAL);

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
}// End of TX_bite()

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
      }/*End of while */

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
         }/*End of for*/
         shift--;
      }/*End of while*/

      //Exclusive OR between pec and crc
      for(i=0; i<=5; i++)
      {
         pec[i] ^=crc[i];
      }/*End of for*/
   } while(BitPosition>8);/*End of do-while*/

   return pec[0];
}/*End of PEC_calculation*/

int16 ReadTemp(int8 addr, int8 select)    // Read sensor RAM
{
   unsigned char arr[6];         // Buffer for the sent bytes
   int8 crc;                     // Readed CRC
   int16 temp;                   // Readed temperature

   addr<<=1;

   SMB_STOP_bit();             //If slave send NACK stop comunication
   SMB_START_bit();            //Start condition
   SMB_TX_byte(addr);
   SMB_TX_byte(RAM_Access|select);
   SMB_START_bit();            //Repeated Start condition
   SMB_TX_byte(addr);
   arr[2]=SMB_RX_byte(ACK);        //Read low data,master must send ACK
   arr[1]=SMB_RX_byte(ACK);     //Read high data,master must send ACK
   temp=MAKE16(arr[1],arr[2]);
   crc=SMB_RX_byte(NACK);         //Read PEC byte, master must send NACK
   SMB_STOP_bit();             //Stop condition

   arr[5]=addr;
   arr[4]=RAM_Access|select;
   arr[3]=addr;
   arr[0]=0;
   if (crc != PEC_calculation(arr)) temp=0; // Calculate and check CRC

   return temp;
}

void main()
{
   unsigned int16 temp, tempa;
   signed int16 ta, to;

   setup_adc_ports(NO_ANALOGS);
   setup_adc(ADC_OFF);
   setup_psp(PSP_DISABLED);
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);
   setup_timer_1(T1_DISABLED);
   setup_timer_2(T2_DISABLED,0,1);

   output_low(KLAKSON);    // Ticho
   output_high(LED);       // Blik
   delay_ms(50);
   output_low(LED);
   printf("\n\r\n\rVER: %s\n\r\n\r", VER);   // Vypis verzi

   enable_interrupts(INT_RDA);
   enable_interrupts(GLOBAL);

   flag=false;

   while (true)
   {
         float ta1, ta2, to1, to2;
         int16 s1, s2, s3, s4, s5, s6;
         int8 c;
         int8 tlacitko;

         if (flag)
         {
            flag=false;

            output_high(KLAKSON);
            delay_ms(400);
            output_low(KLAKSON);
            delay_ms(100);
            output_high(KLAKSON);
            delay_ms(700);
            output_low(KLAKSON);
         }

         tlacitko=0;

         tempa=ReadTemp(1, RAM_Tamb);       // Read temperatures from sensor
         temp=ReadTemp(1, RAM_Tobj1);
         to=(signed int16)(temp*2-27315);
         ta=(signed int16)(tempa*2-27315);
         ta1=(float)ta/100;
         to1=(float)to/100;

         if(!input(TL)) tlacitko=1;

         tempa=ReadTemp(2, RAM_Tamb);       // Read temperatures from sensor
         temp=ReadTemp(2, RAM_Tobj1);
         to=(signed int16)(temp*2-27315);
         ta=(signed int16)(tempa*2-27315);
         ta2=(float)ta/100;
         to2=(float)to/100;
//         printf("T2 %.1g %.1g ",(float)ta/100,(float)to/100);

//         printf("S1 %Lu ", sonar_ping(SONAR1));
         if(!input(TL)) tlacitko=1;
         output_high(LED);
         s1=sonar_ping(SONAR1);
         output_low(LED);
         if(!input(TL)) tlacitko=1;
         s2=sonar_ping(SONAR2);
         if(!input(TL)) tlacitko=1;
         s3=sonar_ping(SONAR3);
         if(!input(TL)) tlacitko=1;
         s4=sonar_ping(SONAR4);
         if(!input(TL)) tlacitko=1;
         s5=sonar_ping(SONAR5);
         if(!input(TL)) tlacitko=1;
         s6=sonar_ping(SONAR6);
         if(!input(TL)) tlacitko=1;
         c=cmps_azimuth();
         if(!input(TL)) tlacitko=1;

         printf("#T1 %.1g %.1g T2 %.1g %.1g ",ta1,to1,ta2,to2);
         printf("S1 %Lu S2 %Lu S3 %Lu S4 %Lu S5 %Lu S6 %Lu C %u TL %u\n\r",s1,s2,s3,s4,s5,s6,c,tlacitko);
   }

}
