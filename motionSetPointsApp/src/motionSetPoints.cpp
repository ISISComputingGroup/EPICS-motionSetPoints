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

#include "motionSetPoints.h"

#include "setPoint.hpp"

#include <macLib.h>
#include <epicsGuard.h>

#include <epicsExport.h>

static const char *driverName = "motionSetPoints";

motionSetPoints::motionSetPoints(const char *portName, const char* fileName) 
   : asynPortDriver(portName, 
                    0, /* maxAddr */ 
                    NUM_MSP_PARAMS, /* num parameters */
                    asynInt32Mask | asynFloat64Mask | asynOctetMask | asynDrvUserMask, /* Interface mask */
                    asynInt32Mask | asynFloat64Mask | asynOctetMask,  /* Interrupt mask */
                    ASYN_CANBLOCK, /* asynFlags.  This driver can block but it is not multi-device */
                    1, /* Autoconnect */
                    0,
                    0), m_fileName(fileName)
{
   	createParam(P_positionsString, asynParamOctet, &P_positions);
    createParam(P_posnSPRBVString, asynParamOctet, &P_posnSPRBV);
	createParam(P_posnSPString, asynParamOctet, &P_posnSP);
    createParam(P_posnString, asynParamOctet, &P_posn);
	createParam(P_coord1String, asynParamFloat64, &P_coord1);
    createParam(P_coord1RBVString, asynParamFloat64, &P_coord1RBV);
    createParam(P_resetString, asynParamFloat64, &P_reset);
    createParam(P_filter1String, asynParamOctet, &P_filter1);
    createParam(P_filter2String, asynParamOctet, &P_filter2);
    createParam(P_filteroutString, asynParamOctet, &P_filterout);  
	// initial values
    setStringParam(P_filterout, "");
    setStringParam(P_posn, "");
    setStringParam(P_posnSPRBV, "");
    setDoubleParam(P_coord1, 0.0);
}

asynStatus motionSetPoints::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    return writeFloat64(pasynUser, value);
}

void motionSetPoints::updatePositions() 
{
    const int buffer_size = 4096;
	char* buffer = new char[buffer_size];
	getPositions(buffer, MAX_STRING_SIZE, buffer_size / MAX_STRING_SIZE, m_fileName.c_str());
	setStringParam(P_positions, buffer);  
		printf("positions: %s\n", buffer);
	delete[] buffer;
}

asynStatus motionSetPoints::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    int function = pasynUser->reason;
    const char* functionName = "writeFloat64";
    asynStatus status = asynSuccess;
    const char *paramName = NULL;
	getParamName(function, &paramName);
    if (function == P_coord1RBV)
	{
//		printf("value: %f\n", value);
        posn2name(value, 1e10, m_fileName.c_str());
        setDoubleParam(P_coord1, currentPosn(1, m_fileName.c_str()));
//		printf("coord1: %f\n", currentPosn(1, m_fileName.c_str()));
	}
	else if (function == P_reset)
	{
    	loadDefFile(m_fileName.c_str());
		updatePositions();
	}
	else
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

asynStatus motionSetPoints::writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual)
{
    int function = pasynUser->reason;
    const char* functionName = "writeOctet";
    asynStatus status = asynSuccess;
    const char *paramName = NULL;
    char buffer[256], buffer2[256];
	getParamName(function, &paramName);
//	printf("value: %s\n", value);
    if (function == P_posnSP)
	{
	    name2posn(value, m_fileName.c_str());
        getPosnName(buffer, 0, m_fileName.c_str());
        setStringParam(P_posn, buffer);  
        getPosnName(buffer2, 1, m_fileName.c_str());
        setStringParam(P_posnSPRBV, buffer2);  
        setDoubleParam(P_coord1, currentPosn(1, m_fileName.c_str()));
		*nActual = strlen(value);
//		printf("posn %s posnsprbv %s\n", buffer, buffer2);
	}
	else if (function == P_filter1)
	{
        setFilter("FILTER1", value, m_fileName.c_str());
		updatePositions();
        getFilterOut(buffer, m_fileName.c_str());
        setStringParam(P_filterout, buffer);
		*nActual = strlen(value);
//		printf("filterout %s\n", buffer);
	}
	else if (function == P_filter2)
	{
        setFilter("FILTER2", value, m_fileName.c_str());
		updatePositions();
        getFilterOut(buffer, m_fileName.c_str());
        setStringParam(P_filterout, buffer);
		*nActual = strlen(value);
//		printf("filterout %s\n", buffer);
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

int motionSetPointsConfigure(const char *portName, const char* fileName)
{
	try
	{
		new motionSetPoints(portName, fileName);
		return(asynSuccess);
	}
	catch(const std::exception& ex)
	{
		std::cerr << "motionSetPoints failed: " << ex.what() << std::endl;
		return(asynError);
	}
}

// EPICS iocsh shell commands 

static const iocshArg initArg0 = { "portName", iocshArgString};			///< The name of the asyn driver port we will create
static const iocshArg initArg1 = { "fileName", iocshArgString};			///< The name of the lookup file

static const iocshArg * const initArgs[] = { &initArg0, &initArg1 };

static const iocshFuncDef initFuncDef = {"motionSetPointsConfigure", sizeof(initArgs) / sizeof(iocshArg*), initArgs};

static void initCallFunc(const iocshArgBuf *args)
{
    motionSetPointsConfigure(args[0].sval, args[1].sval);
}

static void motionSetPointsRegister(void)
{
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(motionSetPointsRegister);

}

