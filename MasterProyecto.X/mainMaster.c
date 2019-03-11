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

char time[6] = {0}, temp[6] = {0}, *day2, password[5] = {1,5,1,0,3}, passwordu[5];
char door, trip, PIR, IR, state, day1, i, j, k, change, change1, change2, change3, change4, alarm, bank, cont, val, hour, min, seguridad;
char newhour, newmin;

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
    //Interrupción para realizar el PWM del servo. La interrupción se da cada 0.1ms
    if (INTCONbits.T0IF == 1){    
        cont++;         // Aumenta la variable cont
        if(cont < 200){     // Revisa que no se haya pasado del periodo
            if(cont < val){     // Si es menor al val mandado, la salida es high
                PORTAbits.RA1 = 1;
            }else{
                PORTAbits.RA1 = 0;  // Si se pasa de val, la salida es low
            }    
        } else {
            cont = 0;   // Si cont es mayor a 200, reinicia el PWM
        }
        TMR0 = 56;  // Se reinicia el TMR0
        INTCONbits.T0IF = 0;    // Se limpia la bandera           
    }
}

void main(void) {
    setup();        //Configurar el PIC e inicializar variables
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    // Mostrar Iniciando Sistema de Seguridad y desplazarlo para que se lea el mensaje completo 
    Lcd_Write_String("Iniciando Sistema de Seguridad");
    for (k = 0; k < 16; k++){
        Lcd_Shift_Left();
        __delay_ms(300);
    }
    for (k = 0; k < 16; k++){
        Lcd_Shift_Right();
        __delay_ms(300);
    }
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    // Mostrar Espera un momento por 4 segundos
    Lcd_Write_String("Espere");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String("un momento");
    __delay_ms(4000);
    Lcd_Clear();
    
    while (1) {
/* ******************************************************************
 * Bloque para las funciones que obtienen los datos de los esclavos *
 ********************************************************************/
        
        get_temp(temp);         // Obtiene la temperatura del reloj de tiempo real
        door = get_hall();      // Obtiene el estado de la bóveda de la puerta
        trip = get_tripwire();  // Obtiene el estado del láser
        PIR = get_PIR();        // Obtiene el estado del sensor de movimiento
        IR = get_IR();          // Obtiene el estado del sensor infrarrojo
        get_time(time);         // Obtiene la hora del reloj de tiempo real
        day1 = get_day();       // Obtiene el día de la semana del reloj
        bank = get_tripwire();  // Obtiene el estado del banco. Se utiliza la misma función debido a que es el mismo PIC
        
/***********************************************************************
 * Condicional para abrir y cerrar el banco con el botón en el pin RD7 *
 ***********************************************************************/        
        
        if(PORTDbits.RD7 == 1){         
            // Abrir la puerta del banco si el banco está cerrado
            if (bank == 2){
                // Se realiza una sola vez por la variable change1
                if (change1 == 0){
                    open_door();
                    j = 0;
                    while(j < 50){      //Función de antirrebote
                        j++;
                    }
                    change1 = 1;        //Cambia la variable para que solo ocurra una vez
                }
            } else if (bank == 3){
                // Cerrar el banco si está abierto
                if (change1 == 0){
                    close_door();
                    j = 0;
                    while(j < 50){
                        j++;        // Antirrebote
                    }
                    change1 = 1;    // Cambio para que ocurra una sola vez
                }
            }
        } else if(PORTDbits.RD7 == 0){
            change1 = 0;        // Al soltar el botón, se limpia la variable de cambio
        }

/**********************************************************
 * Bloque que controla la alarma y la seguridad del banco *
 **********************************************************/        
        
        // Si la alarma está desactivada y la seguridad está encendida, la alarma se activa si algún sensor cambia a high
        if (alarm == 0 & seguridad == 1){
            alarm = trip | PIR | IR;
        }
        
        // Ocurre si la alarma se activa
        if (alarm == 1){
            if (change2 == 0){
                change2 = 1;    // Se cambia una variable para que ocurra una sola vez
                close_door();   // Se cierra la puerta del banco
                PORTAbits.RA6 = 1;  //Se enciende el buzzer en el pin RA6
            }      
        } else {
            PORTAbits.RA6 = 0;  // Si se desactiva la alarma, se apaga el buzzer
            change2 = 0;        // Se reinicia la variable para que ocurra la alarma
        }
        
        // El botón en RD6 solo funciona si la seguridad está activada. Es un botón de emergencias y para apagar la alarma
        if (seguridad == 1){
            // Al presionar el botón, la alarma se activa o se apaga dependiendo del estado en el que está
            if (PORTDbits.RD6 == 1){
                // Ocurre una sola vez si se deja presionado el botón
                if (change3 == 0){
                    if (alarm == 0){
                        alarm = 1;      // Enciende la alarma
                    }else {
                        alarm = 0;      // Apaga la alrma
                    }
                    change3 = 1;        // Cambia la variable
                }
            }
            else{
                change3 = 0;        // Si se suelta el botón, se reinicia la variable
            }
        }
/*************************************
 * Bloque de envío de datos por UART *
 *************************************/
        // Se activa la comunicación UART y se desactiva la comunicación I2C
        SSPCONbits.SSPEN = 0;
        RCSTAbits.SPEN = 1;
        
        // Si el buffer de transmisión está vacío se comienzan a enviar datos
        if (UART_TX_Empty()){
            // Se mandan datos uno por uno con un delay de 1ms para que el PIC traductor pueda procesar la información
            // El orden de envío de datos es temperatura, puerta de la bóveda, alarma, banco y seguridad
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
            UART_Write(door);
            __delay_ms(1);
            UART_Write(alarm);
            __delay_ms(1);
            UART_Write(bank);
            __delay_ms(1);
            UART_Write(seguridad);
            __delay_ms(1);
            UART_Write('A');        // Se envía una A para indicar el final de transmisión
            __delay_ms(1);
        }
        
        // Se desactiva UART y se activa I2C nuevamente
        RCSTAbits.SPEN = 0;
        SSPCONbits.SSPEN = 1;
        
/************************************************
 * Condicionales para acciones varias del banco *
 ************************************************/
        
        // Controla el PWM de la puerta del banco
        if (door == 1){
            val = 8;    // Si la puerta está abierta, manda un 8 a la interrupción del PWM
        } else if (door == 0){
            val = 16;   // Si la puerta está cerrada, manda un 16.
        }
        
        // Obtiene el día por medio de la codificación del reloj de tiempo real
        // 1 es lunes, 2 es martes, etc.
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
            PORTAbits.RA0 = 1;      // Enciende el ventilador
        } else {
            PORTAbits.RA0 = 0;      // Apaga el ventilador
        }
        
        // Condicional para cerrar el banco automáticamente
        // Si el reloj da la 22:00 y no es fin de semana, cierra el banco
        if ((strcmp(time, "22:00") == 0) & ((day1 != 6) | (day1 != 7))){
            close_door();
        // Si la hora es 21:00 y es sábado o domingo, se cierra el banco
        } else if ((strcmp(time, "21:00") == 0) & ((day1 == 6) | (day1 == 7))){
            close_door();
        }
        
/************************************************************
 * Bloque que se encarga de los estados principales del PIC *
 ************************************************************/
        
        // Cambia el estado para mostrar los diferentes sensores e información en la LCD
        if(PORTCbits.RC0 == 1){
            if (change == 0){
                // Cambia solamente una vez al presionar el botón
                state++;    // Incrementa el estado
                if (state == 4){
                    k = 0;      // Si el estado es 4, se reinicia la variable k
                }
                Lcd_Clear();    // Al cambiar el estado, se limpia la LCD
                if (state > 5){     // Si se llega a 5, el estado regresa a 0
                    state = 0;
                }
                j = 0;
                while(j < 50){
                    j++;            //Antirrebote
                }
                change = 1;         //Se cambia la variable para que ocurra solo una vez el cambio de estado
            }
        } else {
            change = 0;         // Se reinicia el la varible de cambio
        }
        
        if (state == 0){
            //Muestra la hora, día y la temperatura del banco
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
                Lcd_Write_String("BOVEDA ABIERTA ");
            } else {
                Lcd_Write_String("BOVEDA CERRADA ");
            }

            Lcd_Set_Cursor(2,1);
            if (trip == 1){
                Lcd_Write_String("TRIP ACTIVADO   ");
            } else {
                Lcd_Write_String("TRIP DESACTIVADO");
            }
            
        } else if (state == 2){
            //Muestra el estado del sensor de movimiento y el infrarrojo
            Lcd_Set_Cursor(1,1);
            if (PIR == 1){
                Lcd_Write_String("PIR ACTIVADO   ");
            } else {
                Lcd_Write_String("PIR DESACTIVADO");
            }
            
            Lcd_Set_Cursor(2,1);
            if (IR == 1){
                Lcd_Write_String("IR ACTIVADO   ");
            } else {
                Lcd_Write_String("IR DESACTIVADO");
            }
        } else if (state == 3){
            // Permite cambiar la hora del RTC
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("HORA: ");
            Lcd_Write_String(time);
            
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String("DIA: ");
            Lcd_Write_String(day2);
            
            // Se convierten los strings de hora y minutos a numeros
            newhour = (time[0] - '0') * 10 + (time[1] - '0');
            newmin = (time[3] - '0') * 10 + (time[4] - '0');
            
            if (PORTDbits.RD4 == 1){
                // Se incrementa la hora al presionar RD4
                newhour++;
                if (newhour > 23){
                    newhour = 0;        // Si se llega a 23, regresa a 0
                }
                __delay_ms(100);        // Delay para seguir incrementando la hora al mantener presionado
            } else if (PORTDbits.RD3 == 1){
                // Disminuye la hora con RD3
                newhour--;
                if (newhour == 255){
                    newhour = 23;       // Al bajar de 0, se regresa a 23
                }
                __delay_ms(100);        // Delay para cambio continuo
            } else if (PORTDbits.RD2 == 1){
                // RD2 aumenta los minutos
                newmin++;
                if (newmin > 59){
                    newmin = 0;     // Si se pasa de 59, baja a 0
                }
                __delay_ms(100);    // Delay para continuar aumentando
            } else if (PORTDbits.RD1 == 1){
                // RD1 disminuye los minutos
                newmin--;           
                if (newmin == 255){
                    newmin = 59;    //Al pasarse de 0 cambia a 59 
                }
                __delay_ms(100);   // Delay para seguir disminuyendo
            } else if (PORTDbits.RD0 == 1){
                day1++;             // Se aumenta el día con RD0
                if (day1 > 7){
                    day1 = 1;       // Se se pasa de 7 regresa a 1
                }
                __delay_ms(100);    // Delay para cambiar el día de manera continua
            }
            
            // Se convierte de números a la codificación BCD
            hour = ((newhour / 10) << 4) + (newhour % 10);
            min = ((newmin / 10) << 4) + (newmin % 10);
            
            // Se sobre escriben los valores del RTC
            write_RTC(hour, min, day1);
        } else if (state == 4){
            // En el estado 4 se permite ingresar una contraseña para poder desactivar o activar la seguridad
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("PASSWORD:       ");
            // Se posiciona el cursor en el espacio k +1
            Lcd_Set_Cursor(2,k + 1);
            
            // Debido a que la contraseña es de 5 numeros (15103), solo permite ingresar 5 valores
            if (k < 5) {
                if (PORTDbits.RD0 == 1){
                    // Ocurre solo 1 vez al presionar el botón
                    if (change4 == 0){
                        passwordu[k] = 0;   // Posición k de la lista es 0
                        k++;                // Aumenta el índice de la lista
                        change4 = 1;        // Cambia la variable
                        Lcd_Write_Char('*');    //Se escribe un asterisco
                    }    
                } else if (PORTDbits.RD1 == 1){
                    if (change4 == 0){
                        passwordu[k] = 1;   //Posición k es igual a 1
                        k++;
                        change4 = 1;
                        Lcd_Write_Char('*');
                    }
                } else if (PORTDbits.RD2 == 1){
                    if (change4 == 0){
                        passwordu[k] = 2;   //Posición k es igual a 2
                        k++;
                        change4 = 1;
                        Lcd_Write_Char('*');
                    }
                } else if (PORTDbits.RD3 == 1){
                    if (change4 == 0){
                        passwordu[k] = 3;   //Posición k es igual a 3
                        k++;
                        change4 = 1;
                        Lcd_Write_Char('*');
                    }
                } else if (PORTDbits.RD4 == 1){
                    if (change4 == 0){
                        passwordu[k] = 4;   //Posición k es igual a 4
                        k++;
                        change4 = 1;
                        Lcd_Write_Char('*');
                    }
                } else if (PORTDbits.RD5 == 1){
                    if (change4 == 0){
                        passwordu[k] = 5;   //Posición k es igual a 5
                        k++;
                        change4 = 1;
                        Lcd_Write_Char('*');
                    }
                } else if (PORTD == 0){
                        change4 = 0;        // Si se sueltan todos los botones, se reinicia la variable para poder ingresar otro valor
                }
            }
            
            // Al presionar RE0, se comprueba la contraseña
            if (PORTEbits.RE0 == 1){
                    if (passwordu[0] == password[0] & passwordu[1] == password[1] & passwordu[2] == password[2] & passwordu[3] == password[3] & passwordu[4] == password[4]){
                        // Si la contraseña es correcta, se puuede activar la seguridad si está apgada o se puede apagar si está encendida
                        if (seguridad == 0){
                            seguridad = 1;      // Activa la seguridad
                            k = 0;              // Reinicia la posición de la lista
                            Lcd_Clear();
                            Lcd_Set_Cursor(1,1);
                            Lcd_Write_String("SEGURIDAD ON");   // Escribe seguridad on en la LCD
                            __delay_ms(2000);
                        } else {
                            seguridad = 0;      // Desactiva la seguridad
                            k = 0;              // Reinicia la posición de la lista
                            alarm = 0;
                            Lcd_Clear();
                            Lcd_Set_Cursor(1,1);
                            Lcd_Write_String("SEGURIDAD OFF");  // Escribe seguridad off en la LCD
                            __delay_ms(2000);
                        }
                    } else {
                        // Si la contraseña no coincide, no ocurre nada con la seguridad
                        Lcd_Clear();
                        k = 0;      // Se reinicia la posición k
                        Lcd_Set_Cursor(1,1);
                        Lcd_Write_String("INCORRECTO");         // Se escribe incorrecto en la pantalla
                        __delay_ms(2000);
                    }
            } else if (PORTEbits.RE1 == 1){
                // Si se presiona RE1, se reinicia la contraseña y se limpia la contraseña ingresada
                Lcd_Set_Cursor(2,1);
                Lcd_Write_String("            ");
                k = 0;
            }
        } else if(state == 5){
            // Muestra si el banco está abierto o cerrado y si la seguridad está activada o no
            Lcd_Set_Cursor(1,1);
            if (bank == 3){
                Lcd_Write_String("BANCO ABIERTO");
            } else {
                Lcd_Write_String("BANCO CERRADO");
            }
            
            Lcd_Set_Cursor(2,1);
            if (seguridad == 1){
                Lcd_Write_String("SEGURIDAD ON");
            } else {
                Lcd_Write_String("SEGURIDAD OFF");
            }
        }
    }
}  

