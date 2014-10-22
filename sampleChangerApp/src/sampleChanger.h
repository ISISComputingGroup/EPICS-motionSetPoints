#ifndef SAMPLECHANGER_H
#define SAMPLECHANGER_H
 
#include "asynPortDriver.h"
#include <string>
#include <map>

#define TIXML_USE_STL 
#include "tinyxml.h"

struct samplePosn
{
	std::string name;
	double x;
	double y;
};

struct slotData
{
	std::string name;
	double x;
	double y;
	std::string rackType;
	double xoff;
	double yoff;
};

class sampleChanger : public asynPortDriver 
{
public:
    sampleChanger(const char* portName, const char* fileName);
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
	virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual);
                 
private:
    std::string m_fileName;
	std::map<std::string, std::map<std::string, samplePosn> > m_racks;
	
	std::map<std::string, slotData> m_slots;
	
	double m_outval;
	
	void loadDefRackDefs(const char* env_fname);
	void loadRackDefs(const char* fname);
	void loadRackDefs(TiXmlHandle &hRoot);
	void loadSlotDefs(TiXmlHandle &hRoot);
	void loadSlotDetails(const char* fname);
	void loadSlotDetails(TiXmlHandle &hRoot);
	void createLookup();
	void createLookup(FILE *fpOut);

	int P_recalc; // string
	int P_outval; // string
#define FIRST_MSP_PARAM P_recalc
#define LAST_MSP_PARAM P_outval    
};

#define NUM_MSP_PARAMS (&LAST_MSP_PARAM - &FIRST_MSP_PARAM + 1)

#define P_recalcString		"RECALC"
#define P_outvalString	"OUTVAL"

#endif /* SAMPLECHANGER_H */
