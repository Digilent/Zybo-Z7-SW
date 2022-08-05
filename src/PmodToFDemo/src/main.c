/********************************************************************************************/
/*                                                                           				*/
/* main.c -- PmodToF Demo Project                                          					*/
/*                                                                            				*/
/********************************************************************************************/
/* Author:  Ana-Maria-Eliza Balas                                                   		*/
/*		    ana-maria.balas@digilent.ro										  				*/
/*                                                                            				*/
/* Copyright 2019, Digilent Inc.                                              				*/
/*                                                                            				*/
/********************************************************************************************/
/* Module Description:                                                        				*/
/*                                                                            				*/
/* This file contains source code for running a demonstration of the Pmod ToF device	    */
/* when used with the Pmod ToF Hierarchical Block. 											*/
/* Zynq Pmod ToF Library User Guide for the demo can be found at:							*/
/* 																							*/
/* https://digilent.com/reference/pmod/pmodtof/zynqlibraryuserguide              			*/
/*                                                                            				*/
/* This demo  implements a command interpreter that listens over UART for individual		*/
/* Pmod ToF commands.																		*/
/* The following list enumerates the commands implemented in the PmodToFCMD   				*/
/* module:																					*/
/*                                                                            				*/
/*    ------------------------------------                                    				*/
/*	  | format: <command> (<input code>) | 													*/
/*    ------------------------------------           		                   				*/
/*    | ToFMeasure (m)					 |													*/
/*    | ToFStartCalib (c)				 |													*/
/*    | ToFSaveCalib (s)				 |													*/
/*    | ToFRestoreFactCalib (f)			 |													*/
/*    | ToFReadSerialNo (n)				 |													*/
/*	  ------------------------------------													*/
/*                                                                            				*/
/* The user will have to provide the code corresponding to each command  					*/
/* using a terminal connected to the USB port corresponding to the                          */
/* connected board, configured with 115200 Baud rate.                                       */
/*                                                                            				*/
/*                                                                            				*/
/********************************************************************************************/
/* Revision History:                                                          				*/
/*                                                                           				*/
/*    09/23/2019(anamariabalas):   Created                                    				*/
/*    09/23/2019(anamariabalas): Validated for Vivado 2019.1								*/
/*	  05/08/2022(raduvele): Modified the I/O format										    */
/*                                                                            				*/
/********************************************************************************************/

#include "xparameters.h"
#include <stdio.h>
#include "PmodToF/PmodToF.h"
#include "errors.h"
#include "PmodToFCMD.h"
#include "UART.h"

/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */

void main_PrintMainMenu();

/* ************************************************************************** */

int main(void)
{
	PmodToF_Initialize();
	UART_Init(115200);
	ERRORS_Init("OK", "ERROR");

	char inputOption = 0;

    while(1)
    {
    	main_PrintMainMenu();
    	inputOption = getchar();
    	PmodToFCMD_CheckForCommand(inputOption);

    	if(inputOption != 'q') {
    		xil_printf("\r\n -- Press any key to continue --\r\n");
    		getchar();
    	}
    	else {
    		break;
    	}
    }

    xil_printf("\r\n\r\nExiting...\r\n");

	return 0;
}

void main_PrintMainMenu()
{
	xil_printf("\x1B[H"); //Set cursor to top left of terminal
	xil_printf("\x1B[2J"); //Clear terminal
	xil_printf("**************************************************\n\r");
	xil_printf("*             ZYBO Pmod ToF - Demo               *\n\r");
	xil_printf("**************************************************\n\r");

	xil_printf("\n\r");
	xil_printf("Select one of the available commands displayed below\n\r");
	xil_printf("\n\r");

	xil_printf(" m - ToFMeasure - Measure and display distance.\n\r");
	xil_printf(" c - ToFStartCalib - Start manual calibration\n\r");
	xil_printf(" s - ToFSaveCalib - Save manual calibration to EEPROM\n\r");
	xil_printf(" r - ToFRestoreFactCalib - Restore factory calibration from EEPROM\n\r");
	xil_printf(" n - ToFReadSerialNo - Display Pmod serial number\n\r");
	xil_printf(" q - Quit\n\r");
	xil_printf("\n\r");
	xil_printf("**************************************************\n\r");
	xil_printf("\n\r");
	xil_printf("Enter a selection\r\n");
	xil_printf("\n\r");
}




