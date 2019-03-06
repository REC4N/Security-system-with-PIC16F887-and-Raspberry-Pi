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
#include "UART.h"

char time[6] = {0}, temp[6] = {0}, newtime[6], *day2, *newday2;
char door, trip, PIR, IR, state, day1, i, j, change, newday;

void setup (void);
void write_RTC(char sec, char hour, char minutes, char day);
void get_time(char *time_string);
char get_day(void);
void get_temp(char *temp_string);
char get_hall(void);
char get_tripwire (void);
char get_PIR(void);
char get_IR(void);

void main(void) {
    setup();
    state = 0;
    Lcd_Clear();
    //write_RTC(0x00, 0x21, 0x50, 0x07);
    while (1) {
        //Obtener la información de los esclavos de la red I2C
        RCSTAbits.SPEN = 0;
        SSPCONbits.SSPEN = 1;
        
        get_temp(temp);
        door = get_hall();
        trip = get_tripwire();
        PIR = get_PIR();
        IR = get_IR();
        get_time(time);
        day1 = get_day();
        
        SSPCONbits.SSPEN = 0;
        RCSTAbits.SPEN = 1;
        
        if (UART_TX_Empty()){
            UART_Write(temp[0]);
            __delay_ms(1);
            UART_Write(temp[1]);
            __delay_ms(1);
            UART_Write(temp[2]);
            __delay_ms(1);
            UART_Write(temp[3]);
            __delay_ms(1);
            UART_Write(temp[4]);
            __delay_ms(1);
            UART_Write(time[0]);
            __delay_ms(1);
            UART_Write(time[1]);
            __delay_ms(1);
            UART_Write(time[2]);
            __delay_ms(1);
            UART_Write(time[3]);
            __delay_ms(1);
            UART_Write(time[4]);
            __delay_ms(1);
            UART_Write(door);
            __delay_ms(1);
            UART_Write(trip);
            __delay_ms(1);
            UART_Write(PIR);
            __delay_ms(1);
            UART_Write(IR);
            __delay_ms(1);
            UART_Write('A');
        }
        
        switch(day1){
            case 1:
                day2 = "LUNES    ";
                break;
            case 2:
                day2 = "MARTES   ";
                break;
            case 3:
                day2 = "MIERCOLES";
                break;
            case 4:
                day2 = "JUEVES   ";
                break;
            case 5:
                day2 = "VIERNES  ";
                break;
            case 6:
                day2 = "SABADO   ";
                break;
            case 7:
                day2 = "DOMINGO  ";
                break;
            default:
                day2 = "         ";
                break;  
        }
        
        //Si la temperatura es mayor a 23.50°C activa el relay, encendiendo el ventilador
        if (strcmp(temp,"23.50") > 0){
            PORTAbits.RA0 = 1;
        } else {
            PORTAbits.RA0 = 0;
        }
        
        //Cambia el estado para mostrar los diferentes sensores en la LCD
        if(PORTCbits.RC0 == 1){
            if (change == 0){
                state++;
                Lcd_Clear();
                if (state == 3){
                    newday = get_day();
                    get_time(newtime);
                }
                if (state > 3){
                    state = 0;
                }
                while(j < 50){
                    j++;
                }
                j = 0;
                change = 1;
            }
        } else {
            change =0;
        }
        
        
        if (state == 0){
            //Muestra la hora y la temperatura del banco
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String(time);
            
            Lcd_Set_Cursor(1,7);
            Lcd_Write_String(day2);
            
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String(temp);
            Lcd_Write_Char(223);    //Escribe el caracter de grados (°)
            Lcd_Write_Char('C');
            
        } else if (state == 1){
            //Muestra el estado de la bóveda y el tripwire
            Lcd_Set_Cursor(1,1);
            if (door == 1){
                Lcd_Write_String("Door OPEN  ");
            } else {
                Lcd_Write_String("Door CLOSED");
            }

            Lcd_Set_Cursor(2,1);
            if (trip == 1){
                Lcd_Write_String("Trip ON ");
            } else {
                Lcd_Write_String("Trip OFF");
            }
            
        } else if (state == 2){
            //Muestra el estado del sensor de movimiento y el infrarrojo
            Lcd_Set_Cursor(1,1);
            if (PIR == 1){
                Lcd_Write_String("PIR ON ");
            } else {
                Lcd_Write_String("PIR OFF");
            }
            
            Lcd_Set_Cursor(2,1);
            if (IR == 1){
                Lcd_Write_String("IR ON ");
            } else {
                Lcd_Write_String("IR OFF");
            }
        } else if (state == 3){
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("HORA: ");
            Lcd_Write_String(newtime);
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String("DIA: ");
            switch(newday){
                case 1:
                    newday2 = "LUNES    ";
                    break;
                case 2:
                    newday2 = "MARTES   ";
                    break;
                case 3:
                    newday2 = "MIERCOLES";
                    break;
                case 4:
                    newday2 = "JUEVES   ";
                    break;
                case 5:
                    newday2 = "VIERNES  ";
                    break;
                case 6:
                    newday2 = "SABADO   ";
                    break;
                case 7:
                    newday2 = "DOMINGO  ";
                    break;
                default:
                    newday2 = "         ";
                    break;  
            }
            Lcd_Write_String(newday2);
        }
    }  
}

