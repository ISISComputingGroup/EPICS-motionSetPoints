/* Utility functions for setpoint lookup */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <map>
#include <set>
#include <string>

#include <errlog.h>
#include <epicsMutex.h>
#include <epicsGuard.h>
#include <epicsString.h>

#include <epicsExport.h>

#include "setPoint.hpp"

#define ROW_LEN 200

class FileIO : public FileIOInterface {
public:
    virtual FILE* Open(const char* filename, const char* mode) {
        return fopen(filename, mode);
    }

    virtual char* Read(char *str, int n, FILE *stream) {
        return fgets(str, n, stream);
    }

    virtual int Close(FILE* file) {
        return fclose(file);
    }
};


// Global map of lookup tables, keyed by their environment vars
static std::map<const std::string, LookupTable> gTables;
static epicsMutex g_lock;
static std::map<const std::string, int> g_numCoords;

// implementation of std::less<> for caseless string comparison
struct CaselessCompare {
    bool operator() (const std::string& s1, const std::string& s2) const {
        return (epicsStrCaseCmp(s1.c_str(), s2.c_str()) < 0 ? true : false);
	}
};

/* Load the lookup file if it is not already loaded */
void checkLoadFile(const char* env_fname) {
	std::string key(env_fname);
	if ( gTables.count(key)==0 ) {
		loadDefFile(env_fname);
	}
}

/* Load the lookup file named by the environment variable */
void loadDefFile(const char* env_fname) {
    FileIO fileIO;
	const char *fname = getenv(env_fname);
	if ( fname==NULL ) {
		errlogSevPrintf(errlogMajor, "motionSetPoints: Environment variable \"%s\" not set\n", env_fname);
		return;
	}
	loadFile(&fileIO, fname, env_fname);
}

// Find the table structure for a key
LookupTable& getTable(const char *env_fname) {
	std::string key(env_fname);
    epicsGuard<epicsMutex> _lock(g_lock); // need to protect map as this may create entry	
	LookupTable& table = gTables[key];
	return table;
}

// Load a lookup file
// Arguments
//   FileIOInterface *fileIO    [in] Interface to interact with the file system
//   const char *fname          [in] File to load
//   const char *env_fname      [in] Key to identify file
void loadFile(FileIOInterface *fileIO, const char *fname, const char *env_fname) {
	std::set<std::string, CaselessCompare> read_names; // for checking uniqueness
	FILE *fptr = fileIO->Open(fname, "rt");
	int rowCount = 0;
	char buff[ROW_LEN];
	if ( fptr==NULL ) {
		errlogSevPrintf(errlogMajor, "motionSetPoints: Unable to open lookup file \"%s\"\n", fname);
		return;
	}
	LookupTable& table = getTable(env_fname);
    epicsGuard<epicsMutex> _lock(g_lock); // need to protect write access to map	
	table.rows.clear();
	
    setNumCoords(env_fname, 0);
    int numCoords = 0;
	while ( fileIO->Read(buff, ROW_LEN, fptr) ) {
		if ( buff[0]!='#' ) {
			LookupRow row;
            double x, y;
			int count = sscanf(buff, "%39s %lf %lf", row.name, &x, &y);
            row.coordinates.push_back(x);
            row.coordinates.push_back(y);
			if ( count<2 ) {
				errlogSevPrintf(errlogMajor, "motionSetPoints: Error parsing %s line %d: %s\n", fname, table.rows.size()+1, buff);
				fileIO->Close(fptr);
				table.rows.clear();
				return;
			}
            else if ( numCoords==0 ) {
                numCoords = count - 1;
            }
            else if ( numCoords!=count-1 ) {
				errlogSevPrintf(errlogMajor, "motionSetPoints: Inconsistent column count in %s line %d\n", fname, table.rows.size()+1);
                fileIO->Close(fptr);
				table.rows.clear();
				return;
            }
			if (read_names.count(row.name) != 0)
			{
				errlogSevPrintf(errlogMajor, "motionSetPoints: duplicate name \"%s\" in %s line %d\n", row.name, fname, table.rows.size()+1);
                fileIO->Close(fptr);
				table.rows.clear();
				return;
			}
			for(int i = 0; i < table.rows.size(); ++i)
			{
				if (row.coordinates[0] == table.rows[i].coordinates[0] && row.coordinates[1] == table.rows[i].coordinates[1])
				{
					errlogSevPrintf(errlogMajor, "motionSetPoints: duplicate coordinates for name \"%s\" in %s line %d\n", row.name, fname, table.rows.size()+1);
                    fileIO->Close(fptr);
					table.rows.clear();
					return;
				}
			}
			table.rows.push_back(row);
			read_names.insert(row.name);
		}
	}
    fileIO->Close(fptr);
    
    setNumCoords(env_fname, numCoords);

	if ( table.rows.size()==0 ) {
		errlogSevPrintf(errlogMinor, "motionSetPoints: Lookup file %s contains no rows\n", fname);
	} else {
	    errlogSevPrintf(errlogInfo, "motionSetPoints: Table %s, %d rows\n", env_fname, table.rows.size());
	}
}

/* Get the position for a name - also sets the requested position row pointer
 * Arguments:
//   const char *name      [in] Name to look up
//   const char *env_fname [in] Key to identify file
 * Return:
 *   int - 0=OK, -1=Name not found
 */
int name2posn(const char *name, const char* env_fname) {
	LookupTable& table = getTable(env_fname);
    table.pRowRBV = NULL;
	for ( std::vector<LookupRow>::iterator it = table.rows.begin() ; it != table.rows.end() ; ++it ) {
		if ( epicsStrCaseCmp(name, it->name) == 0 ) {
			table.pRowRBV = &(*it);
		}
	}
	return (table.pRowRBV != NULL ? 0 : -1);
}

