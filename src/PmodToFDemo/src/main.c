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
/* https://reference.digilentinc.com/reference/pmod/pmodtof/demouserguide       			*/
/*                                                                            				*/
/* This demo  implements a command interpreter that listens over UART for individual		*/
/* Pmod ToF commands.																		*/
/* The following list enumerates the commands implemented in the PmodToFCMD   				*/
/* module:																					*/
/*																							*/
/*    ToFMeasure																			*/
/*    ToFStartCalib																			*/
/*    ToFSaveCalib																			*/
/*    ToFRestoreFactCalib																	*/
/*    ToFReadSerialNo																		*/
/*																							*/
/* The user will have to provide these commands using a terminal connected    				*/
/* to the USB port corresponding to the connected board,           				            */
/* configured with 115200 Baud rate.                                           				*/
/*                                                                            				*/
/*                                                                            				*/
/********************************************************************************************/
/* Revision History:                                                          				*/
/*                                                                            				*/
/*    09/23/2019(anamariabalas):   Created                                    				*/
/*    09/23/2019(anamariabalas): Validated for Vivado 2019.1                  				*/
/*                                                                            				*/
/********************************************************************************************/

#include "xparameters.h"
#include <stdio.h>
#include "PmodToF/PmodToF.h"
#include "errors.h"
#include "PmodToFCMD.h"
#include "UART.h"


int main(void)
{


	PmodToF_Initialize();
	UART_Init(115200);
	ERRORS_Init("OK", "ERROR");
	UART_PutString("Command loop\r\n");
    while(1)
    {
    	PmodToFCMD_CheckForCommand();
    }

	return 0;
}




