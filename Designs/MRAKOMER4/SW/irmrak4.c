/**** IR Mrakomer 4 ****/
#define VERSION "4.0"
#define ID "$Id$"

#include "irmrak4.h"

#bit CREN = 0x18.4      // USART registers
#bit SPEN = 0x18.7
#bit OERR = 0x18.1
#bit FERR = 0x18.2

#include <string.h>

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
void main()
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
               reset_cpu();             // Update firmware
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


#include "dbloader.c" // Space reservation for the BootLoader
