/** \file input_capture.c
 *  \brief library for input capturing functions
 */

/**
\addtogroup 
@{
*/

/* **************************************************************************																					
 *                                OpenPicus                 www.openpicus.com
 *                                                            italian concept
 * 
 *            openSource wireless Platform for sensors and Internet of Things	
 * **************************************************************************
 *  FileName:        input_capture.c
 *  Dependencies:    Microchip configs files
 *  Module:          FlyPort WI-FI
 *  Compiler:        Microchip C30 v3.12 or higher
 *
 *  Author               Rev.    Date              Comment
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Davide Vicca   1.0     11/17/2012		   First release  (core team)
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  Software License Agreement
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  This is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License (version 2) as published by 
 *  the Free Software Foundation AND MODIFIED BY OpenPicus team.
 *  
 *  ***NOTE*** The exception to the GPL is included to allow you to distribute
 *  a combined work that includes OpenPicus code without being obliged to 
 *  provide the source code for proprietary components outside of the OpenPicus
 *  code. 
 *  OpenPicus software is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details. 
 * 
 * 
 * Warranty
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * WE ARE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 **************************************************************************/
/// @cond debug

#include "input_capture.h"
#include "taskFlyport.h"



#define IC1				(51)
#define IC2				(52)
#define IC3				(53)
#define IC4				(54)
#define IC5				(55)
#define IC6				(56)


#define TIMEOUT			(65500)

unsigned char icflag;//input capture flag 
unsigned long icvalue;//ICXBUF value


extern int *TRISs[];	
extern int IOPos[];

/*********************************************************/
/*Remappable inputs registers for Input Capturing function
IC1-IC2-IC3-IC4-IC5-IC6	capture ****/						
int *RPICRs[]	= {
	(int*) 0x068E, (int*) 0x068E,
	(int*) 0x0690, (int*) 0x0690,
	(int*) 0x0692,(int*) 0x0692
	};					
	

/*********************************************************/
/*ICXCON1 registers for the first six IC modules*****/				
int *ICCON1Rs [] = {
	(int*) 0x0140, (int*) 0x0148, (int*) 0x0150,
	(int*) 0x0158,(int*) 0x0160,(int*) 0x0168
	};

/*********************************************************/
/*ICXCON2 registers for the first six IC modules*****/				
int *ICCON2Rs [] = {
	(int*) 0x0142, (int*) 0x014A, (int*) 0x0152,
	(int*) 0x015A,(int*) 0x0162,(int*) 0x016A
	};


#if ( (defined FLYPORT_B) || (defined FLYPORT_G) )
//Flyport pins for Input Capturing function  for WIFI module
const BYTE ICPos[] = {
	0,  10,  0, 17, 16,  30, 45, 2, 
	4, 3,  12,  11, 24, 23, 22,
	8,6,9,7,0,14,29
	};

#elif ( defined (FLYPORT_ETH) || defined(FLYPORT_LITE))
//Flyport pins for Input Capturing function  for ETH & LITE module
const BYTE ICPos[] = {
	0,  10,  0, 17, 16,  30, 45, 2, 
	4, 3,  12,  11, 24, 23, 22,
	8,6,9,7,0,14,29
	};
	
#elif defined (FLYPORT_GPRS)
//Flyport pins for Input Capturing function  for GPRS module
#error ICPOS REGISTERS NOT DEFINED FOR GPRS
const BYTE ICPos[] = {
	0,  10,  0, 17, 16,  30, 45, 2, 
	4, 3,  12,  11, 24, 23, 22,
	8,6,9,7,0,14,29
	};

#endif
/// @endcond



/**
\defgroup Input Capture
@{

*/

/**
 * ICXInterrupt - Save value of the dedicated timer stored in ICXBUF.
 * \param None
 * \return None
 */
/***********ISR's for input capture modules *********/
/****************************************************/


/***********ISR for input capture module 1 *********/
void __attribute__((interrupt, no_auto_psv)) _IC1Interrupt(void)
{
	unsigned long timer_32bit;
	icvalue = IC1BUF;
 	timer_32bit = IC2BUF;
	timer_32bit = timer_32bit <<16;
	icvalue = icvalue |timer_32bit;
	IFS0bits.IC1IF = 0;
	icflag = 1;
}

/***********ISR for input capture module 2 *********/
void __attribute__((interrupt, no_auto_psv)) _IC2Interrupt(void)
{
	icvalue = IC2BUF;
	IFS0bits.IC2IF = 0;
	icflag = 1;
}

/***********ISR for input capture module 3 *********/
void __attribute__((interrupt, no_auto_psv)) _IC3Interrupt(void)
{
	unsigned long timer_32bit;
	icvalue = IC3BUF;
 	timer_32bit = IC4BUF;
	timer_32bit = timer_32bit <<16;
	IFS2bits.IC3IF = 0;
	icflag = 1;
}

/***********ISR for input capture module 4 *********/
void __attribute__((interrupt, no_auto_psv)) _IC4Interrupt(void)
{
	icvalue = IC4BUF;
	IFS2bits.IC4IF = 0;
	icflag = 1;
}

