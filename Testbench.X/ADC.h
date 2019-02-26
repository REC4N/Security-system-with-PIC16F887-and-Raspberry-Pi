/* **********************************************************************
 * File: Library for ADC                                                *
 * Author: Edgar Alejandro Recancoj Pajarez								*
 * Comments: None                                                       *
 * Revision history: 1													*
 ************************************************************************/

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.


#endif	/* XC_HEADER_TEMPLATE_H */

void ADC_init(char ADCS, char CHS){
	char temp, ADCS_select, CHS_select, ADON_select;    // Variables that are used to put the bits in order to OR them and move it to the ADCON0 register.
	ADCS_select = ADCS << 6;                            // Configuration for A/D conversion clock select. 0x00 for Fosc/2, 0x01 for Fosc/8, 0x02 for Fosc/32 and 0x03 for Frc.
	CHS_select = CHS << 2;                              // Configuration for Analog Channel Select. 0x00 to 0x0D for AN0 to AN13 respectively. 0x0E for CVref and 0x0F for 0.6V fixed voltage reference.
	ADON_select = 0x01;                                 // Enables ADC
	temp = ADCS_select || CHS_select || ADON_select;    // Variables are ORd to make a single byte containing the config of the ADC
	ADCON1bits.ADFM = 0;                                // A/D conversion result format is selected as right justified.
	ADCON1bits.VCFG0 = 0;                               // Voltage reference 1 is selected to Vdd.
	ADCON1bits.VCFG1 = 0;                               // Voltage reference 2 is selected to Vss.
	ADCON0 = temp;                                      // Configuration byte is moved to the ADCON0 register.
}