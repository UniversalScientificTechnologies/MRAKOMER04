/**** IR Mrakomer 4 ****/
#define VERSION "4.0"
#define ID "$Id$"
#include "irmrak4.h"

#define  MAXHEAT        20       // Number of cycles for heating
#define  MAXOPEN        20       // Number of cycles for dome open
#define  MEASURE_DELAY  10000    // Delay to a next measurement
#define  RESPONSE_DELAY 100      // Reaction time after receiving a command
#define  SAFETY_COUNT   100      // Time of one emergency cycle
#define  SEND_DELAY     50       // Time between two characters on RS232

#define  DOME        PIN_B4   // Dome controll port
#define  HEATING     PIN_B3   // Heating for defrosting

#bit CREN = 0x18.4      // USART registers
#bit SPEN = 0x18.7
#bit OERR = 0x18.1
#bit FERR = 0x18.2

char  VER[4]=VERSION;   // Buffer for concatenate of a version string
char  REV[50]=ID;

int8  heat;    // Status variables
int8  open;

inline void toggle_dome(void)
{
   if (open>0)
      {output_toggle(DOME);}
      else
      {output_low(DOME);}
}

void delay(int16 cycles)
{
   int16 i;

   for(i=0; i<cycles; i++) {toggle_dome(); delay_us(100);}
}


#include "smb.c"


// Read sensor RAM
// Returns temperature in �K
int16 ReadTemp(int8 addr, int8 select)
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
   unsigned int16 seq, temp, tempa;
   signed int16 ta, to;
   int8 safety_counter;

   output_low(HEATING);                 // Heating off
   setup_wdt(WDT_2304MS);               // Setup Watch Dog
   setup_adc_ports(NO_ANALOGS);
   setup_adc(ADC_OFF);
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);
   setup_timer_1(T1_DISABLED);
   setup_timer_2(T2_DISABLED,0,1);
   setup_comparator(NC_NC_NC_NC);
   setup_vref(FALSE);
//   setup_oscillator(OSC_4MHZ|OSC_INTRC,+2); // Pokud je nutna kalibrace RCosc
   setup_oscillator(OSC_4MHZ|OSC_INTRC);

   delay_ms(1000);
   restart_wdt();
   printf("\n\r* Mrakomer %s (C) 2007 KAKL *\n\r",VER);   // Welcome message
   printf("* %s *\n\r",REV);
   printf("<#sequence> <ambient [1/100 C]> <sky [1/100 C]> ");
   printf("<heating [s]> <dome [s]>\n\r\n\r");
   tempa=ReadTemp(SA, RAM_Tamb);       // Dummy read
   temp=ReadTemp(SA, RAM_Tobj1);

   seq=0;
   heat=0;
   open=0;

//   enable_interrupts(GLOBAL);
//   enable_interrupts(INT_RDA);

//---WDT
   restart_wdt();

   while(TRUE)
   {
      while(kbhit()) getc();        // Flush USART buffer
      CREN=0; CREN=1;               // Reinitialise USART

      safety_counter=0;

      do
      {
         if (safety_counter<SAFETY_COUNT) safety_counter++;

         delay(RESPONSE_DELAY);

         if (safety_counter>=SAFETY_COUNT)
         {
            if (heat>0)
               {
                  output_high(HEATING);
                  heat--;
               }
               else
               {
                  output_low(HEATING);
               }

            if (open>0) open--;

            safety_counter=0;
//---WDT
            restart_wdt();
         }
      } while (!kbhit());

//---WDT
      restart_wdt();
      {
         char ch;

         ch=getc();

         switch (ch)
         {
            case 'h':
               heat=MAXHEAT;           // Need heating
               break;

            case 'c':
               heat=0;                 // Need colder
               break;

            case 'o':
               open=MAXOPEN;           // Open the dome
               break;

            case 'l':
               open=0;                 // Lock the dome
               break;
         }
      }

      seq++;        // Increment the number of measurement

      tempa=ReadTemp(SA, RAM_Tamb);       // Read temperatures from sensor
      temp=ReadTemp(SA, RAM_Tobj1);

      ta=tempa*2-27315;    // �K -> �C
      to=temp*2-27315;

      { // printf
         char output[10];  // Output buffer
         int8 j;           // Counter

         delay(SEND_DELAY);
         sprintf(output,"#%Lu ", seq);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j++]); }
         sprintf(output,"%Ld ", ta);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j++]); }
         sprintf(output,"%Ld ", to);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j++]); }
         sprintf(output,"%u", heat);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j++]); }
         sprintf(output,"%u\n\r\0", open);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j++]); }
      }

      delay(MEASURE_DELAY);   // Delay to a next measurement
//---WDT
      restart_wdt();
   }
}

