/*
 * File:   mainTraductor.c
 * Author: Julio Shin
 * Recibe todos los valores del master por medio de UART y los transmite a la Raspberry por medio SPI
 * Created on February 28, 2019, 2:44 PM
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
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
#include "Oscilador.h"
#include "SPI.h"
#include "UART.h"


void setup(void);
char val, received, info[10], i, done, j, temp;

void __interrupt() isr(void){
    if (PIR1bits.RCIF == 1){
        temp = UART_Read();
        info[i] = temp;
        i++;
        RB0 = 1;
        if (temp == 'A'){
            done = 1;
        }
    }else if(SSPIF == 1){
        val = spiRead();
        spiWrite(info[j]);
        j++;
        //__delay_us(500);
        if (j == 10){
            j = 0;
        }
        PORTDbits.RD0 = ~PORTDbits.RD0;
        SSPIF = 0;
    }  
}

void main(void) {
    setup();
    while (1){
        if (done == 1){
            info[9] = 'A';
            UART_Write(info[0]);
            UART_Write(info[1]);
            UART_Write(info[2]);
            UART_Write(info[3]);
            UART_Write(info[4]);
            UART_Write(info[5]);
            UART_Write(info[6]);
            UART_Write(info[7]);
            UART_Write(info[8]);
            done = 0;
            i = 0;
            RB0 = 0;
        }
    }
}

void setup(void){
    OSCCONbits.IRCF0 = 1;       // Oscillator is configured to 8 MHz.
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;
    OSCCONbits.SCS = 1;  
    val = 0;
    i = 0;
    j = 0;
    ANSELH = 0;
    TRISB = 0;
    PORTB = 0;
    ANSEL = 0;
    TRISD = 0;
    PORTD = 0;
    TRISAbits.TRISA5 = 1;       // Slave Select
    TRISCbits.TRISC3 = 1; 
    INTCONbits.GIE = 1;         // Habilitamos interrupciones
    INTCONbits.PEIE = 1;        // Habilitamos interrupciones PEIE
    PIR1bits.SSPIF = 0;         // Borramos bandera interrupción MSSP
    PIE1bits.SSPIE = 1;         // MSSP interruption is enabled.
    PIR1bits.RCIF = 0;
    PIE1bits.RCIE = 1;
    UART_Init(9600);
    spiInit(SPI_SLAVE_SS_EN, SPI_DATA_SAMPLE_MIDDLE, SPI_CLOCK_IDLE_LOW, SPI_IDLE_2_ACTIVE);
}