#include "C:\projekts\PIC_SW\PIC16F88\RS232\rs232.h"

#define RCSTA *0x18
 #define CREN 4
 #define OERR 1
 #define FERR 2

#define _BV(a) (1<<(a))

#int_RDA
RDA_isr()
{
   putc(getc());
}



char bit_1,bit_2,bit_3,bit_4;
unsigned int time;

void main() {

   //char a,b,c;

   setup_adc_ports(NO_ANALOGS|VSS_VDD);
   setup_adc(ADC_OFF);
   setup_spi(FALSE);
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);
   setup_timer_1(T1_DISABLED);
   setup_timer_2(T2_DISABLED,0,1);
   setup_comparator(NC_NC_NC_NC);
   setup_vref(FALSE);
   //enable_interrupts(INT_RDA);
   //enable_interrupts(GLOBAL);
   setup_oscillator(OSC_4MHZ);

   input(PIN_B2);

   printf("\ftest rs232\n\r");

   time = 0;
   for(;;)
   {
      bit_1 = getc();
      delay_ms(time);

      if (RCSTA & (_BV(OERR))) {printf ("1_RX_halt, overrun\r\n");RCSTA &= (~(1<<CREN));RCSTA |= (1<<CREN);}
      if (RCSTA & (1 << FERR)) printf ("1_RX_halt, frame error\r\n");
      bit_2 = getc();
      delay_ms(time);

      if (RCSTA & (1 << OERR)) {printf ("2_RX_halt, overrun\n\r");RCSTA &= (~(1<<CREN));RCSTA |= (1<<CREN);}
      if (RCSTA & (1 << FERR)) printf ("2_RX_halt, frame error\r\n");
      bit_3 = getc();
      delay_ms(time);

      if (RCSTA & (1 << OERR)) {printf ("3_RX_halt, overrun\r\n");RCSTA &= (~(1<<CREN));RCSTA |= (1<<CREN);}
      if (RCSTA & (1 << FERR)) printf ("3_RX_halt, frame error\r\n");
      bit_4 = getc();
      delay_ms(time);

      if (RCSTA & (1 << FERR)) printf ("4_RX_halt, frame error\r\n");
      if (RCSTA & (1 << OERR)) {printf ("4_RX_halt, overrun\r\n");RCSTA &= (~(1<<CREN));RCSTA |= (1<<CREN);};


      putc(bit_1);
      putc(bit_2);
      putc(bit_3);
      putc(bit_4);
      putc('\r');
      putc('\n');
      time = (bit_1 - 0x30) * 1000 + (bit_2 - 0x30)*100 + (bit_3 - 0x30)*10 + bit_4 - 0x30;
      if (time > 1000) time = 0;

   };

}
