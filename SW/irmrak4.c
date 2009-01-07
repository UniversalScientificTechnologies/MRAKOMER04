/**** IR Mrakomer 4 ****/
#define VERSION "4.0"
#define ID "$Id$"
#include "irmrak4.h"

#define  MAXHEAT        10       // Number of cycles for heating
#define  MAXOPEN        10       // Number of cycles for dome open
#define  MEASURE_DELAY  10000
#define  SEND_DELAY     50

#define  DOME        PIN_B4   // Dome controll port
#define  HEATING     PIN_B3   // Heating for defrosting

#bit CREN = 0x18.4      // USART registers
#bit SPEN = 0x18.7
#bit OERR = 0x18.1
#bit FERR = 0x18.2

char  VER[4]=VERSION;
char  REV[50]=ID;

int8  heat;
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

   restart_wdt();
}


#include "smb.c"


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
   unsigned int16 n, temp, tempa;
   signed int16 ta, to;

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
   printf("<#seq.> <ambient temp.> <space temp.> <status>\n\r\n\r");
   tempa=ReadTemp(SA, RAM_Tamb);       // Dummy read
   temp=ReadTemp(SA, RAM_Tobj1);

   n=0;
   heat=0;
   open=0;

//   enable_interrupts(GLOBAL);
//   enable_interrupts(INT_RDA);

   restart_wdt();

   while(TRUE)
   {
      while(kbhit()) getc();        // Flush USART buffer
      CREN=0; CREN=1;               // Reinitialise USART

      do 
      {
         delay(MEASURE_DELAY);
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
      } while (!kbhit());

      {
         char ch;
               
         ch=getc();
         
         switch (ch)
         {
            case 'h':
               heat=MAXHEAT;           // Needs heating
               break;
      
            case 'c':
               heat=0;                 // Needs colder
               break;
      
            case 'o':
               open=MAXOPEN;           // Open the dome
               break;
      
            case 'l':
               open=0;                 // Lock the dome
               break;
         }
      }

      n++;        // Increment the number of measurement

      tempa=ReadTemp(SA, RAM_Tamb);       // Read temperatures from sensor
      temp=ReadTemp(SA, RAM_Tobj1);

      ta=tempa*2-27315;    // °K -> °C
      to=temp*2-27315;

      { // printf
         char output[30];  // Output buffer
         int8 j;           // Counter
               
         sprintf(output,"#%Lu %Ld %Ld %u %u\n\r\0", n, ta, to, heat, open);
   
         j=0;
         while(output[j]!=0)
         {
            delay(SEND_DELAY);
            putc(output[j++]);
            output_toggle(DOME);
         }
      }
   }
}

