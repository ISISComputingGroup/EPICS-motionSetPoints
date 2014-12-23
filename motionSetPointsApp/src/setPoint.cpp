/* Utility functions for setpoint lookup */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <map>
#include <string>

#include <errlog.h>
#include <epicsMutex.h>
#include <epicsGuard.h>
#include <epicsString.h>

#include <epicsExport.h>

#include "setPoint.hpp"

#define ROW_LEN 200

// Global map of lookup tables, keyed by their environment vars
static std::map<const std::string, LookupTable> gTables;
static epicsMutex g_lock;
static std::map<const std::string, int> g_numCoords;

/* Load the lookup file if it is not already loaded */
void checkLoadFile(const char* env_fname) {
	std::string key(env_fname);
	if ( gTables.count(key)==0 ) {
		loadDefFile(env_fname);
	}
}

/* Load the lookup file named by the environment variable */
void loadDefFile(const char* env_fname) {
	const char *fname = getenv(env_fname);
	if ( fname==NULL ) {
		errlogPrintf("motionSetPoints: Environment variable \"%s\" not set\n", env_fname);
		return;
	}
	loadFile(fname, env_fname);
}

// Find the table structure for a key
LookupTable& getTable(const char *env_fname) {
	std::string key(env_fname);
    epicsGuard<epicsMutex> _lock(g_lock); // need to protect map as this may create entry	
	LookupTable& table = gTables[key];
	if ( table.rows.size()==0 ) {
		printf("motionSetPoints: Table %s is empty\n", env_fname);
	}
	return table;
}

// Load a lookup file
// Arguments
//   const char *fname     [in] File to load
//   const char *env_fname [in] Key to identify file
void loadFile(const char *fname, const char *env_fname) {
	FILE *fptr = fopen(fname, "rt");
	int rowCount = 0;
	char buff[ROW_LEN];
	if ( fptr==NULL ) {
		errlogPrintf("motionSetPoints: Unable to open lookup file \"%s\"\n", fname);
		return;
	}
	//printf("Creating table: %s[%s]\n", env_fname, fname);
	LookupTable& table = getTable(env_fname);
    epicsGuard<epicsMutex> _lock(g_lock); // need to protect write access to map	
	table.rows.clear();
	
    int numCoords = 0;
	while ( fgets(buff, ROW_LEN, fptr) ) {
		if ( buff[0]!='#' ) {
			LookupRow row;
			int count = sscanf(buff, "%39s %lf %lf", row.name, &row.x, &row.y);
			if ( count<2 ) {
				errlogPrintf("motionSetPoints: Error parsing %s line %d\n", fname, table.rows.size()+1);
				return;
			}
            else if ( numCoords==0 ) {
                numCoords = count - 1;
            }
            else if ( numCoords!=count-1 ) {
				errlogPrintf("motionSetPoints: Inconsistent column count %s line %d\n", fname, table.rows.size()+1);
				return;
            }
			table.rows.push_back(row);
			//printf("Row: %s\n", buff);
		}
	}
    
    setNumCoords(env_fname, numCoords);

	if ( table.rows.size()==0 ) {
		errlogPrintf("motionSetPoints: Lookup file %s contains no rows\n", fname);
		return;
	}
	printf("motionSetPoints: Table %s, %d rows\n", env_fname, table.rows.size());
	
	fclose(fptr);
}

/* Get the position for a name 
 * Arguments:
//   const char *name      [in] Name to look up
//   const char *env_fname [in] Key to identify file
 * Return:
 *   int - 0=OK, 1=Name not found
 */
int name2posn(const char *name, const char* env_fname) {
	LookupTable &table = getTable(env_fname);

	for ( std::vector<LookupRow>::iterator it=table.rows.begin() ; it!=table.rows.end() ; it++ ) {
		if ( strcmp(name, it->name)==0 ) {
			table.pRowRBV = &(*it);
			return 0;
		}
	}
	return 1;
}

/* Get the name that best corresponds to the current position 
 * Arguments:
 *   double  x    [in]  Coord
 *   double  tol  [in]  Tolerence for match
//   const char *env_fname [in] Key to identify file
 */
int posn2name(double x, double tol, const char* env_fname) {
	double best = tol;
	LookupTable &table = getTable(env_fname);

	table.pRowCurr = NULL;
	for ( std::vector<LookupRow>::iterator it=table.rows.begin() ; it!=table.rows.end() ; it++ ) {
		double diff = fabs(x - it->x);
		if ( diff<best ) {
			best = diff;
			table.pRowCurr = &(*it);
			//printf("Set c %s\n", table.pRowCurr->name);
		}
	}
	return 0;
}

int posn2name(double x, double y, double tol, const char* env_fname) {
	double best = tol*tol;
	LookupTable &table = getTable(env_fname);

	table.pRowCurr = NULL;
	for ( std::vector<LookupRow>::iterator it=table.rows.begin() ; it!=table.rows.end() ; it++ ) {
		double diff1 = fabs(x - it->x);
		double diff2 = fabs(y - it->y);
		double diff = diff1*diff1+diff2*diff2;
		if ( diff<best ) {
			best = diff;
			table.pRowCurr = &(*it);
			//printf("Set c %s\n", table.pRowCurr->name);
		}
	}
	return 0;
}

// Return the requested coordinate
// Arguments:
//         int   bFirst    [in] Whether to return the 1st coord (x)
//   const char *env_fname [in] Key to identify file
// Return:
//   double - the coordinate (or 0)
double currentPosn(int bFirst, const char* env_fname) {
	LookupTable &table = getTable(env_fname);

	if ( table.pRowRBV==NULL ) {
		//printf("Coord %s %d None\n", env_fname, bFirst);
		return 0;
	}
	else {
		//printf("Coord %s %d %s %f %f\n", env_fname, bFirst, table.pRowRBV->name, table.pRowRBV->x, table.pRowRBV->y);
		return bFirst ? table.pRowRBV->x : table.pRowRBV->y;
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

	LookupRow *pRow = isRBV ? table.pRowRBV : table.pRowCurr;
	if ( pRow==NULL ) {
		strcpy(target, "Unknown");
	}
	else {
		strncpy(target, pRow->name, NAME_LEN);
	}
	return 0;
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
	for ( std::vector<LookupRow>::iterator it=table.rows.begin() ; it!=table.rows.end() ; it++ ) {
		if ( count==max_count ) {
			errlogPrintf("motionSetPoints: Unable to return all positions\n");
			break;
		}
		int len = strlen(it->name);
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

// Return the number of coordinates in the current lookup
// Arguments:
//   const char *env_fname [in]  Key to identify file
int getNumCoords(const char *env_fname) {
                std::string key(env_fname);
                epicsGuard<epicsMutex> _lock(g_lock); // need to protect map as this may create entry             
                int numCoords = g_numCoords[key];
                if ( numCoords==0 ) {
                                printf("motionSetPoints: Table %s has zero coords\n", env_fname);
                }
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
