/*
 * File:   mainHall.c
 * Author: Julio Shin
 * Detecta el switch del efecto Hall.
 * Created on February 21, 2019, 2:13 PM
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

#define _XTAL_FREQ 8000000

#include <xc.h>
#include <pic16f887.h>
#include "ADC.h"
#include "I2C.h"
#include "Oscilador.h"

char z, key, ADC, cont, val, door;

void setup (void);

void __interrupt() isr(void){
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
            z = SSPBUF;             // SSPBUF is of no use, so it is stored in the dummy variable.
            __delay_us(250);
            
        }else if(!SSPSTATbits.D_nA && SSPSTATbits.R_nW){
            z = SSPBUF;                 // Read to refresh the buffer and reset the BF bit.
            BF = 0;
            SSPBUF = door;               // door value is put on the SSPBUF to transmit.
            SSPCONbits.CKP = 1;         // SCL pulses are activated.
            __delay_us(250);
            while(SSPSTATbits.BF);      // It waits until transmit is complete.
        }
       
        PIR1bits.SSPIF = 0;             // Interrupt flag is cleared.
    }
    
    if (INTCONbits.T0IF == 1){
            cont++;
            if(cont < 200){
                if(cont < val){
                    PORTAbits.RA1 = 1;
                }else{
                    PORTAbits.RA1 = 0;
                }    
            } else {
                cont = 0;
            }
            TMR0 = 56;
            INTCONbits.T0IF = 0;
        }
}

void main(void) {
    setup();
    while (1){
        ADCON0bits.GO = 1;                  // ADC conversion is set to go.
        while(ADCON0bits.GO == 1){
            asm("nop");                     // While ADC conversion is not finished, the PIC does not do anything.
        }
        ADC = ADRESH;
        PORTB = key;
        __delay_ms(200);
        if (ADC > 134 | ADC < 120){
            key = 1;
        } else {
            key = 0;
        }
        if (door == 0){
            if (key == 1 & PORTAbits.RA2 == 1){
                val = 15;
                door = 1;
                while(PORTAbits.RA2 == 1);
            }
        } else if (door == 1){
            if (PORTAbits.RA2 == 1){
                val = 8;
                door = 0;
                while(PORTAbits.RA2 == 1);
            }
        }
        
    }
}

void setup (void){
    initOscilador(7);
    ANSEL = 0;
    ANSELH = 0;
    TRISA = 0;
    TRISAbits.TRISA2 = 1;
    TRISB = 0;
    PORTA = 0;
    PORTB = 0;
    val = 8;
    door = 0;
    ADC_channel(0);
    initADC(2);
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.T0SE = 0;  
    OPTION_REGbits.PSA = 1;   
    OPTION_REGbits.PS2 = 0;   
    OPTION_REGbits.PS1 = 0;
    OPTION_REGbits.PS0 = 0;
    TMR0 = 56;
    INTCONbits.T0IF = 0;
    INTCONbits.T0IE = 1;
    INTCONbits.GIE = 1;
    I2C_Slave_Init(0x10);
}
    
    