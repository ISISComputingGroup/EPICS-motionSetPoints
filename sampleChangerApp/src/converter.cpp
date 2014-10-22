#include "converter.h"

converter::converter()
{
	loadDefRackDefs("RACKDEFS");
} 

void converter::loadDefRackDefs(const char* env_fname) 
{
	const char *fname = getenv(env_fname);
	if ( fname==NULL ) {
		printf("sampleChanger: Environment variable \"%s\" not set\n", env_fname);
		return;
	}
	loadRackDefs(fname);
}

void converter::loadRackDefs(const char* fname) 
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

void converter::loadRackDefs(TiXmlHandle &hRoot)
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
				printf("sampleChanger: rack has no name attribute \"%s\"\n", rackName.c_str());
			}
			if ( pRack->QueryDoubleAttribute("x", &posn.x)!=TIXML_SUCCESS ) {
				printf("sampleChanger: unable to read x attribute \"%s\" \"%s\"\n", rackName.c_str(), posn.name.c_str());
			}
			if ( pRack->QueryDoubleAttribute("y", &posn.y)!=TIXML_SUCCESS ) {
				printf("sampleChanger: unable to read y attribute \"%s\" \"%s\"\n", rackName.c_str(), posn.name.c_str());
			}
			posns[posn.name] = posn;
		}
		m_racks[rackName] = posns;
	}
}

void converter::loadSlotDefs(TiXmlHandle &hRoot)
{
	m_slots.clear();
	
	for( TiXmlElement* pElem=hRoot.FirstChild("slots").FirstChild().Element(); pElem; pElem=pElem->NextSiblingElement())
	{
		slotData slot;
		std::string slotName = pElem->Attribute("name");
		slot.name = slotName;
		
		if ( pElem->QueryDoubleAttribute("x", &slot.x)!=TIXML_SUCCESS ) {
			printf("sampleChanger: unable to read slot x attribute \"%s\"\n", slotName.c_str());
		}
		if ( pElem->QueryDoubleAttribute("y", &slot.y)!=TIXML_SUCCESS ) {
			printf("sampleChanger: unable to read slot y attribute \"%s\"\n", slotName.c_str());
		}
		
		m_slots[slotName] = slot;
		
		//printf("Def slot %s\n", slot.name);
	}
}

void converter::loadSlotDetails(const char* fname) 
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

void converter::loadSlotDetails(TiXmlHandle &hRoot)
{
	//printf("Slot details\n");
	for( TiXmlElement* pElem=hRoot.FirstChild().Element(); pElem; pElem=pElem->NextSiblingElement())
	{
		std::string slotName = pElem->Attribute("name");
		
		std::map<std::string, slotData>::iterator iter = m_slots.find(slotName);
		if ( iter==m_slots.end() ) {
			printf("sampleChanger: Unknown slot '%s' in slot details\n", slotName.c_str());
		}
		else {
			iter->second.rackType = pElem->Attribute("rack_type");
			
			if ( pElem->QueryDoubleAttribute("xoff", &(iter->second.xoff))!=TIXML_SUCCESS ) {
				printf("sampleChanger: unable to read slot xoff attribute \"%s\"\n", slotName.c_str());
			}
			if ( pElem->QueryDoubleAttribute("yoff", &(iter->second.yoff))!=TIXML_SUCCESS ) {
				printf("sampleChanger: unable to read slot yoff attribute \"%s\"\n", slotName.c_str());
			}
		
			//printf("Det slot %s %s %f\n", iter->second.name, iter->second.rackType, iter->second.xoff);
		}
	}
}

void converter::createLookup() 
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

void converter::createLookup(FILE *fpOut) 
{
	//printf("Create Lookup\n");
	for ( std::map<std::string, slotData>::iterator it=m_slots.begin() ; it!=m_slots.end() ; it++ ) {
		slotData &slot = it->second;
		//printf("Create Lookup %s\n", slot.name);
		std::map<std::string, std::map<std::string, samplePosn> >::iterator iter = m_racks.find(slot.rackType);
		if ( iter==m_racks.end() ) {
			printf("sampleChanger: Unknown rack type '%s' of slot %s\n", slot.rackType.c_str(), slot.name.c_str());
		}
		else {
			for ( std::map<std::string, samplePosn>::iterator it2 = iter->second.begin() ; it2!=iter->second.end() ; it2++ ) {
				fprintf(fpOut, "%s_%s %f %f\n", slot.name.c_str(), it2->second.name.c_str(), it2->second.x+slot.x+slot.xoff, it2->second.y+slot.y+slot.yoff);
			}
		}
	}
}
