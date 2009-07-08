#include "main.h"
#include "string.h"

#define  SA          0x00     // Slave Address (0 for single slave / 0x5A<<1 default)
#define  EEPROM_Access  0x20     // EEPROM access command
#define  EEPROM_Addr 0x0E     // I2CAddr address in the eeprom
#define  RAM_Access  0x00     // RAM access command
#define  RAM_Tobj1   0x07     // To1 address in the ram
#define  RAM_Tamb    0x06     // Ta address in the ram
#define  MAXHEAT     60       // Number of cycles for heating


BYTE gethex1() {
   char digit;

   digit = getc();

   putc(digit);

   if(digit<='9')
     return(digit-'0');
   else
     return((toupper(digit)-'A')+10);
}

BYTE gethex() {
   unsigned int8 lo,hi;

   hi = gethex1();
   lo = gethex1();
   if(lo==0xdd)
     return(hi);
   else
     return( hi*16+lo );
}

int16 sonar_ping (int8 addr)
{
   int16 distance;
   int8 pom;

   i2c_start();     // So Sonar Ping
   i2c_write(addr);
   i2c_write(0x0);
   i2c_write(0x51);  // 50 mereni v palcich, 51 mereni v cm, 52 v us
   i2c_stop();
   delay_ms(70);
   i2c_start();     // Read Reflection
   i2c_write(addr);
   i2c_write(0x2);
   i2c_stop();
   i2c_start();
   i2c_write(addr+1);
   pom=i2c_read(1);
   distance=MAKE16(pom,i2c_read(0));
   i2c_stop();

   return distance;
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

   arr[5]=addr;
   arr[4]=RAM_Access|select;
   arr[3]=addr;
   arr[0]=0;
   if (crc != PEC_calculation(arr)) temp=0; // Calculate and check CRC

   return temp;
}

int16 ReadEeprom(int8 addr, int8 select)
{
   unsigned char arr[6];         // Buffer for the sent bytes
   int8 crc;                     // Readed CRC
   int16 temp;                   // Readed temperature

   i2c_stop();
   i2c_start();
   i2c_write(addr);
   i2c_write(EEPROM_Access|select);  // Select the teperature sensor in device
   i2c_start();
   i2c_write(addr);
   arr[2]=i2c_read(1);        // lo
   arr[1]=i2c_read(1);        // hi
   temp=MAKE16(arr[1],arr[2]);
   crc=i2c_read(0);           //crc
   i2c_stop();

   arr[5]=addr;
   arr[4]=EEPROM_Access|select;
   arr[3]=addr;
   arr[0]=0;
   if (crc != PEC_calculation(arr)) temp=0; // Calculate and check CRC

   return temp;
}

void WriteEeprom(int8 addr, int8 select, int16 value)
{
   unsigned char arr[6];         // Buffer for the sent bytes
   byte crc,hi,lo;

   hi=MAKE8(value,1);
   lo=MAKE8(value,0);

   arr[0]=0;
   arr[1]=hi;
   arr[2]=lo;
   arr[3]=EEPROM_Access|select;
   arr[4]=addr;
   arr[5]=0;

   crc=PEC_calculation(arr);

   i2c_stop();
   i2c_start();
   i2c_write(addr);
   i2c_write(EEPROM_Access|select);
   i2c_write(lo);
   i2c_write(hi);
   i2c_write(crc);
   i2c_stop();

   printf("CRC: %X \n\r",crc);
}

void setadresstemp()
{
   int newaddr=0;

   printf("Zadej adresu (Hex):");
   newaddr=gethex();
   putc('\n');
   putc('\r');

   WriteEeprom(SA,EEPROM_Addr,0);
   delay_ms(8);
   WriteEeprom(SA,EEPROM_Addr,newaddr);
   delay_ms(8);
}

void main()
{
   char c;
   unsigned int16 temp, tempa;
   signed int16 ta, to;
   byte addr, addr_new;

   setup_adc_ports(NO_ANALOGS);
   setup_adc(ADC_OFF);
//   setup_psp(PSP_DISABLED);
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);
   setup_timer_1(T1_DISABLED);
   setup_timer_2(T2_DISABLED, 0, 1);
   setup_oscillator(OSC_4MHZ|OSC_INTRC);

   delay_ms(100);

   printf("\n\r SETCIDLA - Martin Poviser 2008 \n\r");

   while(true)
   {
      printf("k - zapsani adresy do teplotniho cidla\n\r");
      printf("o - precteni adresy teplotniho cidla\n\r");
      printf("r - precteni teploty teplotniho cidla\n\r");
      printf("t - precteni teploty teplotniho cidla z adresy 0\n\r");
      printf("s - precteni vzdalenosti ze sonaru\n\r");
      printf("c - zmena adresy sonaru\n\r");
      printf("?");
      c=getc();
      putc('\n');
      putc('\r');

      switch(c)
      {
         case 'k':
               setadresstemp();
            break;

         case 'r':
               printf("Zadej adresu (Hex):");
               addr=gethex()<<1;
               putc('\n');
               putc('\r');

               tempa=ReadTemp(addr, RAM_Tamb);       // Read temperatures from sensor
               temp=ReadTemp(addr, RAM_Tobj1);

               to=(signed int16)(temp*2-27315);
               ta=(signed int16)(tempa*2-27315);

               printf("-- %.2g %.2g \n\r",(float)ta/100,(float)to/100);
            break;

         case 't':
               while(!kbhit())
               {
                  tempa=ReadTemp(0, RAM_Tamb);       // Read temperatures from sensor
                  temp=ReadTemp(0, RAM_Tobj1);
   
                  to=(signed int16)(temp*2-27315);
                  ta=(signed int16)(tempa*2-27315);
   
                  printf("-- %.2g %.2g %.2g\n\r",(float)ta/100,(float)to/100, (float)(ta-to)/100);
               delay_ms(1100);
               };
               getc();
            break;

         case 'o':
               printf("Adresa: %X \n\r",ReadEeprom(SA,EEPROM_Addr));
            break;

         case 's':
               printf("Zadej adresu (Hex):");
               addr=gethex();
               putc('\n');
               putc('\r');

               printf("Distance [cm]: %Lu \n\r", sonar_ping(addr));
            break;

         case 'c':
               printf("Zadej starou adresu (Hex):");
               addr=gethex();
               putc('\n');
               putc('\r');
               printf("Zadej novou adresu (Hex):");
               addr_new=gethex();
               putc('\n');
               putc('\r');

               i2c_start();
               i2c_write(addr);
               i2c_write(0x0);
               i2c_write(0xA0);  // Change address sequence
               i2c_stop();

               i2c_start();
               i2c_write(addr);
               i2c_write(0x0);
               i2c_write(0xAA);  // Change address sequence
               i2c_stop();

               i2c_start();
               i2c_write(addr);
               i2c_write(0x0);
               i2c_write(0xA5);  // Change address sequence
               i2c_stop();

               i2c_start();
               i2c_write(addr);
               i2c_write(0x0);
               i2c_write(addr_new);  // New address
               i2c_stop();
            break;
      }
   }
}

