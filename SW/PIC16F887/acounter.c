//*** Atomic counter up to 800 MHz ***
#include "acounter.h"
#include <string.h>

#define LED    PIN_C1  //CHANGE PIN_XX TO YOUR LED PIN NUMBER, EX: PIN_A5
#define SEL0   PIN_E0   // external counter division ratio
#define SEL1   PIN_E1   // external counter division ratio
#define MR     PIN_E2   // external counter master reset
#define CLKI   PIN_C0   // internal counter input
#define BEEP   PIN_C3   // buzzer

// LCD definitions
#define LCD_ENABLE_PIN  PIN_D4                                    ////
#define LCD_RS_PIN      PIN_D6                                    ////
#define LCD_RW_PIN      PIN_D5                                    ////
#define LCD_DATA4       PIN_D0                                    ////
#define LCD_DATA5       PIN_D1                                    ////
#define LCD_DATA6       PIN_D2                                    ////
#define LCD_DATA7       PIN_D3 
#include <lcd.c>

int16 of=0; // count of overflow
int1  flag; // flag for a blinking dot 

// GPS setup for frequency measurement
const char cmd[40]={0xB5, 0x62, 0x06, 0x31, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x80, 0x84, 0x1E, 0x00, 0xE0, 0xC8, 0x10, 0x00, 0x40, 0x42, 0x0F, 0x00, 0xA0, 0x86, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7, 0x00, 0x00, 0x00, 0x12, 0x03};


#int_EXT  // Interrupt from 1PPS
void  EXT_isr(void) 
{
   unsigned int16 countH;
   unsigned int8  countL;
   unsigned int32 count;
   char countS[10], a[4], b[4], c[4];  // strings for printing results
   
   countL=0;
   countH=get_timer1();    // read internal counter
   output_low(SEL0);
   output_low(SEL1);
   countL=input(CLKI);     // read bit 0 of external counter
   output_high(SEL0);
   output_low(SEL1);
   countL|=input(CLKI)<<1; // read bit 1 of external counter
   output_low(SEL0);
   output_high(SEL1);
   countL|=input(CLKI)<<2; // read bit 2 of external counter
   output_high(SEL0);
   output_high(SEL1);
   countL|=input(CLKI)<<3; // read bit 3 of external counter

   output_low(MR);   // External counter Master Reset
   output_high(MR);
   
   set_timer1(0);    // Internal counter reset
   
   count=((unsigned int32)of<<20)+((unsigned int32)countH<<4)+(unsigned int32)countL; // concatenate 

   sprintf(countS,"%09Lu", count);  // engeneering values conversion
   strncpy(a, countS, 3); a[3]='\0';
   strncpy(b, &countS[3], 3); b[3]='\0';
   strncpy(c, &countS[6], 3); c[3]='\0';
   
   printf("%s\r\n", countS);                    // output to RS232
   if(flag==0){lcd_putc("\fCvak... \n"); flag=1;} else {lcd_putc("\fCvak....\n"); flag=0;};
   printf(lcd_putc, "%s %s %s Hz\n", a, b, c);  // output to LCD

   output_toggle(BEEP); // cvak...

   of=0; // reset overflow counter
}

#int_TIMER1  // Interrupf from overflow
void  TIMER1_isr(void) 
{
   of++;
}

void main()
{

   setup_adc_ports(NO_ANALOGS|VSS_VDD);
   setup_adc(ADC_CLOCK_DIV_2);
   setup_spi(SPI_SS_DISABLED);
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);
   setup_timer_1(T1_EXTERNAL|T1_DIV_BY_1);
   setup_timer_2(T2_DISABLED,0,1);
   setup_ccp1(CCP_OFF);
   setup_comparator(NC_NC_NC_NC);// This device COMP currently not supported by the PICWizard

   output_toggle(BEEP); // cvak...
   
   ext_int_edge( L_TO_H );       // set 1PPS active edge
   enable_interrupts(INT_TIMER1);
   enable_interrupts(INT_EXT);
   enable_interrupts(GLOBAL);

   lcd_init();
   delay_ms(100);
   lcd_putc("\f  ACOUNTER02A\n (c)mlab.cz 2014\n");


   delay_ms(1000);      //wait for GPS init.
   int n;
   for (n=0;n<40;n++) putc(cmd[n]); // send setup to GPS

   lcd_putc("\fCvak...\nHmmm...\n");

   while(true)
   {
   }

}