/***********ISR for input capture module 5 *********/
void __attribute__((interrupt, no_auto_psv)) _IC5Interrupt(void)
{
	unsigned long timer_32bit;
	icvalue = IC5BUF;
 	timer_32bit = IC6BUF;
	timer_32bit = timer_32bit <<16;
	IFS2bits.IC5IF = 0;
	icflag = 1;
}

/***********ISR for input capture module 6 *********/
void __attribute__((interrupt, no_auto_psv)) _IC6Interrupt(void)
{
	icvalue = IC6BUF;
	IFS2bits.IC6IF = 0;
	icflag = 1;
}


/**
\defgroup Input Capture
@{

*/

/**
 * void IOInit_(int io, int putval) - Initializes the specified pin for Input Capture operations.
 * \param io - specifies the pin.
 * \param putval - specifies which IC module the pin must be initialized to. The valid parameters are the following:
 <UL>
	<LI><B>IC1</B> Input Capture 1.</LI>
	<LI><B>IC2</B> Input Capture 2.</LI>
	<LI><B>IC3</B> Input Capture 3.</LI>
	<LI><B>IC4</B> Input Capture 4.</LI>
	<LI><B>IC5</B> Input Capture 5.</LI>
	<LI><B>IC6</B> Input Capture 6.</LI>
</UL>
 * \return None
 */ 
void IOInit_(int io, int putval) 
{	
	io--;
	if(putval > 50)
	{
		putval = putval -51;
		if (ICPos[io] != 0)
		{	
			__builtin_write_OSCCONL(OSCCON & 0xBF);							// Unlock registers
			if(!(putval %2))
				*RPICRs[putval] = (*RPICRs[putval] & 0x3F00) | (ICPos[io]);//low byte register address			
			else
				*RPICRs[putval] = (*RPICRs[putval] & 0x003F) | (ICPos[io]<<8);//high byte register address			
			__builtin_write_OSCCONL(OSCCON | 0x40);			
		}
	}
}


/**
 *void ICInit(BYTE ic_module,BYTE active_edge,BYTE doublemodule)- Initializes the input capture module.
 * \param ic_module - specifies the ic_module.
 * \param  active_edge - specifies which edge the input is sensible to. The valid parameters are the following:
 <UL>
	<LI><B>0 </B> falling edge.</LI>
	<LI><B>1 </B> rising edge</LI>	
	<LI><B>2 </B> both edge.</LI>
 * \param  doublemodule - specifies if the module selected is provided with a 32 bit timer.
 <B>NOTE:</B> Only the odd module can have a 32 timer and once a module is used with a 32 bit timer the successive module to even one, can't be used for further operations.
	The valid parameters are the following:
	<LI><B>0 </B> 16 bit timer</LI>	
	<LI><B>1 </B> 32 bit timer.</LI>
	</UL>
 * \return None
 */ 
 void ICInit(BYTE ic_module,BYTE active_edge,BYTE doublemodule)
{
	ic_module--;
	unsigned int data;
	if(active_edge == 0)//falling edge 
		data = 0x1C02;
	else if(active_edge == 1)//rising edge
		data = 0x1C03;
	else
		data = 0x1C01;
	*ICCON1Rs[ic_module] = data;
	*ICCON2Rs[ic_module] = 0x0000;
	if(doublemodule)
	{
		/***** 32 bit timer enabling****/ 
		/***** ICXCON2 odd module setting****/ 
		*ICCON2Rs[ic_module] = 0x0100;
		ic_module++;
		/***** ICXCON1/2 even module setting****/ 
		*ICCON1Rs[ic_module] = 0x0000;//clear ICXCON1 even module register 
		*ICCON1Rs[ic_module] = data;//set ICXCON1 even module register 
		*ICCON2Rs[ic_module] = 0x0100;//set ICXCON2 even module register 
	}
}




/**
 * int ICOn(BYTE ic_module,int time,unsigned long *data) - Switch on the input capture module.
 * \param BYTE ic_module - specifies the ic_module.
 * \param int time - specifies the timeout event.
 * \param unsigned long *data - specifies the variable where the ICXBUF value is stored on.
* \return the status of operation
 	<LI><B>0 </B> the operation was successful</LI>	
 	<LI><B>-1 </B> the operation was unsuccessful (a timeout event occurred)</LI>	
	</UL>
 * \return None
 */ 
