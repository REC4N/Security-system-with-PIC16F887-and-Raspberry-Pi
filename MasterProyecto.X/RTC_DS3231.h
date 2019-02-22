/* 
 * File:   RTC_DS3231.h
 * Author: Julio Shin
 * Librería que permite el uso de un RTC DS3231 con comunicación I2C.
 * Created on February 19, 2019, 11:56 AM
 */
/*
#include <stdio.h>
#include <string.h>
#include "I2C.h"


void write_RTC (char hour, char minutes, char day) {
    I2C_Master_Start();
    I2C_Master_Write(0xD0);
    I2C_Master_Write(0x01);
    I2C_Master_Write(minutes);
    I2C_Master_Write(hour);
    I2C_Master_Write(day);
    I2C_Master_Stop();
}

char* get_time (char string_time[5]){
    char hour, min, temp;
    char string_hour[2], string_min[2];
    
    I2C_Master_Start();
    I2C_Master_Write(0xD0);
    I2C_Master_Write(0x01);         
    I2C_Master_RepeatedStart();     
    I2C_Master_Write(0xD1);
    min = I2C_Master_Read(1);      
    hour = I2C_Master_Read(0);      
    I2C_Master_Stop();
    
    temp = min >> 4;
    min = min & 0x0F;
    min = min + temp * 10;
    
    temp = hour >> 4;
    hour = hour & 0x0F;
    hour = hour + temp * 10;
    
    sprintf(string_hour, "%d", hour);
    sprintf(string_min, ":%d", min);
    
    strcpy(string_time, string_hour);
    strcat(string_time, string_min);
    
    return string_time;
}
*/
