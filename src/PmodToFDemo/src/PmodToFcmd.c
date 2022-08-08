/*********************************************************************************/
/*                                                                               */
/* PmodToFCMD.c -- PmodToFCMD module                                             */
/*                                                                               */
/*********************************************************************************/
/* Author: Ana-Maria-Eliza Balas                                                 */
/*		   ana-maria.balas@digilent.ro										     */
/* Copyright 2019, Digilent Inc.                                                 */
/*                                                                               */
/*********************************************************************************/
/* Module Description:                                                           */
/*                                                                               */
/* This module is used to interpret the UART received content,                   */
/* recognize specific commands and call the appropriate functions.				 */
/* It access all the Pmod ToF functionality .                                    */
/* More details can be found at:												 */
/*                                                                               */
/* https://reference.digilentinc.com/reference/pmod/pmodtof/zynqlibraryuserguide */
/*                                                                               */
/*                                                                               */
/*********************************************************************************/
/* Revision History:                                                             */
/*                                                                               */
/*    09/23/2019(anamariabalas):   Created                                       */
/*    09/23/2019(anamariabalas): Validated for Vivado 2019.1					 */
/*	  05/08/2022(raduvele): Modified the I/O format							     */
/*                                                                               */
/*********************************************************************************/


/* ----------------------------------------------------------------------------- */
/*					Include Files    						                     */
/* ----------------------------------------------------------------------------- */

#include "PmodToFCMD.h"
#include "errors.h"
#include "PmodToF/PmodToF.h"
#include "UART.h"
#include "sleep.h"

#define MAX_CMD_LENGTH			100
#define MAX_DIST_LENGTH			5

/********************* Global Constant Definitions ***************************/
const cmd_map_t uartCommands[] = {
	{'c',   	CMD_StartCalib},
	{'n',   	CMD_ReadSerialNo},
	{'m',   	CMD_Measure},
	{'s',   	CMD_SaveCalibToEprom},
	{'r',   	CMD_RestoreFactCalib}
};


/********************* Global Variables Definitions ***************************/

char *pszLastErr;
char readDistance[MAX_DIST_LENGTH];

// variables used in multiple functions// allocate them only once.
char szVal[20];
int dMeasuredVal, N = 100;

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions Prototypes                                        */
/* ************************************************************************** */
/* ************************************************************************** */

cmd_key_t PmodToFCMD_CmdDecode(char inputOption);
void PmodToFCMD_ProcessCmd(cmd_key_t keyCmd);
char* PmodToFCMD_CmdGetNextArg();
u8 PmodToFCMD_CmdReadSerialNo();
void PmodToFCMD_MeasureCmd();

/********************* Function Definitions ***************************/

/***	PmodToFCMD_CheckForCommand()
**
**	Parameters:
**          char inputOption		- the character corresponding to the command selected by the user
**
**
**	Return Value:
**          none
**
**	Description:
**		This function checks on UART if a command was received.
**      It compares the received command with the commands defined in the commands array. If recognized, the command is processed accordingly.
**
*/
void PmodToFCMD_CheckForCommand(char inputOption)
{
	sprintf(szMsg, "Received command: %c\r\n", inputOption);
	xil_printf("%s\r\n", szMsg);
	PmodToFCMD_ProcessCmd(PmodToFCMD_CmdDecode(inputOption));
}

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */
/*****************************************************************************/

/*****************************************************************************/
/***	PmodToFCMD_CmdDecode
**
**	Parameters:
**		char inputOption       - character that represents the chosen command
**
**	Return Value:
**          cmd_key_t - the command enumeration value for the found command, or INVALID if command was not found.
**
**	Description:
**		This function tries to identify a command among the defined commands.
**      It compares the received command with the commands defined in the list of defined commands.
 *      If the command is found, then the command key is returned. Otherwise INVALID
**      If the command is not found, INVALID enumeration value is returned.
**
*/
cmd_key_t PmodToFCMD_CmdDecode(char inputOption)
{

	int idxCmd;

	for(idxCmd = 0; idxCmd < sizeof(uartCommands)/sizeof(uartCommands[0]); idxCmd++)
	{
		if(inputOption == uartCommands[idxCmd].pchCmd)
		{
			return uartCommands[idxCmd].eCmd;
		}
	}

	if(inputOption == 'q') {
		return INVALID;
	}

	xil_printf("Unrecognized command\r\n");
	return INVALID;
}

