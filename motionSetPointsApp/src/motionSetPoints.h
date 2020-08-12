#ifndef MOTIONSETPOINTS_H
#define MOTIONSETPOINTS_H
 
#include "asynPortDriver.h"

class motionSetPoints : public asynPortDriver 
{
public:
    motionSetPoints(const char* portName, const char* fileName, int numberOfCoordinates);
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
	virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual);
                 
private:
    std::string m_fileName; // Lookup file name - used as a key to identify this lookup instance
    std::vector<double> m_currentCoordinates; // Last values of the current motor positions
	double m_tol; // tolerance to use for position match

    // Parameters for general control, only used on the 0th address
    int P_positions; // string - list of posibble positions
    int P_posnSPRBV; // string - requested position readback
    int P_iposnSPRBV; // int - index of requested position readback 
    int P_posnSP; // string - requested position
    int P_iposnSP; // int - requested position (by index)
    int P_posn; // string - current position
    int P_posnIndex; // int - index of current position
    int P_nearestPosn; // string - nearest current position
    int P_nearestPosnIndex; // int - index of nearest current position
    int P_reset; // double
    int P_numpos; // int
	int P_tol; // double
	int P_posDiff; // double - position difference (< m_tol) for current position (if there is a valid current position)
    int P_numAxes; // double

    // Parameters for each coordinate, these will be set on each of the addresses
    int P_coord; // double - current position of co-ordinate
    int P_coordRBV;  // double - requested position of co-ordinate
#define FIRST_MSP_PARAM P_positions
#define LAST_MSP_PARAM P_coordRBV

	void updateAvailablePositions();
    void updateCurrPosn();
    int gotoPosition(const char* value);
	
};

#define NUM_MSP_PARAMS (&LAST_MSP_PARAM - &FIRST_MSP_PARAM + 1)

#define P_positionsString	"POSITIONS"
#define P_posnSPRBVString	"POSNSPRBV"
#define P_iposnSPRBVString	"IPOSNSPRBV"
#define P_posnSPString	    "POSNSP"
#define P_iposnSPString	    "IPOSNSP"
#define P_posnString	    "POSN"
#define P_iposnString	    "IPOSN"
#define P_nposnString	    "NPOSN"
#define P_niposnString	    "NIPOSN"
#define P_coordString	    "COORD"
#define P_coordRBVString	"COORDRBV"
#define P_resetString	    "RESET"
#define P_numPosString      "NUMPOS"
#define P_numAxesString     "NUMAXES"
#define P_tolString         "TOL"
#define P_posDiffString     "POSDIFF"

#endif /* MOTIONSETPOINTS_H */
