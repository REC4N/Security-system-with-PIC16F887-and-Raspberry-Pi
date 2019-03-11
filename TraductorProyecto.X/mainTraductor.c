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
char val, send[10], info[10], i, done, j, temp, k;

void __interrupt() isr(void){
    if (PIR1bits.RCIF == 1){
        // Interrupción por recepción UART
        temp = UART_Read();         // Lee el buffer si llega algún dato
        info[i] = temp;             // Ingresa el valor a una lista
        i++;                        // Incrementa el índice de la lista
        if (temp == 'A'){           // Si recibe una letra A, enciende la bandera done
            done = 1;
        }
    }else if(SSPIF == 1){
        // Interrupción SPI
        val = spiRead();            // Obtiene el valor del buffer del SPI, no es utilizado para nada
        spiWrite(send[j]);          // Envía el valor de la lista que serán enviados
        j++;                        // Incrementa el índice de la lista
        if (j == 10){               // Si llega a 10, la lista inicia nuevamente en 0
            j = 0;
        }
        SSPIF = 0;
    }  
}

void main(void) {
    setup();
    while (1){
        // Si la bandera de done se activa, mueve los valores de la lista a los valores que serán enviados
        if (done == 1){
            info[9] = 'A';
            for (k = 0; k < 10; k++){       //Asignando valores de la lista recibida a la lista de envío
                send[k] = info[k];
            }
            done = 0;       // Desactiva la bandera de done
            i = 0;          // Reinicia el valor del índice de la lista
        }
    }
}

void setup(void){
    OSCCONbits.IRCF0 = 1;       // Oscillator is configured to 8 MHz.
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;
    OSCCONbits.SCS = 1;  
    val = 0;                    //Inicialización de variables
    i = 0;
    j = 0;
    TRISAbits.TRISA5 = 1;       // Slave Select
    TRISCbits.TRISC3 = 1; 
    INTCONbits.GIE = 1;         // Habilitamos interrupciones
    INTCONbits.PEIE = 1;        // Habilitamos interrupciones PEIE
    PIR1bits.SSPIF = 0;         // Borramos bandera interrupción MSSP
    PIE1bits.SSPIE = 1;         // MSSP interruption is enabled.
    PIR1bits.RCIF = 0;
    PIE1bits.RCIE = 1;
    UART_Init(9600);            // Inicialización del UART
    spiInit(SPI_SLAVE_SS_EN, SPI_DATA_SAMPLE_MIDDLE, SPI_CLOCK_IDLE_LOW, SPI_IDLE_2_ACTIVE);        // Inicialización del SPI
}