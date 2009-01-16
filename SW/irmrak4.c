/**** IR Mrakomer 4 ****/
#define VERSION "4.0"
#define ID "$Id$"

#include "irmrak4.h"

#bit CREN = 0x18.4      // USART registers
#bit SPEN = 0x18.7
#bit OERR = 0x18.1
#bit FERR = 0x18.2

#include <string.h>
//!!! #include "bloader.c"             // Boot Loader driver

#CASE    // Case sensitive compiler

#define  MAXHEAT        20       // Number of cycles for heating
#define  MAXOPEN        20       // Number of cycles for dome open
#define  MEASURE_DELAY  6000     // Delay to a next measurement
#define  RESPONSE_DELAY 100      // Reaction time after receiving a command
#define  SAFETY_COUNT   90       // Time of one emergency cycle
#define  SEND_DELAY     50       // Time between two characters on RS232

#define  DOME        PIN_B4   // Dome controll port
#define  HEATING     PIN_B3   // Heating for defrosting


char  VER[4]=VERSION;   // Buffer for concatenate of a version string

int8  heat;    // Status variables
int8  open;

inline void toggle_dome(void)    // Wire exercise
{
   if (open>0)
   {output_toggle(DOME);}  // Toggle = Open Dome
   else
   {output_high(DOME);}    // Do not toggle = Close Dome
}

void delay(int16 cycles)         // Wire exercise with delay
{
   int16 i;

   for(i=0; i<cycles; i++) {toggle_dome(); delay_us(100);}
}

void welcome(void)               // Welcome message
{
   char  REV[50]=ID;       // Buffer for concatenate of a version string

   if (REV[strlen(REV)-1]=='$') REV[strlen(REV)-1]=0;
   printf("\n\r# Mrakomer %s (C) 2007 KAKL\n\r",VER);   // Welcome message
   printf("#%s\n\r",&REV[4]);
   printf("# <sequence> <ambient[1/100 C]> <sky[1/100 C]> ");
   printf("<heating[s]> <dome[s]> <check>\n\r\n\r");
}


#include "smb.c"                 // System Management Bus driver


// Read sensor's RAM
// Returns temperature in °K
int16 ReadTemp(int8 addr, int8 select)
{
   unsigned char arr[6];         // Buffer for the sent bytes
   int8 crc;                     // Readed CRC
   int16 temp;                   // Readed temperature

   addr<<=1;

   SMB_STOP_bit();               //If slave send NACK stop comunication
   SMB_START_bit();              //Start condition
   SMB_TX_byte(addr);
   SMB_TX_byte(RAM_Access|select);
   SMB_START_bit();              //Repeated Start condition
   SMB_TX_byte(addr);
   arr[2]=SMB_RX_byte(ACK);      //Read low data,master must send ACK
   arr[1]=SMB_RX_byte(ACK);      //Read high data,master must send ACK
   temp=make16(arr[1],arr[2]);
   crc=SMB_RX_byte(NACK);        //Read PEC byte, master must send NACK
   SMB_STOP_bit();               //Stop condition

   arr[5]=addr;
   arr[4]=RAM_Access|select;
   arr[3]=addr;
   arr[0]=0;
   if (crc != PEC_calculation(arr)) temp=0; // Calculate and check CRC

   return temp;
}


/*-------------------------------- MAIN --------------------------------------*/
void real_main()
{
   unsigned int16 seq, temp, tempa;
   signed int16 ta, to;
   int8 safety_counter;
   int1 repeat;

   output_high(DOME);                   // Close Dome
   output_low(HEATING);                 // Heating off

   delay_ms(1000);
   restart_wdt();

   seq=0;         // Variables initiation
   heat=0;
   open=0;
   repeat=TRUE;

   welcome();

   tempa=ReadTemp(SA, RAM_Tamb);       // Dummy read
   temp=ReadTemp(SA, RAM_Tobj1);

   delay_ms(1000);
//---WDT
   restart_wdt();

   while(TRUE)    // Main Loop
   {
      safety_counter=SAFETY_COUNT;  // Heating and Dome  Count Down
      do
      {
         if (safety_counter<SAFETY_COUNT) safety_counter++;

         delay(RESPONSE_DELAY);

         if (safety_counter>=SAFETY_COUNT)
         {
            if (heat>0) heat--;
            if (open>0) open--;

            if (heat>0) { output_high(HEATING); } else { output_low(HEATING); }

            safety_counter=0;
//---WDT
            restart_wdt();
         }
      } while (!kbhit()&&!repeat);

//---WDT
      restart_wdt();
      {                 // Retrieve command
         char ch='k';

         if(kbhit()) ch=getc();

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

            case 'x':
               open=MAXOPEN;           // Open the dome
               heat=MAXHEAT;           // Need heating
               break;

            case 'l':
               open=0;                 // Lock the dome
               break;

            case 'i':
               if (open==0) welcome(); // Information about version, etc...
               break;                  // Only when dome is closed

            case 'r':
               repeat=TRUE;            // Repeated measure mode
               break;

            case 's':
               repeat=FALSE;            // Single measure mode
               break;

            case 'u':
//               load_program();          // Update firmware
         }
      }
//      while(kbhit()) getc();        // Flush USART buffer
      CREN=0; CREN=1;               // Reinitialise USART

      seq++;        // Increment the number of measurement

      tempa=ReadTemp(SA, RAM_Tamb);       // Read temperatures from sensor
      temp=ReadTemp(SA, RAM_Tobj1);

      ta=tempa*2-27315;    // °K -> °C
      to=temp*2-27315;

      { // printf
         char output[8];   // Output buffer
         int8 j;           // String pointer
         int8 check=0;     // Checksum is calculated between '$' and '*'

         delay(SEND_DELAY);
         putc('$');
         delay(SEND_DELAY);
         sprintf(output,"M%s ",VER);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j]); check^=output[j++]; }
         sprintf(output,"%Lu ", seq);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j]); check^=output[j++]; }
         sprintf(output,"%Ld ", ta);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j]); check^=output[j++]; }
         sprintf(output,"%Ld ", to);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j]); check^=output[j++]; }
         sprintf(output,"%u ", heat);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j]); check^=output[j++]; }
         sprintf(output,"%u ", open);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j]); check^=output[j++]; }
         sprintf(output,"*%X\n\r\0", check);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j++]); }
         delay(SEND_DELAY);
      }

      delay(MEASURE_DELAY);   // Delay to a next measurement
