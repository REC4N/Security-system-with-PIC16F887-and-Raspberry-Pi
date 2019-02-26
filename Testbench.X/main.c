#define _XTAL_FREQ 8000000

#define RS RB4
#define EN RB5
#define D4 RB0
#define D5 RB1
#define D6 RB2
#define D7 RB3


// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include "lcd.h"
#include "ADC.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void readADC(void);
float ADC;                          // Variable that will display voltage value of potentiometer.
    char buffer[3];                         // Array to use the sprintf function and display voltage on LCD.
void main(void)
{
  OSCCONbits.IRCF0 = 1;       // Se configura oscilador a 8 Mhz
  OSCCONbits.IRCF1 = 1;
  OSCCONbits.IRCF2 = 1;
  OSCCONbits.SCS = 1; 
  unsigned int a;
  ANSELH = 0;
  ANSEL = 0x01;
  TRISA = 0x01;
  
  TRISD = 0x00;
  TRISB = 0x00;
  ADC_init(0x00, 0x00);                   // ADC is initialized with Fosc/32 on channel 0.
  Lcd_Init();
  
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("LCD Library for");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String("MPLAB XC8");
    __delay_ms(2000);
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("Developed By");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String("RECAN");
    __delay_ms(2000);
    Lcd_Clear();
    
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("Empezando ADC");
    __delay_ms(2000);
    
    Lcd_Clear();
    while(1)
  {
    readADC();
    ADC = ADC / 51;             // The voltage is divided by 51 to get a value from 0 to 5.
        sprintf(buffer, "%3.2f", ADC);  // sprintf function is used to convert the float value of voltage into a string using the array of buffer.
        Lcd_Set_Cursor(1,1);               // Cursor is set to first row, 10th column.
        Lcd_Write_String(buffer);           // voltage value (as a string) is displayed on the LCD.
        Lcd_Set_Cursor(1,5);               // Cursor is set to first row, 14th column.
        Lcd_Write_Char('V');                // 'V' is diplayed on the LCD.
    
    
    

  }
  
}

void readADC(void){
    __delay_us(500);                    // Delay of 500us for the ADC to finish any pending tasks.
    ADCON0bits.GO = 1;                  // ADC conversion is set to go.
    while(ADCON0bits.GO == 1){
        asm("nop");                     // While ADC conversion is not finished, the PIC does not do anything.
    }
    ADC = ADRESH;                       // The result of the ADC conversion, stored on register ADRESH is put in the ADC variable.
    PIR1bits.ADIF = 0;                  // Flag of ADC is cleared.
}