/***	PmodToFCMD_ProcessCmd
**
**	Parameters:
**     cmd_key_t keyCmd           - the enumerator key corresponding to the command
**
**	Return Value:
**		none
**
**	Description:
**		This function calls the processing function corresponding to the provided enumerator key.
**      It properly provides the command arguments.
**
**
*/
void PmodToFCMD_ProcessCmd(cmd_key_t keyCmd)
{
	u8 bErrCode;
    switch(keyCmd)
    {
        case CMD_StartCalib:
        	bErrCode = PmodToF_start_calibration(atof(PmodToFCMD_CmdGetNextArg()));
    		ERRORS_GetPrefixedMessageString(bErrCode, "", szMsg);
    		UART_PutString(szMsg);
            break;
        case CMD_ReadSerialNo:
        	PmodToFCMD_CmdReadSerialNo();
            break;
        case CMD_Measure:
        	PmodToFCMD_MeasureCmd();
			break;
        case CMD_RestoreFactCalib:
        	bErrCode = PmodToF_RestoreAllCalibsFromEPROM_Factory();
        	ERRORS_GetPrefixedMessageString(bErrCode, "", szMsg);
        	UART_PutString(szMsg);
			break;
        case CMD_SaveCalibToEprom:
        	bErrCode = PmodToF_WriteCalibsToEPROM_User();
        	ERRORS_GetPrefixedMessageString(bErrCode, "", szMsg);
        	UART_PutString(szMsg);
			break;
        default:
        	// do nothing
            break;
    }
    usleep(500);
    return;
}

/*****************************************************************************/

/***	PmodToFCMD_CmdReadSerialNo
**
**	Parameters:
**     none
**
**	Return Value:
**		uint8_t     - the error code
**          ERRVAL_SUCCESS              0       // success
**          ERRVAL_EPROM_CRC            0xFE    // wrong CRC when reading data from EPROM
**          ERRVAL_EPROM_MAGICNO        0xFD    // wrong Magic No. when reading data from EPROM
**
**	Description:
**		This function implements the ToFReadSerialNo text command.
**      It calls SERIALNO_ReadSerialNoFromEPROM and collects the serial number string.
**		In case of success, the function sends the success message containing the serial number over UART.
**		In case of error, the error specific message is sent over UART.
**      The function returns the error code, which is the error code returned by the SERIALNO_ReadSerialNoFromEPROM function.
**      The function is called by PmodToF_ProcessCmd function.
**
*/
u8 PmodToFCMD_CmdReadSerialNo()
{
	u8 bErrCode = ERRVAL_SUCCESS;
    char szSerialNo[SERIALNO_SIZE + 1];
    bErrCode = PmodToF_ReadSerialNoFromEPROM(szSerialNo);
    if (bErrCode == ERRVAL_SUCCESS)
    {
        sprintf(szMsg, "SerialNo = \"%s\".", szSerialNo);
    }
    ERRORS_GetPrefixedMessageString(bErrCode, "", szMsg);
    UART_PutString(szMsg);
    return bErrCode;
}

/***	PmodToFCMD_CmdGetNextArg
**
**	Parameters:
**		<none>
**
**	Return Value:
**          char *  string containing the parameter for ToFStartCalib
**
**	Description:
**		This function retrieves the calibration distance from the user. The calibration distance is used in the ToFStartCalib command.
**
**
*/
char* PmodToFCMD_CmdGetNextArg()
{
	xil_printf("Input the manual calibration distance: ");
	char readDigit = 0;
	int distArrLength = 0;

	while(readDigit != 13) //reading ends with an enter
	{
		readDigit = getchar();
		putchar(readDigit);
		readDistance[distArrLength] = readDigit;
		distArrLength++;
	}

	readDistance[distArrLength] = '\0'; //null-ended string
	return readDistance;
}


/***	PmodToFCMD_MeasureCmd
**
**	Parameters:
**     none
**
**	Return Value:
**          ERRVAL_SUCCESS              0       // success
**
**	Description:
**		This function displays over UART the distance measured by the device.
**		Before calling this function, it is important that a manual calibration was made or the calibration
**		was imported(calibration stored by the user in EEPROM user area )/restored from EEPROM(factory calibration).
*/
void PmodToFCMD_MeasureCmd()
{
	int N = 100, sum = 0;
	int distance_val, distance_val_avg;
	// 100 distance values that are measure will be averaged into a final distance value
	for(int j=0;j<N;j++)
	{
		distance_val = 1000 * PmodToF_perform_distance_measurement(); // the distance value is in millimeters
		sum = sum + distance_val;
	}
	distance_val_avg = sum/N;
    sprintf(szMsg, "Distance measured D = %d mm.", distance_val_avg);
    ERRORS_GetPrefixedMessageString(ERRVAL_SUCCESS, "", szMsg);
    UART_PutString(szMsg);

}
