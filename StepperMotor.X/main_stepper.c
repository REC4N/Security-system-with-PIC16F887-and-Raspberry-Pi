/*
 * File:   main.c
 * Author: Sourav Gupta
 * By:- circuitdigest.com
 * Created on May 10, 2018, 1:26 PM
 * This program will drive a servo motor.
 */
 
// PIC16F877A Configuration Bit Settings
 
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
 
 
 
#include <xc.h>
#include <stdio.h>
 
/*
 Hardware related definition
 */
 

#define speed 1 // Speed Range 10 to 1  10 = lowest , 1 = highest
#define steps 250 // how much step it will take
#define clockwise 0 // clockwise direction macro
#define anti_clockwise 1 // anti clockwise direction macro
 
 
/*
 *Application related function and definition
 */
 
void system_init (void); // This function will initialise the ports.
void full_drive (char direction); // This function will drive the motor in full drive mode
void half_drive (char direction); // This function will drive the motor in full drive mode
void wave_drive (char direction); // This function will drive the motor in full drive mode
void delay(unsigned int val);
 
/*
 * main function starts here
 */
 
 
void main(void)
{
system_init();
OSCCONbits.IRCF0 = 1;       // Oscillator is configured to 8 MHz.
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;
    OSCCONbits.SCS = 1;  
while(1){
/* Drive the motor in full drive mode clockwise */
for(int i=0;i<steps;i++)
{
            full_drive(clockwise);
}
        
}}
 
/*System Initialising function to set the pin direction Input or Output*/
 
void system_init (void){
    ANSELH = 0;
    TRISB = 0x00;     // PORT B as output port
    PORTB = 0x0F;
}
 
/*This will drive the motor in full drive mode depending on the direction*/
 
void full_drive (char direction){
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
 
/* This method will drive the motor in half-drive mode using direction input */
 
void half_drive (char direction){
    if (direction == anti_clockwise){
        PORTB = 0b00000001;
        delay(speed);
        PORTB = 0b00000011;
        delay(speed);
        PORTB = 0b00000010;
        delay(speed);
        PORTB = 0b00000110;
        delay(speed);
        PORTB = 0b00000100;
        delay(speed);
        PORTB = 0b00001100;
        delay(speed);
        PORTB = 0b00001000;
        delay(speed);
        PORTB = 0b00001001;
        delay(speed);
    }
    if (direction == clockwise){
       PORTB = 0b00001001;
       delay(speed);
       PORTB = 0b00001000;
       delay(speed);
       PORTB = 0b00001100;
       delay(speed); 
       PORTB = 0b00000100;
       delay(speed);
       PORTB = 0b00000110;
       delay(speed);
       PORTB = 0b00000010;
       delay(speed);
       PORTB = 0b00000011;
       delay(speed);
       PORTB = 0b00000001;
       delay(speed);
    }
}
 
/* This function will drive the the motor in wave drive mode with direction input*/
 
void wave_drive (char direction){
    if (direction == anti_clockwise){
        PORTB = 0b00000001;
        delay(speed);
        PORTB = 0b00000010;
        delay(speed);
        PORTB = 0b00000100;
        delay(speed);
        PORTB = 0b00001000;
        delay(speed);
    }
     if (direction == clockwise){
        PORTB = 0b00001000;
        delay(speed);
        PORTB = 0b00000100;
        delay(speed);
        PORTB = 0b00000010;
        delay(speed);
        PORTB = 0b00000001;
        delay(speed);
    }
    
}
 
/*This method will create required delay*/
 
void delay(unsigned int val)
{
     unsigned int i,j;
        for(i=0;i<val;i++)
            for(j=0;j<250;j++);
}