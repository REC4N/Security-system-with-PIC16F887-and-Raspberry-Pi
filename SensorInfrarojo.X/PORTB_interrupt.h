/*
 * File:   main.c
 * Author: Alejandro Recancoj
 * Lab 1 - Digital 2 - Libreria de activacion de Interrupt on change del PORTB
 * Created on 16 de enero de 2019, 10:45 PM
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.  

#endif	/* XC_HEADER_TEMPLATE_H */

void PORTB_interrupt_init(){
    PORTB = 0;                    // Se limpia PORTB
    TRISB = 0x02;                 // Se activan como inputs todos los pines de PORTB
    ANSELH = 0;                   // Se activan pines como digitales
    
    IOCBbits.IOCB1 = 1;
    INTCONbits.RBIF = 0;          // Se limpia bandera de interrupt on change de PORTB
    INTCONbits.RBIE = 1;          // Se activa el interrupt on change del PORTB
}