//---WDT
      restart_wdt();
   }
}


/*------------------- BOOT LOADER --------------------------------------------*/
#define LOADER_RESERVED    getenv("PROGRAM_MEMORY")-getenv("FLASH_ERASE_SIZE")-800
#define BUFFER_LEN_LOD     46

#ORG LOADER_RESERVED,getenv("PROGRAM_MEMORY")-201 auto=0 default

unsigned int atoi_b16(char *s) {  // Convert two hex characters to a int8
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

boot_loader()
{
   int  buffidx;
   char buffer[BUFFER_LEN_LOD];

   int8  checksum, line_type;
   int16 l_addr,h_addr=0;
   int32 addr;
   #if getenv("FLASH_ERASE_SIZE")>2
      int32 next_addr;
   #endif

//!!! #error ble getenv("FLASH_ERASE_SIZE") getenv("FLASH_WRITE_SIZE")

   int8  dataidx, i, count;
   union program_data {
      int8  i8[16];
      int16 i16[8];
   } data;

   putchar('@');

//!!!nesmaze obsluhu preruseni
   for(i=getenv("FLASH_ERASE_SIZE")+1;i<LOADER_RESERVED;i+=getenv("FLASH_ERASE_SIZE"))
     erase_program_eeprom(i);

   putchar('@');

   while(TRUE)
   {
//---WDT
//!!! musi fungovat watchdog
      while (getc()!=':') restart_wdt(); // Only process data blocks that starts with ':'

      buffidx = 0;  // Read into the buffer until 'x' is received or buffer is full
      do
      {
         buffer[buffidx] = getc();
      } while ( (buffer[buffidx++] != 'x') && (buffidx < BUFFER_LEN_LOD) );
      assert(buffidx == BUFFER_LEN_LOD,1); // Overrun buffer?

//---WDT
      restart_wdt();

      checksum = 0;  // Sum the bytes to find the check sum value
      for (i=0; i<(buffidx-3); i+=2)
         checksum += atoi_b16 (&buffer[i]);
      checksum = 0xFF - checksum + 1;
      assert(checksum != atoi_b16 (&buffer[buffidx-3]),2); // Bad CheckSum?

      count = atoi_b16 (&buffer[0]);  // Get the number of bytes from the buffer

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

      assert (line_type == 4,4);


//!!! pozor, nevypalilo by to obsluhu preruseni
      if (addr > 3 || addr < LOADER_RESERVED)
      {

         if (line_type == 0)
         {
            for (i=0,next_addr=addr;i<8;i++)
               data.i16[i]=read_program_eeprom(next_addr++);
            // Loops through all of the data and stores it in data
            // The last 2 bytes are the check sum, hence buffidx-3
            for (i=8,dataidx=0; i < buffidx-3; i += 2)
               data.i8[dataidx++]=atoi_b16(&buffer[i]);

               write_program_memory(addr, data.i8, count);
         }
putchar('*');
      }
   }
}

#ORG default

#ORG getenv("PROGRAM_MEMORY")-200,getenv("PROGRAM_MEMORY")-1
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

/*
   for(timeout=0; timeout<(3*20); timeout++) //cca 20s
    if (kbhit())
    {
      if (getc()=='u') if (getc()=='f') boot_loader(); // Update Firmware starter
      pause();
      CREN=0; CREN=1;
      restart_wdt();
    };
*/
   real_main();
}

#include "dbloader.c"
