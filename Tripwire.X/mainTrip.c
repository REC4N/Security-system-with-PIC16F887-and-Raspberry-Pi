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
#include <stdio.h>

#define speed 1 // Speed Range 10 to 1  10 = lowest , 1 = highest
#define steps 1000 // how much step it will take
#define clockwise 0 // clockwise direction macro
#define anti_clockwise 1 // anti clockwise direction macro

char z, val, i; 
int j;
char send[2];

void system_init (void); // This function will initialise the ports.
void full_drive (char direction); // This function will drive the motor in full drive mode
void delay(unsigned int val);

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
            val = SSPBUF;               // SSPBUF es guardado en val cuando hay una recepci�n de datos
            __delay_us(250);
            
        }else if(!SSPSTATbits.D_nA && SSPSTATbits.R_nW){
            
            z = SSPBUF;                 // Read to refresh the buffer and reset the BF bit.
            BF = 0;
            SSPBUF = send[i];               // send value is put on the SSPBUF to transmit.
            i++;                        // Se incrementa el �ndice de la lista a enviar
            if (i>1){
                i = 0;                  // Si se pasa de 1, regresa a 0
            }
            PORTAbits.RA6 = 1;
            SSPCONbits.CKP = 1;         // SCL pulses are activated.
            __delay_us(250);
            while(SSPSTATbits.BF);      // It waits until transmit is complete.
        }
       
        PIR1bits.SSPIF = 0;             // Interrupt flag is cleared.
    }
}

void main(void) {
    
    OSCCONbits.IRCF0 = 1;       // Se configura oscilador a 8 Mhz
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;
    OSCCONbits.SCS = 1;         
  
    
    i = 0;                      // Inicializaci�n de variables
    j = 1000;                   // Banco inicia abierto, j empieza en 1000
    send[0] = 0;                // Trip desactivado
    send[1] = 3;                // Banco abierto
    val = 0;
    PORTA = 0;                  // Se limpia PORTA y PORTB
    PORTB = 0;                  
    TRISA = 0;                  // Se pone PORTA como output
    TRISD = 0x02;               // RD1 como input
    ANSEL = 0;                  // Se pone PORTA como salida digital
    ANSELH = 0;
    INTCONbits.GIE = 1;         // Se activan interrupciones
    system_init();              // Inicializa el puerto a utilizar por el stepper
    I2C_Slave_Init(0x20);       // Inicializa I2C con direcci�n 0x20
    
    while(1){
        if (PORTDbits.RD1 == 1){
            send[0] = 1;        // Si el trip est� activado, se manda un 1
        } else {
            send[0] = 0;        // Si el trip se desactiva, manda un 0
        }
        
        // Si el PIC recibe un 0xFF, comienza a girar a favor de las agujas del reloj
        if (val == 0xFF){
            // Va a dar los steps necesarios para que llegue a 1000
            if (j < 1000){
                full_drive(clockwise);  // Va girando el stepper
                j++;                    // Va incrementando la variable
            } else {
                send[1] = 3;            // Cuando llega a 1000, se manda un 3 para decir que el banco est� abierto
                val = 0;                // Se reincia val
            }
        }
        
        // Si el PIC recibe un 0xFF, comienza a girar en contra de las agujas del reloj
        if (val == 0xF0){
            // Va a dar las suficientes vueltas para regresar a 0
            if (j > 0){
                full_drive(anti_clockwise); // Va girando el stepper
                j--;                        // Va incrementando la variable
            }
            else{
                send[1] = 2;                // Cuando llega a 0, se manda un 2 para decir que el banco est� cerrado
                val = 0;                    // Se reinicia val
            }   
        }
    }
}


void system_init (void){
    ANSELH = 0;
    TRISB = 0x00;     // PORT B as output port
    PORTB = 0x0F;
}

void full_drive (char direction){
    // Secuencias para poder rotar el stepper en la direcci�n deseada
    if (direction == anti_clockwise){
        PORTB = 0b00000011;
        delay(speed);
        PORTB = 0b00000110;
        delay(speed);
        PORTB = 0b00001100;
        delay(speed);
        PORTB = 0b00001001;
        delay(speed);
        PORTB = 0b00000011;
        delay(speed);
    }
    if (direction == clockwise){
        PORTB = 0b00001001;
        delay(speed);
        PORTB = 0b00001100;
        delay(speed);
        PORTB = 0b00000110;
        delay(speed);
        PORTB = 0b00000011;
        delay(speed);
        PORTB = 0b00001001;
        delay(speed);
    }
        
}

// Genera un delay para el movimiento del stepper
void delay(unsigned int val){
     unsigned int i,j;
        for(i=0;i<val;i++)
            for(j=0;j<250;j++);
}