void setup (void){
    initOscilador(7);           // Oscilador interno de 8 MHz
    ANSELH = 0;                 //Puerto B digital
    ANSEL = 0;                  //Puerto A digital
    TRISB = 0;                  //PORTB como output
    TRISA = 0;                  //PORTA como output
    TRISD = 15;
    PORTB = 0;                  //Inicializar puertos
    PORTA = 0;
    TRISC = 0x01;               //RC0 como input
    PORTC = 0;
    i = 0;
    door = 0;
    trip = 0;
    IR = 0;
    PIR = 0;
    Lcd_Init();                 //Inicializar LCD
    UART_Init(9600);            //Inicializa la comunicación UART
    I2C_Master_Init(100000);        // Inicializar Comuncación I2C
}

void write_RTC (char sec, char hour, char minutes, char day) {
    //Cambia los segundos, minutos, hora y día del DS3231
    I2C_Master_Start();
    I2C_Master_Write(0xD0);
    I2C_Master_Write(0x00);
    I2C_Master_Write(sec);
    I2C_Master_Write(minutes);
    I2C_Master_Write(hour);
    I2C_Master_Write(day);
    I2C_Master_Stop();
}

void get_time (char *time_string){
    //Obtiene la hora del RTC y la convierte en un string
    char hour, min;
    
    I2C_Master_Start();
    I2C_Master_Write(0xD0);
    I2C_Master_Write(0x01);         
    I2C_Master_RepeatedStart();     
    I2C_Master_Write(0xD1);
    min = I2C_Master_Read(1);      
    hour = I2C_Master_Read(0);      
    I2C_Master_Stop();
    
    time_string[0] = (hour >> 4) + '0';
    time_string[1] = (hour & 0x0F) + '0';
    time_string[2] = ':';
    time_string[3] = (min >> 4) + '0';
    time_string[4] = (min & 0x0F) + '0';
    time_string[5] = '\0';
}

char get_day(void){
    //Obtiene día del DS3231. 1 es lunes, 2 martes, etc.
    char d;
    
    I2C_Master_Start();
    I2C_Master_Write(0xD0);
    I2C_Master_Write(0x03);         
    I2C_Master_RepeatedStart();     
    I2C_Master_Write(0xD1);
    d = I2C_Master_Read(0);            
    I2C_Master_Stop();
    
    return d;
}

void get_temp(char *temp_string){
    //Obtiene la temperatura del RTC y la devuelve como string
    char tempLSB, tempMSB;
    char decimal[3];
    
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
    sprintf(temp_string, "%d", tempMSB);
    if (tempLSB == 0){
        sprintf(decimal, ".%d0", tempLSB);
    } else {
        sprintf(decimal, ".%d", tempLSB);
    }
    strcat(temp_string, decimal);
    temp_string[5] = '\0';
    
}

char get_hall (void){
    //Obtiene el estado de la puerta
    char key;
    
    I2C_Master_Start();
    I2C_Master_Write(0x11);     
    key = I2C_Master_Read(0);
    I2C_Master_Stop();
    
    return (key);
}

char get_tripwire (void){
    //Obtiene el estado del láser
    char trip;

    I2C_Master_Start();
    I2C_Master_Write(0x21);     
    trip = I2C_Master_Read(0);
    I2C_Master_Stop();
    
    return (trip);
}

char get_PIR (void){
    //Obtiene el estado del sensor de movimiento
    char PIR;

    I2C_Master_Start();
    I2C_Master_Write(0x41);     
    PIR = I2C_Master_Read(0);
    I2C_Master_Stop();
    
    return (PIR);
}

char get_IR (void){
    //Obtiene el estado del sensor IR
    char IR;

    I2C_Master_Start();
    I2C_Master_Write(0x31);     
    IR = I2C_Master_Read(0);
    I2C_Master_Stop();
    
    return (IR);
}