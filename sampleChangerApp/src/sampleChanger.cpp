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

#include "setPoint.hpp"

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
		loadDefRackDefs("RACKDEFS");
		createLookup();
		
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

void sampleChanger::loadDefRackDefs(const char* env_fname) 
{
	const char *fname = getenv(env_fname);
	if ( fname==NULL ) {
		printf("sampleChanger: Environment variable \"%s\" not set\n", env_fname);
		return;
	}
	loadRackDefs(fname);
}

void sampleChanger::loadRackDefs(const char* fname) 
{
	TiXmlDocument doc(fname);
	if (!doc.LoadFile()) {
		printf("sampleChanger: Unable to open rack defs file \"%s\"\n", fname);
		return;
	}

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	pElem=hDoc.FirstChildElement().Element();
	if (!pElem) {
		printf("sampleChanger: Unable to parse rack defs file \"%s\"\n", fname);
		return;
	}

	// save this for later
	hRoot=TiXmlHandle(pElem);

	loadRackDefs(hRoot);
	loadSlotDefs(hRoot);
}

void sampleChanger::loadRackDefs(TiXmlHandle &hRoot)
{
	m_racks.clear();
	
	for( TiXmlElement* pElem=hRoot.FirstChild("racks").FirstChild().Element(); pElem; pElem=pElem->NextSiblingElement())
	{
		std::map<std::string, samplePosn> posns;
		std::string rackName = pElem->Attribute("name");
		for ( TiXmlElement *pRack = pElem->FirstChildElement("position") ; pRack ; pRack=pRack->NextSiblingElement() ) {
			samplePosn posn;
			const char *attrib = pRack->Attribute("name");
			if ( attrib!=NULL ) {
				posn.name = attrib;
			}
			else {
				printf("sampleChanger: rack has no name attribute \"%s\"\n", rackName);
			}
			if ( pRack->QueryDoubleAttribute("x", &posn.x)!=TIXML_SUCCESS ) {
				printf("sampleChanger: unable to read x attribute \"%s\" \"%s\"\n", rackName, posn.name);
			}
			if ( pRack->QueryDoubleAttribute("y", &posn.y)!=TIXML_SUCCESS ) {
				printf("sampleChanger: unable to read y attribute \"%s\" \"%s\"\n", rackName, posn.name);
			}
			posns[posn.name] = posn;
		}
		m_racks[rackName] = posns;
	}
}

void sampleChanger::loadSlotDefs(TiXmlHandle &hRoot)
{
	m_slots.clear();
	
	for( TiXmlElement* pElem=hRoot.FirstChild("slots").FirstChild().Element(); pElem; pElem=pElem->NextSiblingElement())
	{
		slotData slot;
		std::string slotName = pElem->Attribute("name");
		slot.name = slotName;
		
		if ( pElem->QueryDoubleAttribute("x", &slot.x)!=TIXML_SUCCESS ) {
			printf("sampleChanger: unable to read slot x attribute \"%s\"\n", slotName);
		}
		if ( pElem->QueryDoubleAttribute("y", &slot.y)!=TIXML_SUCCESS ) {
			printf("sampleChanger: unable to read slot y attribute \"%s\"\n", slotName);
		}
		
		m_slots[slotName] = slot;
		
		//printf("Def slot %s\n", slot.name);
	}
}

void sampleChanger::loadSlotDetails(const char* fname) 
{
	TiXmlDocument doc(fname);
	if (!doc.LoadFile()) {
		printf("sampleChanger: Unable to open slot details file \"%s\"\n", fname);
		return;
	}

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	pElem=hDoc.FirstChildElement().Element();
	if (!pElem) {
		printf("sampleChanger: Unable to parse slot details file \"%s\"\n", fname);
		return;
	}

	// save this for later
	hRoot=TiXmlHandle(pElem);

	loadSlotDetails(hRoot);
}

void sampleChanger::loadSlotDetails(TiXmlHandle &hRoot)
{
	//printf("Slot details\n");
	for( TiXmlElement* pElem=hRoot.FirstChild().Element(); pElem; pElem=pElem->NextSiblingElement())
	{
		std::string slotName = pElem->Attribute("name");
		
		std::map<std::string, slotData>::iterator iter = m_slots.find(slotName);
		if ( iter==m_slots.end() ) {
			printf("sampleChanger: Unknown slot '%s' in slot details\n", slotName);
		}
		else {
			iter->second.rackType = pElem->Attribute("rack_type");
			
			if ( pElem->QueryDoubleAttribute("xoff", &(iter->second.xoff))!=TIXML_SUCCESS ) {
				printf("sampleChanger: unable to read slot xoff attribute \"%s\"\n", slotName);
			}
			if ( pElem->QueryDoubleAttribute("yoff", &(iter->second.yoff))!=TIXML_SUCCESS ) {
				printf("sampleChanger: unable to read slot yoff attribute \"%s\"\n", slotName);
			}
		
			//printf("Det slot %s %s %f\n", iter->second.name, iter->second.rackType, iter->second.xoff);
		}
	}
}

void sampleChanger::createLookup() 
{
	const char *fnameIn = getenv("SLOT_DETAILS_FILE");
	if ( fnameIn==NULL ) {
		printf("Environment variable SLOT_DETAILS_FILE not set\n");
		return;
	}
	loadSlotDetails(fnameIn);

	const char *fnameOut = getenv("SAMPLE_LKUP_FILE");
	if ( fnameOut==NULL ) {
		printf("Environment variable SAMPLE_LKUP_FILE not set\n");
		return;
	}
	FILE *fpOut = fopen(fnameOut, "w");
	if ( fpOut==NULL ) {
		printf("Unable to open %s\n", fnameOut);
		return;
	}
	
	createLookup(fpOut);
	
	fclose(fpOut);
}

void sampleChanger::createLookup(FILE *fpOut) 
{
	//printf("Create Lookup\n");
	for ( std::map<std::string, slotData>::iterator it=m_slots.begin() ; it!=m_slots.end() ; it++ ) {
		slotData &slot = it->second;
		//printf("Create Lookup %s\n", slot.name);
		std::map<std::string, std::map<std::string, samplePosn> >::iterator iter = m_racks.find(slot.rackType);
		if ( iter==m_racks.end() ) {
			printf("sampleChanger: Unknown rack type '%s' of slot %s\n", slot.rackType, slot.name);
		}
		else {
			for ( std::map<std::string, samplePosn>::iterator it2 = iter->second.begin() ; it2!=iter->second.end() ; it2++ ) {
				fprintf(fpOut, "%s_%s %f %f\n", slot.name, it2->second.name, it2->second.x+slot.x+slot.xoff, it2->second.y+slot.y+slot.yoff);
			}
		}
	}
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

