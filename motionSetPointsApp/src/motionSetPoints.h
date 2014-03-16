#ifndef MOTIONSETPOINTS_H
#define MOTIONSETPOINTS_H
 
#include "asynPortDriver.h"

class motionSetPoints : public asynPortDriver 
{
public:
    motionSetPoints(const char* portName, const char* fileName);
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
	virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual);
                 
private:
    std::string m_fileName;

    int P_positions; // string
    int P_posnSPRBV; // string
    int P_posnSP; // string
    int P_posn; // string
    int P_coord1; // double
    int P_coord1RBV;  // double
    int P_reset; // double
    int P_filter1; // string
    int P_filter2; // string
    int P_filterout; // string
#define FIRST_MSP_PARAM P_positions
#define LAST_MSP_PARAM P_filterout    

};

#define NUM_MSP_PARAMS (&LAST_MSP_PARAM - &FIRST_MSP_PARAM + 1)

#define P_positionsString	"POSITIONS"
#define P_posnSPRBVString	"POSNSPRBV"
#define P_posnSPString	    "POSNSP"
#define P_posnString	    "POSN"
#define P_coord1String	    "COORD1"
#define P_coord1RBVString	"COORD1RBV"
#define P_resetString	    "RESET"
#define P_filter1String	    "FILTER1"
#define P_filter2String	    "FILTER2"
#define P_filteroutString	"FILTEROUT"

#endif /* MOTIONSETPOINTS_H */
