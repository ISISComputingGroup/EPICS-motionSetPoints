#ifndef SAMPLECHANGER_H
#define SAMPLECHANGER_H
 
#include <string>

#include "asynPortDriver.h"
#include "converter.h"

class sampleChanger : public asynPortDriver 
{
public:
    sampleChanger(const char* portName, const char* fileName);
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
	virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual);
                 
private:
	double m_outval;
	//converter m_converter;
    std::string m_fileName;

	int P_recalc; // string
	int P_outval; // string
#define FIRST_MSP_PARAM P_recalc
#define LAST_MSP_PARAM P_outval    
};

#define NUM_MSP_PARAMS (&LAST_MSP_PARAM - &FIRST_MSP_PARAM + 1)

#define P_recalcString		"RECALC"
#define P_outvalString	"OUTVAL"

#endif /* SAMPLECHANGER_H */
