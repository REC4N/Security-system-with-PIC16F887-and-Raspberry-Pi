/* 
 * File:   oscilador.h
 * Author: Julio Shin
 * Librería para el oscilador del PIC16F887
 * Created on January 15, 2019, 2:28 PM
 */

#pragma config FOSC = INTRC_NOCLKOUT
// Funcion para seleccionar el oscilador interno del PIC

void initOscilador(char option){
    
    switch(option){
        case 0:
            // Oscilador a 31kHz
            OSCCONbits.IRCF = 0;
            break;
        case 1:
            // Oscilador a 125kHz
            OSCCONbits.IRCF = 1;
            break;
        case 2:
            // Oscilador a 250kHz
            OSCCONbits.IRCF = 2;
            break;
        case 3:
            // Oscilador a 500kHz
            OSCCONbits.IRCF = 3;
            break;
        case 4:
            // Oscilador a 1MHz
            OSCCONbits.IRCF = 4;
            break;
        case 5:
            // Oscilador a 2MHz
            OSCCONbits.IRCF = 5;
            break;
        case 6:
            // Oscilador a 4MHz
            OSCCONbits.IRCF = 6;
            break;
        case 7:
            // Oscilador a 8MHz
            OSCCONbits.IRCF = 7;
            break;
        default:
            // Oscilador a 8MHz
            OSCCONbits.IRCF = 7;
            break;
    }
    OSCCONbits.SCS = 1; 
    
}