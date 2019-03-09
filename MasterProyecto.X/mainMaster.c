/*
 * File:   mainMaster.c
 * Author: Alejandro Recancoj y Julio Shin
 * Main del proyecto 1. Recolecta informaci�n de la red I2C y controla los actuadores del proyecto.
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
#include <stdlib.h>
#include <string.h>
#include "I2C.h"
#include "LCD4bits.h"
#include "Oscilador.h"
#include "UART.h"

char time[6] = {0}, temp[6] = {0}, *day2;
char door, trip, PIR, IR, state, day1, i, j, change, change1, change2, change3, alarm, bank, cont, val, hour, min;
char newhour, newmin;
int k;

void setup (void);
void write_RTC(char hour, char minutes, char day);
void get_time(char *time_string);
char get_day(void);
void get_temp(char *temp_string);
char get_hall(void);
char get_tripwire (void);
char get_PIR(void);
char get_IR(void);
void open_door(void);
void close_door(void);

void __interrupt() manual_pwm(void){
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
    state = 0;
    Lcd_Clear();
    PORTAbits.RA7 = 0;
    while (1) {
        //Obtener la informaci�n de los esclavos de la red I2C
        
        get_temp(temp);
        door = get_hall();
        trip = get_tripwire();
        PIR = get_PIR();
        IR = get_IR();
        get_time(time);
        day1 = get_day();
        
        if(PORTDbits.RD7 == 1){         // abrir puerta
            if (bank == 1){
                if (change1 == 0){
                    bank = 0;
                    alarm = 0;
                    open_door();
                    j = 0;
                    while(j < 50){
                        j++;
                    }
                    
                    change1 = 1;
                }
            } else if (bank == 0){
                if (change1 == 0){
                    bank = 1;
                    //alarm = 0;
                    close_door();
                    j = 0;
                    while(j < 50){
                        j++;
                    }
                    
                    change1 = 1;
                }
            }
        } else if(PORTDbits.RD7 == 0){
            change1 = 0;
        }
        
        /*else if(PORTDbits.RD1 == 1){         // cerrar puerta
                if (bank == 0){
                if (change2 == 0){
                bank = 1;
                alarm = 0;
                close_door();
                
                while(j < 50){
                    j++;
                }
                j = 0;
                change2 = 1;
            }
            }
        } else if (PORTDbits.RD1 == 0) {
            change2 =0;
        }*/
        
        alarm = trip | PIR | IR;
        
        /*if (trip | PIR | IR){
            alarm = 1;
            PORTAbits.RA7 = 1;
        }*/
        //agregar reset de alarma
        
        
        /*if (alarm == 1){
            if (change2 == 0){
                bank = 0;
                close_door();
                change2 = 1;
            }      
        } else {
            change2 = 0;
        }*/
        
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
        
        RCSTAbits.SPEN = 0;
        SSPCONbits.SSPEN = 1;
        
        if (door == 1){
            val = 8;
        } else if (door == 0){
            val = 17;
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
        
        //Si la temperatura es mayor a 23.50�C activa el relay, encendiendo el ventilador
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
                if (state > 3){
                    state = 0;
                }
                j = 0;
                while(j < 50){
                    j++;
                }
                
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
            Lcd_Write_Char(223);    //Escribe el caracter de grados (�)
            Lcd_Write_Char('C');
            
        } else if (state == 1){
            //Muestra el estado de la b�veda y el tripwire
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
            Lcd_Write_String(time);
            
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String("DIA: ");
            Lcd_Write_String(day2);
            
            newhour = (time[0] - '0') * 10 + (time[1] - '0');
            newmin = (time[3] - '0') * 10 + (time[4] - '0');
            
            if (PORTDbits.RD4 == 1){
                newhour++;
                if (newhour > 23){
                    newhour = 0;
                }
                __delay_ms(100);  
            } else if (PORTDbits.RD3 == 1){
                newhour--;
                if (newhour == 255){
                    newhour = 23;
                }
                __delay_ms(100);   
            } else if (PORTDbits.RD2 == 1){
                newmin++;
                if (newmin > 59){
                    newmin = 0;
                }
                __delay_ms(100);    
            } else if (PORTDbits.RD1 == 1){
                newmin--;
                if (newmin == 255){
                    newmin = 59;
                }
                __delay_ms(100);   
            } else if (PORTDbits.RD0 == 1){
                day1++;
                if (day1 > 7){
                    day1 = 1;
                }
                __delay_ms(100);    
            }
            
            hour = ((newhour / 10) << 4) + (newhour % 10);
            min = ((newmin / 10) << 4) + (newmin % 10);
            write_RTC(hour, min, day1);
        }
    }  
}

void setup (void){
    initOscilador(7);           // Oscilador interno de 8 MHz
    ANSELH = 0;                 //Puerto B digital
    ANSEL = 0;                  //Puerto A digital
    TRISB = 0;                  //PORTB como output
    TRISA = 0;                  //PORTA como output
    TRISD = 0xFF;
    PORTB = 0;                  //Inicializar puertos
    PORTA = 0;
    TRISC = 0x01;               //RC0 como input
    PORTC = 0;
    i = 0;
    door = 0;
    trip = 0;
    IR = 0;
    PIR = 0;
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
    Lcd_Init();                 //Inicializar LCD
    UART_Init(9600);            //Inicializa la comunicaci�n UART
    I2C_Master_Init(100000);        // Inicializar Comuncaci�n I2C
}

void write_RTC (char hour, char minutes, char day) {
    //Cambia los segundos, minutos, hora y d�a del DS3231
    I2C_Master_Start();
    I2C_Master_Write(0xD0);
    I2C_Master_Write(0x01);
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
    //Obtiene d�a del DS3231. 1 es lunes, 2 martes, etc.
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
    //Obtiene el estado del l�ser
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

void open_door(void){
    
    I2C_Master_Start();
    I2C_Master_Write(0x20);     
    I2C_Master_Write(0xFF);
    I2C_Master_Stop();
    
}

void close_door(void){
    
    I2C_Master_Start();
    I2C_Master_Write(0x20);     
    I2C_Master_Write(0xF0);
    I2C_Master_Stop();
    
}