/****************************
 * Funciones para el master *
 ****************************/

void setup (void){
    initOscilador(7);           // Oscilador interno de 8 MHz
    ANSELH = 0;                 //Puerto B digital
    ANSEL = 0;                  //Puerto A digital
    TRISB = 0;                  //PORTB como output
    TRISA = 0;                  //PORTA como output
    TRISD = 0xFF;               //PORTD como input
    TRISE = 0xFF;               // PORTE como entrada
    TRISC = 0x01;               //RC0 como input
    PORTB = 0;                  //Inicializar puertos
    PORTA = 0;
    PORTC = 0;
    PORTE  = 0;                 
   
    state = 0;                  // Inicia en estado 0
    seguridad = 1;              // Seguridad activada
    i = 0;
    door = 0;                   //Inicialización de variables de la red
    trip = 0;
    alarm = 0;
    change2 = 0;
    change3 = 0;
    IR = 0;
    PIR = 0;
    
    OPTION_REGbits.T0CS = 0;    // Se habilita el TMR0
    OPTION_REGbits.T0SE = 0;  
    OPTION_REGbits.PSA = 1;     // Prescaler para el TMR0
    OPTION_REGbits.PS2 = 0;     // Prescaler de 1
    OPTION_REGbits.PS1 = 0;
    OPTION_REGbits.PS0 = 0;
    TMR0 = 56;                  // Valor de inicio de 56 para un periodo de 0.1ms
    INTCONbits.T0IF = 0;        // Se activan interrupciones
    INTCONbits.T0IE = 1;
    INTCONbits.GIE = 1;
    Lcd_Init();                 //Inicializar LCD
    UART_Init(9600);            //Inicializa la comunicación UART
    I2C_Master_Init(100000);        // Inicializar Comuncación I2C
}

void write_RTC (char hour, char minutes, char day) {
    //Cambia los segundos, minutos, hora y día del DS3231
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
    
    // Convierte de BCD a strings para mostrarlos en el display
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
    
    // Convierte el decimal de la temperatura a un string para poder ser mostrado en la LCD
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

void open_door(void){
    // Envía un 0xFF para abrir la puerta del banco
    
    I2C_Master_Start();
    I2C_Master_Write(0x20);     
    I2C_Master_Write(0xFF);
    I2C_Master_Stop();
    
}

void close_door(void){
    // Envía un 0xF0 para cerrar la puerta del banco
    
    I2C_Master_Start();
    I2C_Master_Write(0x20);     
    I2C_Master_Write(0xF0);
    I2C_Master_Stop();
    
}