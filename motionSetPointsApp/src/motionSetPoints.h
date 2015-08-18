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
    std::string m_fileName; // Lookup file name - used as a key to identify this lookup instance

    int P_positions; // string
    int P_posnSPRBV; // string
    int P_iposnSPRBV; // int
    int P_posnSP; // string
    int P_iposnSP; // int
    int P_posn; // string
    int P_iposn; // int
    int P_coord1; // double
    int P_coord2; // double
    int P_coord1RBV;  // double
    int P_coord2RBV;  // double
    int P_reset; // double
    int P_numpos; // int
    int P_numAxes; // double
#define FIRST_MSP_PARAM P_positions
#define LAST_MSP_PARAM P_numAxes

	void updatePositions();
	void updateCurrPosn(double coord1, double coord2);
    int gotoPosition(const char* value);
	
	double m_coord1; // Last values of the current motor positions
	double m_coord2;
};

#define NUM_MSP_PARAMS (&LAST_MSP_PARAM - &FIRST_MSP_PARAM + 1)

#define P_positionsString	"POSITIONS"
#define P_posnSPRBVString	"POSNSPRBV"
#define P_iposnSPRBVString	"IPOSNSPRBV"
#define P_posnSPString	    "POSNSP"
#define P_iposnSPString	    "IPOSNSP"
#define P_posnString	    "POSN"
#define P_iposnString	    "IPOSN"
#define P_coord1String	    "COORD1"
#define P_coord2String	    "COORD2"
#define P_coord1RBVString	"COORD1RBV"
#define P_coord2RBVString	"COORD2RBV"
#define P_resetString	    "RESET"
#define P_numPosString      "NUMPOS"
#define P_numAxesString     "NUMAXES"

#endif /* MOTIONSETPOINTS_H */
