#ifndef CONVERTER_H
#define CONVERTER_H
 
#include <map>
#include <string>

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

class converter
{
public:
	converter();
	virtual ~converter() {};
	void createLookup();
	
private:
	std::map<std::string, std::map<std::string, samplePosn> > m_racks;
	
	std::map<std::string, slotData> m_slots;

	void loadDefRackDefs(const char* env_fname);
	void loadRackDefs(const char* fname);
	void loadRackDefs(TiXmlHandle &hRoot);
	void loadSlotDefs(TiXmlHandle &hRoot);
	void loadSlotDetails(const char* fname);
	void loadSlotDetails(TiXmlHandle &hRoot);
	void createLookup(FILE *fpOut);
};

#endif /* CONVERTER_H */