#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <exception>
#include <iostream>
#include <limits>

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
                    NUM_AXES, /* maxAddr */ 
                    NUM_MSP_PARAMS, /* num parameters */
                    asynInt32Mask | asynFloat64Mask | asynOctetMask | asynDrvUserMask, /* Interface mask */
                    asynInt32Mask | asynFloat64Mask | asynOctetMask,  /* Interrupt mask */
                    ASYN_CANBLOCK, /* asynFlags.  This driver can block but it is not multi-device */
                    1, /* Autoconnect */
                    0,
                    0), m_fileName(fileName), m_coord1(0.0), m_coord2(0.0), m_tol(1.e10)
{
    // Parameters for general control, only used on the 0th address
   	createParam(P_positionsString, asynParamOctet, &P_positions);
    createParam(P_posnSPRBVString, asynParamOctet, &P_posnSPRBV);
    createParam(P_iposnSPRBVString, asynParamInt32, &P_iposnSPRBV);
	createParam(P_posnSPString, asynParamOctet, &P_posnSP);
	createParam(P_iposnSPString, asynParamInt32, &P_iposnSP);
    createParam(P_posnString, asynParamOctet, &P_posn);
    createParam(P_iposnString, asynParamInt32, &P_iposn);
    createParam(P_nposnString, asynParamOctet, &P_nposn);
    createParam(P_niposnString, asynParamInt32, &P_niposn);
    createParam(P_resetString, asynParamFloat64, &P_reset);
    createParam(P_numPosString, asynParamInt32, &P_numpos);
    createParam(P_numAxesString, asynParamFloat64, &P_numAxes);
    createParam(P_tolString, asynParamFloat64, &P_tol);
    createParam(P_posDiffString, asynParamFloat64, &P_posDiff);

    // Parameters for each coordinate, these will be set on each of the addresses
	createParam(P_coordString, asynParamFloat64, &P_coord);
    createParam(P_coordRBVString, asynParamFloat64, &P_coordRBV);

	// initial values
    setStringParam(P_posn, "");
    setStringParam(P_nposn, "");
    setStringParam(P_posnSP, "");
    setStringParam(P_posnSPRBV, "");
    setIntegerParam(P_iposn, -1);
    setIntegerParam(P_niposn, -1);
    setIntegerParam(P_iposnSP, -1);
    setIntegerParam(P_iposnSPRBV, -1);
    setIntegerParam(P_numpos, 0);
    setDoubleParam(P_tol, m_tol);

    for (int i = 0; i < NUM_AXES; i++) {
        setDoubleParam(i, P_coord, 0.0);
        setDoubleParam(i, P_coordRBV, 0.0);
    }

    loadDefFile(m_fileName.c_str());
	updatePositions();
}

