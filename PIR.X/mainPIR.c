/*
 * File:   mainIR.c
 * Author: Alejandro Recancoj y Julio Shin
 * Descripcion: Control de sensor infrarojo
 * Created on 19 de febrero de 2019, 11:46 AM
 */

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
#define _XTAL_FREQ 8000000
#include <xc.h>
#include "I2C.h"
char z, key, ADC, cont, val;

void setup (void);

#define _XTAL_FREQ 8000000      // Frecuencia de oscilacion de 1 Mhz

                    // Variable de estado para indicar si el contador asciende o desciende

/*void __interrupt() isr(void){
    if(PIR1bits.SSPIF == 1){ 

        SSPCONbits.CKP = 0;
       
        if ((SSPCONbits.SSPOV) || (SSPCONbits.WCOL)){
            z = SSPBUF;                 // Read the previous value to clear the buffer
            SSPCONbits.SSPOV = 0;       // Clear the overflow flag
            SSPCONbits.WCOL = 0;        // Clear the collision bit
            SSPCONbits.CKP = 1;         // Enables SCL (Clock)
        }

        if(!SSPSTATbits.D_nA && !SSPSTATbits.R_nW) {
            //__delay_us(7);
            z = SSPBUF;                 // Read to refresh the buffer and reset the BF bit.
            //__delay_us(2);
            PIR1bits.SSPIF = 0;         // Interruption flag is cleared.
            SSPCONbits.CKP = 1;         // SCL pulses are activated.
            while(!SSPSTATbits.BF);     // Meanwhile the process is complete, no nothing.
            val = SSPBUF;             // SSPBUF is of no use, so it is stored in the dummy variable.
            __delay_us(250);
            
        }else if(!SSPSTATbits.D_nA && SSPSTATbits.R_nW){
            z = SSPBUF;                 // Read to refresh the buffer and reset the BF bit.
            BF = 0;
            SSPBUF = key;               // key value is put on the SSPBUF to transmit.
            PORTAbits.RA6 = 1;
            SSPCONbits.CKP = 1;         // SCL pulses are activated.
            __delay_us(250);
            while(SSPSTATbits.BF);      // It waits until transmit is complete.
        }
       
        PIR1bits.SSPIF = 0;             // Interrupt flag is cleared.
    }
}*/

void main(void) {
    
    OSCCONbits.IRCF0 = 1;       // Se configura oscilador a 8 Mhz
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;
    OSCCONbits.SCS = 1;         
  
    
    
    PORTA = 0;                  // Se limpia PORTD
    PORTB = 0;                  // Se pone PORTD como output
    
    TRISA = 0;               // Se pone PORTA como output
    TRISB = 0x02;
    ANSEL = 0;                  // Se pone PORTA como salida digital
    ANSELH = 0;
    /*INTCONbits.GIE = 1;
    I2C_Slave_Init(0x40);*/
    
    while(1){
        if (PORTBbits.RB1 == 0){
            PORTAbits.RA0 = 1;
            key = 1;
            __delay_ms(250);
            PORTAbits.RA0 = 0;
            
        }
        
        else{
             key = 0;
        }
    }
}