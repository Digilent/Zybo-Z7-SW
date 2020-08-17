/******************************************************************************/
/*                                                                            */
/* PmodToFCMD.h -- PmodToFCMD module                                          */
/*                                                                            */
/******************************************************************************/
/* Author: Ana-Maria-Eliza Balas                                              */
/*		   ana-maria.balas@digilent.ro										  */
/* Copyright 2019, Digilent Inc.                                              */
/*                                                                            */
/******************************************************************************/
/* Module Description:                                                        */
/*                                                                            */
/* This file contains the declaration for the functions of the PmodToFCMD.c   */
/* module. 	                                                                  */
/*                                                                            */
/******************************************************************************/
/* Revision History:                                                          */
/*                                                                            */
/*    09/23/2019(anamariabalas):   Created                                    */
/*    09/23/2019(anamariabalas): Validated for Vivado 2019.1                  */
/*                                                                            */
/******************************************************************************/
#ifndef CMD_H_
#define CMD_H_

/***************************** Include Files *********************************/
#include "string.h"
#include "stdlib.h"
#include "xstatus.h"

/************************** Type definitions ******************************/
typedef enum {
	CMD_NONE = -1, // No command
	INVALID = 0, // Invalid command
/* Add/remove test constants below this line */
	CMD_StartCalib,
	CMD_ReadSerialNo,
	CMD_Measure,
	CMD_SaveCalibToEprom,
	CMD_RestoreFactCalib

} cmd_key_t;

// structure mapping command key to command string
typedef struct {
	char *pchCmd;
	cmd_key_t eCmd;
} cmd_map_t;
/************************** Definitions ******************************/

/************************** Function Prototypes ******************************/

void PmodToFCMD_CheckForCommand();



#endif /* CMD_H_ */
