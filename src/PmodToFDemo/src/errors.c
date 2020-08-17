/******************************************************************************/
/*                                                                            */
/* errors.c --  errors module                                       */
/*                                                                            */
/******************************************************************************/
/* Author: Ana-Maria-Eliza Balas                                              */
/*		   ana-maria.balas@digilent.ro										  */
/* Copyright 2019, Digilent Inc.                                              */
/*                                                                            */
/******************************************************************************/
/* Module Description:                                                        */
/*                                                                            */
/* This module groups the error related functions.                            */
/*                                                                            */
/******************************************************************************/
/* Revision History:                                                          */
/*                                                                            */
/*    09/23/2019(anamariabalas):   Created                                    */
/*    09/23/2019(anamariabalas): Validated for Vivado 2019.1                  */
/*                                                                            */
/******************************************************************************/


/* -------------------------------------------------------------------------- */
/*					Include Files    						                  */
/* -------------------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include "stdint.h"
#include "errors.h"
#include "PmodToF/PmodToF.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Constants                                                         */
/* ************************************************************************** */
/* ************************************************************************** */
#define PREFIX_SIZE 10

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Global Variables                                                  */
/* ************************************************************************** */
/* ************************************************************************** */
char szLastError[MSG_ERROR_SIZE];
char prefixes[2][PREFIX_SIZE];
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Local Functions Prototypes                                                  */
/* ************************************************************************** */
/* ************************************************************************** */
char * ERRORS_PrefixMessage(msg_prefix_status prefix, char *pDestString, const char *szMsg);

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */
/* ------------------------------------------------------------ */
/***    ERRORS_Init
**
**	Synopsis:
**		
**
**	Parameters:
**      const char *szPrefixSuccess  - The error code for which the error string is requested/needed to be transmitted further to the user
**      const char *szPrefixError         - The characters string acting as content for some of the error messages
**		
**
**	Return Values:
**     	none
**
**	Description:
**		This function copies in PREFIX_SUCCESS the prefix message for success and copies in PREFIX_ERROR the prefix message for fail.

**		
*/
void ERRORS_Init(const char *szPrefixSuccess, const char *szPrefixError)
{
    strncpy(prefixes[(int)PREFIX_SUCCESS], szPrefixSuccess, PREFIX_SIZE - 1);
    strncpy(prefixes[(int)PREFIX_ERROR], szPrefixError, PREFIX_SIZE - 1);
}

/* ------------------------------------------------------------ */
/***    ERRORS_GetPrefixedMessageString
**
**	Synopsis:
**		
**
**	Parameters:
**      uint8_t bErrCode  - The error code for which the error string is requested/needed to be transmitted further to the user
**      char *szContent         - The characters string acting as content for some of the error messages
**      char *pSzErr            - String to receive the error meaning
**		
**
**	Return Values:
**      ERRVAL_SUCCESS                   0   - success
**      ERRVAL_CMD_MISSINGCODE        0xF9   - The provided code is not among accepted values
**
**	Description:
**		This function copies in the szLastError the error message corresponding to the provided error code, 
**      for all error codes except ERRVAL_SUCCESS.
**      Then it copies in the pSzErr the properly prefixed message.
**      Therefore is important that the caller of this function allocates enough space in pSzErr (70 characters).
**      Some of the error messages include the string provided in the szContent parameter. The parameter is ignored for the other error messages.
**      If the error code is among the defined ones, a specific error string is copied in the pSzErr
**      and ERRVAL_SUCCESS is returned.
**      If the error is not among the defined ones, ERRVAL_CMD_MISSINGCODE is returned and pSzErr is not altered.
**		
*/
uint8_t ERRORS_GetPrefixedMessageString(uint8_t bErrCode, char *szContent, char *pSzErr)
{
    uint8_t bResult = ERRVAL_SUCCESS;
    msg_prefix_status prefix;
    switch(bErrCode)
    {
        case ERRVAL_SUCCESS: 
//          the message is in pSzErr string
            strcpy(szLastError, pSzErr);
            prefix = PREFIX_SUCCESS;
            break;
        case ERRVAL_INCORRECT_CALIB_DISTACE:
            strcpy(szLastError, "Incorrect calibration distance(distance is less than 5 cm).");
            prefix = PREFIX_ERROR;
            break;
        case ERRVAL_EPROM_CRC:
            strcpy(szLastError, "Invalid EPROM checksum.");
            prefix = PREFIX_ERROR;
            break;
        case ERRVAL_EPROM_MAGICNO:
            strcpy(szLastError, "Invalid EPROM magic number.");
            prefix = PREFIX_ERROR;
            break;
        case ERRVAL_FAILED_STARTING_MEASURE:
            strcpy(szLastError, "Failed starting measurement.");
            prefix = PREFIX_ERROR;
            break;
        case ERRVAL_FAILED_STARTING_CALIB:
			strcpy(szLastError, "Failed starting manual calibration.");
			prefix = PREFIX_ERROR;
			break;
        case ERRVAL_EPROM_WRITE:
            strcpy(szLastError, "EPROM write over IIC error.");
            prefix = PREFIX_ERROR;
            break;
        case ERRVAL_EPROM_READ:
            strcpy(szLastError, "EPROM read over IIC error.");
            prefix = PREFIX_ERROR;
            break;
        case ERRVAL_ToF_WRITE:
            strcpy(szLastError, "ToF write over IIC error.");
            prefix = PREFIX_ERROR;
            break;
        case ERRVAL_ToF_READ:
			strcpy(szLastError, "ToF read over IIC error.");
			prefix = PREFIX_ERROR;
			break;
        default:
               break;
    }
    ERRORS_PrefixMessage(prefix, pSzErr, szLastError);

    return bResult;
    
}
/* ------------------------------------------------------------ */
/***    ERRORS_GetszLastError
**
**	Synopsis:
**		
**
**	Parameters:
**      none
**		
**	Return Values:
**      char szLastError - last saved error code
**
**
**	Description:
**		This function returns the last saved error code.
**      
**		
*/
char *ERRORS_GetszLastError()
{
    return szLastError;
}

char* ERRORS_PrefixMessage(msg_prefix_status prefix, char *pDestString, const char *szMsg)
{
    if(prefix != PREFIX_EMPTY)
    {
        strcpy(pDestString, prefixes[(int)prefix]); // copy prefix
        if(szMsg[0])
        {
            strcat(pDestString, ",");
        }        
    }
    else
    {
        pDestString[0] = 0; // empty string
    }
    strcat(pDestString, szMsg);
    strcat(pDestString, "\r\n");
    return pDestString;
}

/* *****************************************************************************
 End of File
 */
