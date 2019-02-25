/*
 * File:   mainMaster.c
 * Author: Alejandro Recancoj y Julio Shin
 * Main del proyecto 1. Recolecta información de la red I2C y controla los actuadores del proyecto.
 * Created on February 19, 2019, 11:35 AM
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

#define RS PORTBbits.RB4
#define EN PORTBbits.RB5
#define D4 PORTBbits.RB0
#define D5 PORTBbits.RB1
#define D6 PORTBbits.RB2
#define D7 PORTBbits.RB3

#define _XTAL_FREQ 8000000

#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "I2C.h"
#include "LCD4bits.h"
#include "Oscilador.h"

char *time, *temp, key;

void setup (void);
void write_RTC(char sec, char hour, char minutes, char day);
char* get_time(void);
char* get_temp(void);
char get_hall(void);

void main(void) {
    setup();
    Lcd_Clear();
    //write_RTC(0x00, 0x15, 0x22, 0x06);
    while (1) {
        time = get_time();
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String(time);
        temp = get_temp();
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String(temp);
        key = get_hall();
        Lcd_Set_Cursor(1,7);
        if (key == 1){
            Lcd_Write_String("Key ON  ");
        } else {
            Lcd_Write_String("Key OFF ");
        }
        
        if (strcmp(temp,"23.50") > 0){
            PORTAbits.RA0 = 1;
        } else {
            PORTAbits.RA0 = 0;
        }
        
    }  
}

void setup (void){
    initOscilador(7);           // Oscilador interno de 8 MHz
    ANSELH = 0;                 //Puerto B digital
    ANSEL = 0;
    TRISB = 0;                  //PORTB como output
    TRISA = 0;
    PORTB = 0;                  //Inicializar puertos
    PORTA = 0;
    Lcd_Init();                 //Inicializar LCD
    I2C_Master_Init(100000);        // Inicializar Comuncación I2C
}

void write_RTC (char sec, char hour, char minutes, char day) {
    I2C_Master_Start();
    I2C_Master_Write(0xD0);
    I2C_Master_Write(0x00);
    I2C_Master_Write(sec);
    I2C_Master_Write(minutes);
    I2C_Master_Write(hour);
    I2C_Master_Write(day);
    I2C_Master_Stop();
}

char* get_time (void){
    char hour, min;
    char string_time[6];
    
    I2C_Master_Start();
    I2C_Master_Write(0xD0);
    I2C_Master_Write(0x01);         
    I2C_Master_RepeatedStart();     
    I2C_Master_Write(0xD1);
    min = I2C_Master_Read(1);      
    hour = I2C_Master_Read(0);      
    I2C_Master_Stop();
    
    string_time[0] = (hour >> 4) + '0';
    string_time[1] = (hour & 0x0F) + '0';
    string_time[2] = ':';
    string_time[3] = (min >> 4) + '0';
    string_time[4] = (min & 0x0F) + '0';
    string_time[5] = '\0';
    
    return (string_time);
}

char get_day(void){
    char day;
    
    I2C_Master_Start();
    I2C_Master_Write(0xD0);
    I2C_Master_Write(0x03);         
    I2C_Master_RepeatedStart();     
    I2C_Master_Write(0xD1);
    day = I2C_Master_Read(0);            
    I2C_Master_Stop();
    
    return (day);
}

char* get_temp(void){
    char tempLSB, tempMSB;
    char temperature[6], decimal[3];
    
    I2C_Master_Start();
    I2C_Master_Write(0xD0);
    I2C_Master_Write(0x11);         
    I2C_Master_RepeatedStart();     
    I2C_Master_Write(0xD1);
    tempMSB = I2C_Master_Read(1);
    tempLSB = I2C_Master_Read(0);
    I2C_Master_Stop();
    
    tempMSB = tempMSB - 4;
    tempLSB = (tempLSB >> 6) * 25;
    sprintf(temperature, "%d", tempMSB);
    if (tempLSB == 0){
        sprintf(decimal, ".%d0", tempLSB);
    } else {
        sprintf(decimal, ".%d", tempLSB);
    }
    strcat(temperature, decimal);
    temperature[5] = '\0';
    
    return (temperature);
}

char get_hall (void){
    char key;
    
    I2C_Master_Start();
    I2C_Master_Write(0x11);     
    key = I2C_Master_Read(0);
    I2C_Master_Stop();
    
    return (key);
}