int ICOn(BYTE ic_module,int time,unsigned long *data)
{
	unsigned long timeout = ((unsigned long)time) * 500;//timeout
	*data = 0; //clear the return variable
	icflag = 0;//clear the ic interrupt flag
	ic_module--;
	vTaskSuspendAll();//lock the scheduler
	switch(ic_module)
	{
		case 0:
			IFS0bits.IC1IF = 0; // Clear the IC1 interrupt status flag
			IEC0bits.IC1IE = 1; // Enable IC1 interrupts
			break;
		case 1:
			IFS0bits.IC2IF = 0; // Clear the IC2 interrupt status flag
			IEC0bits.IC2IE = 1; // Enable IC2 interrupts
			break;
		case 2:
			IFS2bits.IC3IF = 0; // Clear the IC3 interrupt status flag
			IEC2bits.IC3IE = 1; // Enable IC3 interrupts
			break;
		case 3:
			IFS2bits.IC4IF = 0; // Clear the IC4 interrupt status flag
			IEC2bits.IC4IE = 1; // Enable IC4 interrupts
			break;
		case 4:
			IFS2bits.IC5IF = 0; // Clear the IC5 interrupt status flag
			IEC2bits.IC5IE = 1; // Enable IC5 interrupts
			break;
		case 5:
			IFS2bits.IC6IF = 0; // Clear the IC6 interrupt status flag
			IEC2bits.IC6IE = 1; // Enable IC6 interrupts
			break;
		default:
			return -1;
	}		
	while(!icflag && timeout)
		timeout--;
	xTaskResumeAll();//unlock the scheduler
	if(!timeout)
		return -1;//a timeout event was occurred
	*data = icvalue;
	return 0;
}



/**
 * void ICOff(BYTE ic_module) - Switch off the input capture module.
 * \param ic_module - specifies the ic_module.
 * \return None
 */ 
void ICOff(BYTE ic_module)
{
	ic_module--;
	/***Clear ICXCON1/2 ic_module registers****/
	*ICCON1Rs[ic_module] = 0x0000;
	*ICCON2Rs[ic_module] = 0x0000;
	/***Disable ic_module interrupt****/
	switch(ic_module)
	{
		case 0:
			IEC0bits.IC1IE =0; // Disable IC1 interrupts
			break;
		case 1:
			IEC0bits.IC2IE =0; // Disable IC2 interrupts
			break;
		case 2:
			IEC2bits.IC3IE =0; // Disable IC3 interrupts
			break;
		case 3:
			IEC2bits.IC4IE =0; // Disable IC4 interrupts
			break;
		case 4:
			IEC2bits.IC5IE =0; // Disable IC5 interrupts
			break;
		case 5:
			IEC2bits.IC6IE =0; // Disable IC6 interrupts
			break;
	}
}


/**
 *  int OneWireDecode(BYTE ic_module,unsigned int bitone_lenght,BYTE bit_number,unsigned long *data) - Decode a non-standard onewire protocol
 * \param BYTE ic_module - specifies the ic_module.
 * \param unsigned int bitone_lenght - specifies the one bit pulse lenght.
 * \param unsigned long *data - specifies the variable where the ICXBUF value is stored on.
* \return the status of operation
 	<LI><B>0 </B> the operation was successful</LI>	
 	<LI><B>-1 </B> the operation was unsuccessful (a timeout event occurred)</LI>	
	</UL>
 */ 
int OneWireDecode(BYTE ic_module,unsigned int bitone_lenght,BYTE bit_number,unsigned long *data)
{
	vTaskSuspendAll();//lock the scheduler
	*data = 0; //clear the return variable
	unsigned long d = 0;
	BYTE number = bit_number;
	int one_lenght = (bitone_lenght << 4);
	unsigned int timeout;
	ic_module--;
	switch(ic_module)
	{
		
		case 0:
			IFS0bits.IC1IF = 0; // Clear the IC1 interrupt status flag
			IEC0bits.IC1IE = 1; // Enable IC1 interrupts
			break;
		case 1:
			IFS0bits.IC2IF = 0; // Clear the IC2 interrupt status flag
			IEC0bits.IC2IE = 1; // Enable IC2 interrupts
			break;
		case 2:
			IFS2bits.IC3IF = 0; // Clear the IC3 interrupt status flag
			IEC2bits.IC3IE = 1; // Enable IC3 interrupts
			break;
		case 3:
			IFS2bits.IC4IF = 0; // Clear the IC4 interrupt status flag
			IEC2bits.IC4IE = 1; // Enable IC4 interrupts
			break;
		case 4:
			IFS2bits.IC5IF = 0; // Clear the IC5 interrupt status flag
			IEC2bits.IC5IE = 1; // Enable IC5 interrupts
			break;
		case 5:
			IFS2bits.IC6IF = 0; // Clear the IC6 interrupt status flag
			IEC2bits.IC6IE = 1; // Enable IC6 interrupts
			break;
		default:
			return -1;
	}
	*ICCON2Rs[ic_module]= 0x0000;//Reset the Input capture module 
	while(number)
	{
		icflag = 0;//clear the ic interrupt flag
		*ICCON1Rs[ic_module]= 0x0000;//Reset the Input capture module 
		*ICCON1Rs[ic_module] = 0x1C01;//setting the ic module for a new acquisition
		timeout = TIMEOUT;//Set the timeout time
		//waiting for the active edge capture occurrence
		while((!icflag) && timeout)
			timeout--;
		d = icvalue;
		icflag = 0;
		while((!icflag) && timeout)
			timeout--;
		d = icvalue - d;//pulse lenght
		number--;
		if(!timeout)
		{		
			break;//a timeout occurred
		}
		if(d > one_lenght)//decoding the capture pulse
			*data = *data |((long)1 <<(number));
	}	
	xTaskResumeAll();//unlock the scheduler
	if(!timeout)
		return -1;
	return 0;

	
}