// new position requsted
asynStatus motionSetPoints::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    int function = pasynUser->reason;
    const char* functionName = "writeInt32";
    asynStatus status = asynSuccess;
    const char *paramName = NULL;
	getParamName(function, &paramName);
    if (function == P_iposnSP)
    {
        setIntegerParam(P_iposnSP, value);
        std::string posn = getPositionByIndex(value, m_fileName.c_str());
        if ( (posn.size() > 0) && (gotoPosition(posn.c_str()) == 0) )
        {
            ;
        }
        else
        {
            epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s:%s: Unknown position index %d", driverName, functionName, paramName, value);
		    status = asynError;
        }
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

// Update the list of available setpoint names
void motionSetPoints::updatePositions() 
{
    const int buffer_size = 8192;
	char* buffer = new char[buffer_size];
	getPositions(buffer, MAX_STRING_SIZE, buffer_size / MAX_STRING_SIZE, m_fileName.c_str());
	setStringParam(P_positions, buffer);
    setIntegerParam(P_numpos, static_cast<int>(numPositions(m_fileName.c_str())));
    setDoubleParam(P_numAxes, getNumCoords(m_fileName.c_str()));
	delete[] buffer;
}

asynStatus motionSetPoints::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    int function = pasynUser->reason;
    const char* functionName = "writeFloat64";
    asynStatus status = asynSuccess;
    const char *paramName = NULL;
    int coordinate;

	getParamName(function, &paramName);
    getAddress(pasynUser, &coordinate);
    if (coordinate == 0 && function == P_coord)
	{
		// Motor 1 has moved
		updateCurrPosn(value, m_coord2);
	}
    else if (coordinate == 1 && function == P_coord)
	{
		// Motor 2 has moved
		updateCurrPosn(m_coord1, value);
	}
	else if (function == P_reset)
	{
		// Been asked to reload the files
    	loadDefFile(m_fileName.c_str());
		updatePositions();
	}
    else if (function == P_tol)
	{
		m_tol = value;
		setDoubleParam(P_tol, value);
		updateCurrPosn(m_coord1, m_coord2); // as tolerance has changed, this may no longer count as a valid position 
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

// Update the current position name based on the current motor coordinates
void motionSetPoints::updateCurrPosn(double coord1, double coord2) 
{
	char buffer[256];
	double position, pos_diff;
	bool pos_ok = false;
	int ncoords = getNumCoords(m_fileName.c_str());
	static const double max_tol = sqrt(std::numeric_limits<double>::max());

	m_coord1 = coord1;
	m_coord2 = coord2;
	if ( ncoords == 2 )
	{
        posn2name(coord1, coord2, max_tol, m_fileName.c_str(), pos_diff);
        getPosnName(buffer, 0, m_fileName.c_str());
        setStringParam(P_nposn, buffer);  
        setIntegerParam(P_niposn, getPositionIndexByName(buffer, m_fileName.c_str()));
        if (posn2name(coord1, coord2, m_tol, m_fileName.c_str(), pos_diff) == 0)
		{
		    getPosn(0, false, m_fileName.c_str(), position);
            setDoubleParam(0, P_coord, position);
		    getPosn(1, false, m_fileName.c_str(), position);
            setDoubleParam(1, P_coord, position);
			pos_ok = true;
		}
	}
	else if ( ncoords == 1 )
	{
        posn2name(coord1, max_tol, m_fileName.c_str(), pos_diff);
        getPosnName(buffer, 0, m_fileName.c_str());
        setStringParam(P_nposn, buffer);  
        setIntegerParam(P_niposn, getPositionIndexByName(buffer, m_fileName.c_str()));
        if (posn2name(coord1, m_tol, m_fileName.c_str(), pos_diff) == 0)
		{
		    getPosn(0, false, m_fileName.c_str(), position);
            setDoubleParam(0, P_coord, position);
			pos_ok = true;
		}
	}
    if (pos_ok)
	{
        setDoubleParam(P_posDiff, pos_diff);
        getPosnName(buffer, 0, m_fileName.c_str());
        setStringParam(P_posn, buffer);  
        setIntegerParam(P_iposn, getPositionIndexByName(buffer, m_fileName.c_str()));
	}
	else
	{
		// if no position is within tolerance, should we blank out current position or leave it unchanged?
		// we will blank it out as (iposn == iposnSP) is now a test in the DB file.
        setStringParam(P_posn, "");
        setIntegerParam(P_iposn, -1);
		// note: P_coord and P_posDiff will now refer to last valid position, not sure of sensible reset values
	}
}

int motionSetPoints::gotoPosition(const char* value)
{
	double position;
    char positionName[256];
	if ( name2posn(value, m_fileName.c_str()) == 0 )
    {
			getPosnName(positionName, 1, m_fileName.c_str());
            setStringParam(P_posnSPRBV, positionName);  
            setIntegerParam(P_iposnSPRBV, getPositionIndexByName(positionName, m_fileName.c_str()));
			if (getPosn(0, true, m_fileName.c_str(), position) == 0)
			{
                setDoubleParam(0, P_coordRBV, position);
			}
            if ( getNumCoords(m_fileName.c_str()) == 2 && getPosn(1, true, m_fileName.c_str(), position) == 0 ) 
            {
                setDoubleParam(1, P_coordRBV, position);
            }
            return 0;
    }
    else
    {
        return -1;
    }
}

asynStatus motionSetPoints::writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual)
{
    int function = pasynUser->reason;
    const char* functionName = "writeOctet";
    asynStatus status = asynSuccess;
    const char *paramName = NULL;
	getParamName(function, &paramName);
    if (function == P_posnSP)
	{
		// Been told to go to a new setpoint by name
	    if ( gotoPosition(value) == 0 )
        {
            setStringParam(P_posnSP, value);  
		    *nActual = strlen(value);
        }
        else
        {
            epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s:%s: Unknown position \"%s\"", driverName, functionName, paramName, value);
		    status = asynError;
		    *nActual = 0;
        }
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

// called from the ioc shell to configure this ioc
// portName - a unique name
// fileName - environment variable name containing the lookup file name
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

