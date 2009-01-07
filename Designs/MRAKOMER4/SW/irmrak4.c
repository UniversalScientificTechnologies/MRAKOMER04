/**** IR Mrakomer 4 ****/
#define VERSION "4.0"
#define ID "$Id: irmrak3.c 1215 2008-08-08 12:25:25Z kakl $"
#include "irmrak4.h"

#define  DOME        PIN_B4   // Dome controll port
#define  SA          0x00     // Slave Address (0 for single slave / 0x5A<<1 default)
#define  RAM_Access  0x00     // RAM access command
#define  RAM_Tobj1   0x07     // To1 address in the eeprom
#define  RAM_Tamb    0x06     // Ta address in the eeprom
#define  HEATING     PIN_B3   // Heating for defrosting
#define  MAXHEAT     10       // Number of cycles for heating
#define  MAXOPEN     10       // Number of cycles for dome open

#bit CREN = 0x18.4      // USART registers
#bit SPEN = 0x18.7
#bit OERR = 0x18.1
#bit FERR = 0x18.2

char  VER[4]=VERSION;
char  REV[50]=ID;

int8  heat;
int8  open;


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

   disable_interrupts(GLOBAL);
   i2c_stop();
   i2c_start();
   i2c_write(addr);
   i2c_write(RAM_Access|select);  // Select the teperature sensor in device
   i2c_start();
   i2c_write(addr);
   arr[2]=i2c_read(1);        // lo
   arr[1]=i2c_read(1);        // hi
   temp=MAKE16(arr[1],arr[2]);
   crc=i2c_read(0);           //crc
   i2c_stop();
   enable_interrupts(GLOBAL);

   arr[5]=addr;
   arr[4]=RAM_Access|select;
   arr[3]=addr;
   arr[0]=0;
   if (crc != PEC_calculation(arr)) temp=0; // Calculate and check CRC

   return temp;
}

void delay(int16 cycles)
{
   int16 i;
   
   if (open>0)
      for(i=0; i<cycles; i++) {output_toggle(DOME); delay_us(100);}
      else
      for(i=0; i<cycles; i++) {output_low(DOME); delay_us(100);}      

      restart_wdt();
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
         delay(10000);
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
         char output[30];
         int8 j;
               
         sprintf(output,"#%Lu %Ld %Ld %u %u\n\r\0", n, ta, to, heat, open);
   
         j=0;
         while(output[j]!=0)
         {
            delay(50);
            putc(output[j++]);
            output_toggle(DOME);
         }
      }
   }
}

