/**** Bootloader ****/
#define VERSION "1.0"
#define ID "$Id: irmrak4.c 1293 2009-01-11 22:53:46Z kakl $"

#CASE    // Case sensitive compiler

#include "bloader.h"

#bit CREN = 0x18.4      // USART registers
#bit SPEN = 0x18.7
#bit OERR = 0x18.1
#bit FERR = 0x18.2


#INT_RDA
rs232_handler()
{
   putchar(getc());
}


/*-------------------------------- MAIN --------------------------------------*/
#SEPARATE
void real_main()
{
   int8 i=0;

   printf("/n/rBootloader/n/r");
   enable_interrupts(INT_RDA);
   enable_interrupts(GLOBAL);
   while(TRUE)
   {
      printf("|%u",i++);
      delay_ms(100);
   }
}


/*------------------- BOOT LOADER --------------------------------------------*/
#define FLASH_BLOCK_SIZE   32
#define LOADER_RESERVED    getenv("PROGRAM_MEMORY")-26*FLASH_BLOCK_SIZE
#define BUFFER_LEN_LOD     46
#if FLASH_BLOCK_SIZE != getenv("FLASH_ERASE_SIZE")/2
  #error Wrong length of the Flash Block Size. getenv("FLASH_ERASE_SIZE")/getenv("FLASH_WRITE_SIZE")
#endif


#BUILD(INTERRUPT=FLASH_BLOCK_SIZE)   // Redirect Interrupt routine above first flash block
#ORG 4,5
void JumpToTheInterrupt()     // Jump to the Interrupt Handler
{ #asm GOTO FLASH_BLOCK_SIZE #endasm }
#ORG 6,FLASH_BLOCK_SIZE-1 {} // First Flash block is reserved


#ORG LOADER_RESERVED,LOADER_RESERVED+FLASH_BLOCK_SIZE-1 auto=0
#SEPARATE
void dummy_main() // Main on the fix position
{
   real_main();
}

#ORG LOADER_RESERVED+FLASH_BLOCK_SIZE,getenv("PROGRAM_MEMORY")-125 auto=0 default

unsigned int atoi_b16(char *s)  // Convert two hex characters to an int8
{
   unsigned int result = 0;
   int i;

   for (i=0; i<2; i++,s++)  {
      if (*s >= 'A')
         result = 16*result + (*s) - 'A' + 10;
      else
         result = 16*result + (*s) - '0';
   }

   return(result);
}

void assert(int1 Condition, int8 ErrorCode)
{
   if(Condition)
   {
      putchar('E');
      putchar(ErrorCode+'1');
      reset_cpu();
   }
}

void pause()
{
   int16 timeout;

   for(timeout=0; timeout<65535; timeout++); // Delay cca 300ms
}

#SEPARATE
boot_loader()
{
   int  buffidx;
   char buffer[BUFFER_LEN_LOD];

   int8  checksum, line_type;
   int16 l_addr,h_addr=0;
   int16 addr;
   int32 next_addr;

   int8  dataidx, i, count;
   union program_data {
      int8  i8[16];
      int16 i16[8];
   } data;

   putchar('@');

//nesmaze obsluhu preruseni a jump na main
   for(addr=getenv("FLASH_ERASE_SIZE")/2;addr<=LOADER_RESERVED;addr+=getenv("FLASH_ERASE_SIZE")/2)
   {
      erase_program_eeprom(addr);
      putchar('.');
   }

   putchar('!');

   while(!kbhit()) restart_wdt();

   while(TRUE)
   {
//---WDT
      while (getc()!=':') restart_wdt(); // Only process data blocks that starts with ':'

      buffidx = 0;  // Read into the buffer until 'x' is received or buffer is full
      do
      {
         buffer[buffidx] = getc();
      } while ( (buffer[buffidx++] != 'x') && (buffidx < BUFFER_LEN_LOD) );
      assert(buffidx == BUFFER_LEN_LOD,1); // Error 1 - Buffer Overrun

//---WDT
      restart_wdt();

      checksum = 0;  // Sum the bytes to find the check sum value
      for (i=0; i<(buffidx-3); i+=2)
         checksum += atoi_b16 (&buffer[i]);
      checksum = 0xFF - checksum + 1;
      assert(checksum != atoi_b16 (&buffer[buffidx-3]),2); // Error 2 - Bad CheckSum

//      count = atoi_b16 (&buffer[0]);  // Get the number of bytes from the buffer

      // Get the lower 16 bits of address
      l_addr = make16(atoi_b16(&buffer[2]),atoi_b16(&buffer[4]));

      line_type = atoi_b16 (&buffer[6]);

      addr = make32(h_addr,l_addr);

      addr /= 2;        // PIC16 uses word addresses

      // If the line type is 1, then data is done being sent
      if (line_type == 1)
      {
         putchar('#');
         reset_cpu();
      }

      assert (line_type == 4,4);  // Error 4 - Line type 4

      {

         if (line_type == 0)
         {
            // Read old program memory content
            for (i=0,next_addr=addr;i<8;i++)
               data.i16[i]=read_program_eeprom(next_addr++);
            // Loops through all of the data and stores it in data
            // The last 2 bytes are the check sum, hence buffidx-3
            for (i=8,dataidx=0; i < buffidx-3; i += 2)
               data.i8[dataidx++]=atoi_b16(&buffer[i]);

            if (addr == 0)
            {

               // Write 8 words to the Loader location
               addr=LOADER_RESERVED;
               for (i=0;i<8;i++)
                 write_program_eeprom(addr++, data.i16[i]);
               putchar('%');
            }
            else
            if (addr > 7 && addr < LOADER_RESERVED)
            {
               // Write 8 words
               for (i=0;i<8;i++)
                 write_program_eeprom(addr++, data.i16[i]);
               putchar('*');
            }
            else putchar('.');
//---WDT
      restart_wdt();
      CREN=0; CREN=1;
         }
      }
   }
}

#ORG default

#ORG getenv("PROGRAM_MEMORY")-124,getenv("PROGRAM_MEMORY")-1 auto=0
void main()
{
   int8  timeout;

   disable_interrupts(GLOBAL);
   setup_wdt(WDT_2304MS);               // Setup Watch Dog
   setup_adc_ports(NO_ANALOGS);
   setup_adc(ADC_OFF);
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);
   setup_timer_1(T1_DISABLED);
   setup_timer_2(T2_DISABLED,0,1);
   setup_comparator(NC_NC_NC_NC);
   setup_vref(FALSE);
   setup_oscillator(OSC_8MHZ|OSC_INTRC);

   putchar('?');
   for(timeout=0; timeout<255; timeout++) //cca 50s
   {
      if (kbhit())
        if (getc()=='u') // "uf" as Update Firmware
        {
          if (getc()=='f')
          {
            restart_wdt();
            boot_loader(); // Update Firmware starter
          }
        }
        else break;
      pause();
      CREN=0; CREN=1;   // Reinitialise USART
      restart_wdt();
   };

   CREN=0; CREN=1;   // Reinitialise USART
   restart_wdt();
   goto_address(LOADER_RESERVED); // Jump to the location where is the jump to the main
}

