#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <exception>
#include <iostream>

#include <epicsTypes.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsTimer.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <iocsh.h>

#include "sampleChanger.h"

#include <macLib.h>
#include <epicsGuard.h>

#include <epicsExport.h>

static const char *driverName = "sampleChanger";

sampleChanger::sampleChanger(const char *portName, const char* fileName) 
   : asynPortDriver(portName, 
                    0, /* maxAddr */ 
                    NUM_MSP_PARAMS, /* num parameters */
                    asynInt32Mask | asynFloat64Mask | asynOctetMask | asynDrvUserMask, /* Interface mask */
                    asynInt32Mask | asynFloat64Mask | asynOctetMask,  /* Interrupt mask */
                    ASYN_CANBLOCK, /* asynFlags.  This driver can block but it is not multi-device */
                    1, /* Autoconnect */
                    0,
                    0), m_fileName(fileName), m_outval(1.0)
{
    createParam(P_recalcString, asynParamOctet, &P_recalc);  
    createParam(P_outvalString, asynParamFloat64, &P_outval);  

	// initial values
    setDoubleParam(P_outval, m_outval);
}

asynStatus sampleChanger::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    return writeFloat64(pasynUser, value);
}

asynStatus sampleChanger::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    int function = pasynUser->reason;
    const char* functionName = "writeFloat64";
    asynStatus status = asynSuccess;
    const char *paramName = NULL;
	getParamName(function, &paramName);
//    if (function == P_coord1RBV)
//	{
////		printf("value: %f\n", value);
//        posn2name(value, 1e10, m_fileName.c_str());
//        setDoubleParam(P_coord1, currentPosn(1, m_fileName.c_str()));
////		printf("coord1: %f\n", currentPosn(1, m_fileName.c_str()));
//	}
//	else if (function == P_reset)
//	{
//    	loadDefFile(m_fileName.c_str());
//		updatePositions();
//	}
//	else
	{
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d, name=%s, value=%f, error=%s", 
                  driverName, functionName, status, function, paramName, value, "unknown parameter");
		status = asynError;
	}
	callParamCallbacks();
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d, name=%s, value=%f\n", 
              driverName, functionName, function, paramName, value);
	return status;
}

asynStatus sampleChanger::writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual)
{
    int function = pasynUser->reason;
    const char* functionName = "writeOctet";
    asynStatus status = asynSuccess;
    const char *paramName = NULL;
    //char buffer[256], buffer2[256];
	getParamName(function, &paramName);
	printf("value: %s\n", value);

	if (function == P_recalc) 
	{
		converter c;
		c.createLookup();
		
		setDoubleParam(P_outval, ++m_outval);
		*nActual = strlen(value);
        printf("Here %s %f\n", m_fileName.c_str(), m_outval);
	}
	else
	{
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d, name=%s, value=%s, error=%s", 
                  driverName, functionName, status, function, paramName, value, "unknown parameter");
		status = asynError;
		*nActual = 0;
	}
	callParamCallbacks();
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d, name=%s, value=%s\n", 
              driverName, functionName, function, paramName, value);
	return status;
}


extern "C" {

int sampleChangerConfigure(const char *portName, const char* fileName)
{
	try
	{
		new sampleChanger(portName, fileName);
		return(asynSuccess);
	}
	catch(const std::exception& ex)
	{
		std::cerr << "sampleChanger failed: " << ex.what() << std::endl;
		return(asynError);
	}
}

// EPICS iocsh shell commands 

static const iocshArg initArg0 = { "portName", iocshArgString};			///< The name of the asyn driver port we will create
static const iocshArg initArg1 = { "fileName", iocshArgString};			///< The name of the lookup file

static const iocshArg * const initArgs[] = { &initArg0, &initArg1 };

static const iocshFuncDef initFuncDef = {"sampleChangerConfigure", sizeof(initArgs) / sizeof(iocshArg*), initArgs};

static void initCallFunc(const iocshArgBuf *args)
{
    sampleChangerConfigure(args[0].sval, args[1].sval);
}

static void sampleChangerRegister(void)
{
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(sampleChangerRegister);

}

