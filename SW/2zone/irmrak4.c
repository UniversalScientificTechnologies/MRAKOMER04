/**** IR Mrakomer 4 ****/
#define VERSION "4.1"
#define ID "$Id$"

#include "irmrak4.h"
#include <TOUCH.C>

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
#define  ERROR          -32000   // Error flag

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
   printf("\n\r\n\r# Mrakomer %s (C) 2007 KAKL\n\r",VER);   // Welcome message
   printf("#%s\n\r",&REV[4]);
   printf("#\n\r");
   printf("# h - Switch On Heating for 20s.\n\r");
   printf("# c - Need Colder. Switch Off Heating.\n\r");
   printf("# o - Open the Dome for 20s.\n\r");
   printf("# l - Lock the Dome.\n\r");
   printf("# x - Open the Dome and switch On Heating.\n\r");
   printf("# i - Print this Information.\n\r");
   printf("# r - Repeat measure every second.\n\r");
   printf("# s - Single measure.\n\r");
   printf("# u - Update firmware. Go to the Boot Loader.\n\r");
   printf("#\n\r");
   printf("# <ver> <sequence> <inside[1/100 C]> <sky[1/100 C]> <sky[1/100 C]> ");
   printf("<ambient[1/100 C]> <heating[s]> <dome[s]> <check>\n\r\n\r");
//---WDT
   restart_wdt();
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

// compute CRC 
// *sn - pointer to the byte array
// num - length of array
inline int8 TM_check_CRC(unsigned int8 *sn, unsigned int8 num)
{
// CRC table
   const  int8 TouchCRC[256]= {
      0,  94, 188, 226,  97,  63, 221, 131, 194, 156, 126,  32, 163, 253,  31,  65,
      157, 195,  33, 127, 252, 162,  64,  30,  95,   1, 227, 189,  62,  96, 130, 220,
      35, 125, 159, 193,  66,  28, 254, 160, 225, 191,  93,   3, 128, 222,  60,  98,
      190, 224,   2,  92, 223, 129,  99,  61, 124,  34, 192, 158,  29,  67, 161, 255,
      70,  24, 250, 164,  39, 121, 155, 197, 132, 218,  56, 102, 229, 187,  89,   7,
      219, 133, 103,  57, 186, 228,   6,  88,  25,  71, 165, 251, 120,  38, 196, 154,
      101,  59, 217, 135,   4,  90, 184, 230, 167, 249,  27,  69, 198, 152, 122,  36,
      248, 166,  68,  26, 153, 199,  37, 123,  58, 100, 134, 216,  91,   5, 231, 185,
      140, 210,  48, 110, 237, 179,  81,  15,  78,  16, 242, 172,  47, 113, 147, 205,
      17,  79, 173, 243, 112,  46, 204, 146, 211, 141, 111,  49, 178, 236,  14,  80,
      175, 241,  19,  77, 206, 144, 114,  44, 109,  51, 209, 143,  12,  82, 176, 238,
      50, 108, 142, 208,  83,  13, 239, 177, 240, 174,  76,  18, 145, 207,  45, 115,
      202, 148, 118,  40, 171, 245,  23,  73,   8,  86, 180, 234, 105,  55, 213, 139,
      87,   9, 235, 181,  54, 104, 138, 212, 149, 203,  41, 119, 244, 170,  72,  22,
      233, 183,  85,  11, 136, 214,  52, 106,  43, 117, 151, 201,  74,  20, 246, 168,
      116,  42, 200, 150,  21,  75, 169, 247, 182, 232,  10,  84, 215, 137, 107,  53};

  int8 CRC;
  int8 i;

   CRC=0;
   for(i=0;i<num;i++) CRC=TouchCRC[CRC ^ *(sn+i)];
   return(CRC);
}


/*-------------------------------- MAIN --------------------------------------*/
void main()
{
   unsigned int16 seq, temp, tempa;
   signed int16 ta, to1, to2, tTouch;
   int8 tLSB,tMSB;                     // Temperatures from TouchMemory
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
   touch_present();   //Issues a reset of Touch Memory device
   touch_write_byte(0xCC);    
   touch_write_byte(0x44);

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
      ta=tempa*2-27315;    // °K -> °C

      temp=ReadTemp(SA, RAM_Tobj1);
      to1=temp*2-27315;
      temp=ReadTemp(SA, RAM_Tobj2);
      to2=temp*2-27315;

      touch_present();   //Issues a reset of Touch Memory device
      touch_write_byte(0xCC);    
      touch_write_byte(0x44);
   
//---WDT
      restart_wdt();
      delay(MEASURE_DELAY);   // Delay to a next measurement

      {
         int8 SN[10];
         int8 n;

         touch_present();   //Issues a reset and returns true if the touch device is there.
         touch_write_byte(0xCC);
         touch_write_byte(0xBE);
         for(n=0;n<9;n++) SN[n]=touch_read_byte();
         tLSB=SN[0];
         tMSB=SN[1];
         if ((SN[8]==TM_check_CRC(SN,8))&&(SN[7]==0x10)) // Check CRC and family code to prevent O's error
         {
            tTouch=make16(tMSB,tLSB); 
         }
         else
         {
            tTouch=ERROR;
         }   

for(n=0;n<9;n++) printf("%X ",SN[n]);
         
//!!!         printf("CRC %u %u ",SN[8],TM_check_CRC(SN,8)); 
//!!!            printf("%.4f ",tTouch*6.25);
      }
   
      { // printf
         char output[8];   // Output buffer
         int8 j;           // String pointer
         int8 check=0;     // Checksum is calculated between '$' and '*'

         delay(SEND_DELAY);
         putc('$');
         delay(SEND_DELAY);
         sprintf(output,"M%s \0",VER);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j]); check^=output[j++]; }
         sprintf(output,"%Lu \0", seq);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j]); check^=output[j++]; }
         sprintf(output,"%Ld \0", ta);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j]); check^=output[j++]; }
         sprintf(output,"%Ld \0", to1);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j]); check^=output[j++]; }
         sprintf(output,"%Ld \0", to2);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j]); check^=output[j++]; }
         if(tTouch==ERROR)
         {
            sprintf(output,"-27315 \0"); // Error condition
         }
         else
         {
            sprintf(output,"%Ld \0",tTouch*6+(tTouch/4)); // 1bit = 0,0625gradC
         }
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j]); check^=output[j++]; }
         sprintf(output,"%u \0", heat);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j]); check^=output[j++]; }
         sprintf(output,"%u \0", open);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j]); check^=output[j++]; }
         sprintf(output,"*%X\r\n\0", check);
         j=0; while(output[j]!=0) { delay(SEND_DELAY); putc(output[j++]); }
         delay(SEND_DELAY);
      }
      

//---WDT
      restart_wdt();
   }
}


//#include "dbloader.c" // Space reservation for the BootLoader