// return position, -1 if not found
int getPositionIndexByName(const char* name, const char* env_fname)
{
	LookupTable& table = getTable(env_fname);
    for(int i=0; i<table.rows.size(); ++i)
    {
		if ( epicsStrCaseCmp(name, table.rows[i].name) == 0 ) {
			return i;
		}
	}
	return -1;
}


/* Get the name that best corresponds to the current position, sets the current position row pointer
 * Arguments:
 *   double  x    [in]  Coord
 *   double  tol  [in]  Tolerence for match
//   const char *env_fname [in] Key to identify file
 */
int posn2name(double x, double tol, const char* env_fname, double& pos_diff) {
	double best = tol;
	LookupTable &table = getTable(env_fname);

	table.pRowCurr = NULL;
	for ( std::vector<LookupRow>::iterator it = table.rows.begin() ; it != table.rows.end() ; ++it ) {
		double diff = fabs(x - it->coordinates[0]);
		if ( diff < best ) {
			best = diff;
			table.pRowCurr = &(*it);
		}
	}
	if (table.pRowCurr != NULL) {
	    pos_diff = best;
		return 0;
	} else {
		return -1;
	}
}

// Get the name that best corresponds to the current position 
int posn2name(double x, double y, double tol, const char* env_fname, double& pos_diff) {
	double best = tol * tol;
	LookupTable& table = getTable(env_fname);

	table.pRowCurr = NULL;
	for ( std::vector<LookupRow>::iterator it=table.rows.begin() ; it!=table.rows.end() ; it++ ) {
		double diff1 = fabs(x - it->coordinates[0]);
		double diff2 = fabs(y - it->coordinates[1]);
		double diffsq = diff1 * diff1 + diff2 * diff2;
		if ( diffsq < best ) {
			best = diffsq;
			table.pRowCurr = &(*it);
		}
	}
	if (table.pRowCurr != NULL) {
	    pos_diff = sqrt(best);
		return 0;
	} else {
		return -1;
	}
}

// Return the requested coordinate
// Arguments:
//         int   bFirst    [in] Whether to return the 1st coord (x)
//   const char *env_fname [in] Key to identify file
// Return:
//   0 if found and position set, -1 if not
//   
int getPosn(int bFirst, int isRBV, const char* env_fname, double& position) {
	LookupTable& table = getTable(env_fname);

	LookupRow *pRow = (isRBV ? table.pRowRBV : table.pRowCurr);
	if ( pRow == NULL ) {
		return -1;
	}
	else {
		position = bFirst ? pRow->coordinates[0] : pRow->coordinates[1];
		return 0;
	}
}

// Return the position name
// Arguments:
//         char *target    [out] Char array to which to write
//         int   isRBV     [in]  Whether to return readback (or current)
//   const char *env_fname [in]  Key to identify file
// Return: 0
int getPosnName(char *target, int isRBV, const char* env_fname) {
	LookupTable &table = getTable(env_fname);

	LookupRow *pRow = (isRBV ? table.pRowRBV : table.pRowCurr);
	if ( pRow==NULL ) {
		strcpy(target, "");
		return -1;
	}
	else {
		strncpy(target, pRow->name, NAME_LEN);
	    return 0;
	}
}

// Return a list of available positions
// Arguments:
//         char *target    [out] Char array to which to write
//         int   elem_size [in]  target is treated as a 2D array with each string of length elem_size
//   const char *env_fname [in]  Key to identify file
// Return: 0
int getPositions(char *target, int elem_size, int max_count, const char* env_fname) {
	LookupTable &table = getTable(env_fname);
	
	// Initialise to spaces because nulls confuse caget and python
	memset(target, ' ', elem_size*max_count);
	
	int count = 0;
	for ( std::vector<LookupRow>::iterator it = table.rows.begin() ; it != table.rows.end() ; ++it ) {
		if ( count==max_count ) {
			errlogSevPrintf(errlogMajor, "motionSetPoints: Unable to return all positions\n");
			break;
		}
		size_t len = strlen(it->name);
		if ( len>elem_size ) {
			len = elem_size;
		}
		memcpy(target + elem_size*count, it->name, len);
		count++;
	}
	
	if ( count<max_count ) {
		// Need to append an end marker because NORD is not being passed by the gateway
		strcpy(target + elem_size*count, "END");

		// Have to include END in the count, or it is lost
		count++;
	}
	
	return count;
}


std::string getPositionByIndex(int pos, const char* env_fname) 
{
	LookupTable &table = getTable(env_fname);
	
    if ( pos >= 0 && pos < table.rows.size() )
    {
        return table.rows[pos].name;
    }
    else
    {
        return "";
    }
}

size_t numPositions(const char* env_fname)
{
	LookupTable &table = getTable(env_fname);
    return table.rows.size();
 }

// Return the number of coordinates in the current lookup
// Arguments:
//   const char *env_fname [in]  Key to identify file
int getNumCoords(const char *env_fname) {
                std::string key(env_fname);
                epicsGuard<epicsMutex> _lock(g_lock); // need to protect map as this may create entry             
                int numCoords = g_numCoords[key];
                return numCoords;
}

// Set the number of coordinates in the current lookup
// Arguments:
//   const char *env_fname [in]  Key to identify file
//         int   numCoords [in]  Number of coordinates
void setNumCoords(const char *env_fname, int numCoords) {
                std::string key(env_fname);
                epicsGuard<epicsMutex> _lock(g_lock); // need to protect map as this may create entry             
                g_numCoords[key] = numCoords;
}
