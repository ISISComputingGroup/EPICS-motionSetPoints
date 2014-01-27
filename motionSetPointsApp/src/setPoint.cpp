/* Utility functions for setpoint lookup */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <map>
#include <string>

#include "setPoint.hpp"

#define ROW_LEN 200

// Global map of lookup tables, keyed by their environment vars
std::map<const std::string, LookupTable> gTables;

/* Load the lookup file if it is not already loaded */
void checkLoadFile(const char* env_fname) {
	std::string key(env_fname);
	if ( gTables.count(key)==0 ) {
		loadDefFile(env_fname);
	}
}

/* Load the lookup file named by the environment variable */
void loadDefFile(const char* env_fname) {
	char *fname = getenv(env_fname);
	if ( fname==NULL ) {
		fprintf(stderr, "Environment variable %s not set\n", env_fname);
		exit(1);
	}
	loadFile(fname, env_fname);
}

// Find the table structure for a key
LookupTable &getTable(const char *env_fname) {
	std::string key(env_fname);
	LookupTable &table = gTables[key];
	if ( table.rows.size()==0 ) {
		printf("Table %s is empty\n", env_fname);
	}
	return table;
}

// Load a lookup file
// Arguments
//   const char *fname     [in] File to load
//   const char *env_fname [in] Key to identify file
void loadFile(const char *fname, const char *env_fname) {
	FILE *fptr = fopen(fname, "r");
	int rowCount = 0;
	char buff[ROW_LEN];
	if ( fptr==NULL ) {
		fprintf(stderr, "Unable to open lookup file %s\n", fname);
		exit(1);
	}
	
	//printf("Creating table: %s[%s]\n", env_fname, fname);
	LookupTable &table = getTable(env_fname);
	table.rows.clear();
	
	while ( fgets(buff, ROW_LEN, fptr) ) {
		if ( buff[0]!='#' ) {
			LookupRow row;
			int count = sscanf(buff, "%39s %lf %lf %s39", row.name, &row.x, &row.y, row.filter);
			if ( count<2 ) {
				fprintf(stderr, "Error parsing %s line %d\n", fname, table.rows.size()+1);
				exit(1);
			}
			table.rows.push_back(row);
			//printf("Row: %s\n", buff);
		}
	}

	if ( table.rows.size()==0 ) {
		fprintf(stderr, "Lookup file %s contains no rows\n", fname);
		exit(1);
	}
	printf("Table %s, %d rows\n", env_fname, table.rows.size());
	
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
		if ( table.checkFilters(name)==0 && strcmp(name, it->name)==0 ) {
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
		if ( table.checkFilters(it->name)==0 ) {
			double diff = fabs(x - it->x);
			if ( diff<best ) {
				best = diff;
				table.pRowCurr = &(*it);
				//printf("Set c %s\n", table.pRowCurr->name);
			}
		}
	}
	return 0;
}

// Set a filter
// Arguments:
//   const char *name      [in] The name of the filter (FILTER1 or FILTER2)
//   const char *value     [in] The value to set
//   const char *env_fname [in] Key to identify file
//
int setFilter(const char *name, const char *value, const char* env_fname) {
	LookupTable &table = getTable(env_fname);
	std::string &filter = strstr(name, "FILTER1") ? table.filter1 : table.filter2;

	if ( strlen(value)==0 || value[0]=='*' ) {
		filter = "";
	}
	else {
		filter = value;
	}
	
	return 0;
}

// Check a position name against a filter
// Arguments:
//   const char  *name   [in] Position name to check
//   std::string  filter [in] The filter
// Return:
//   0 if matches. 1 if not
int checkFilter(const char *name, std::string filter) {
	if ( filter.length()==0 ) {
		/* No filter */
		//printf("No filter 0\n");
		return 0;
	}
	else if ( filter.find("|")==std::string::npos ) {
		const char *ptr = filter.c_str();
		//printf("Simple %s==%s %d %d\n", filter, name, strncmp(ptr, name, strlen(ptr)), strlen(ptr));
		return strncmp(filter.c_str(), name, filter.length());
	}
	else {
		//printf("Complex\n");
		/* Filter has several parts */
		char *copy;
		char *p1;
		char *part;
		int ret = 1; /* Assume no part matches */
	
		copy = strdup(filter.c_str());
		if ( copy==NULL ) {
			fprintf(stderr, "Out of memory\n");
			exit(1);
		}
		p1 = copy;
		while ( (part = strtok(p1, "|"))!=NULL ) {
			if ( strncmp(part, name, strlen(part))==0 ) {
				/* Got a match */
				ret = 0;
				break;
			}
			p1 = NULL; /* Continue same tokenisation */
		}
		free(copy);
		
		return ret;
	}
}

// Check whether name passes the filters
// Arguments:
//   const char  *name   [in] Position name to check
// Return:
//   0=passes
int LookupTable::checkFilters(const char *name) {
	return checkFilter(name, filter1) || checkFilter(name, filter2);
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

// Return the output filter
// Arguments:
//         char *target    [out] Char array to which to write
//   const char *env_fname [in]  Key to identify file
// Return 0
int getFilterOut(char *target, const char *env_fname) {
	LookupTable &table = getTable(env_fname);

	if ( table.pRowRBV==NULL ) {
		target[0] = '\0';
	}
	else {
		strncpy(target, table.pRowRBV->filter, NAME_LEN);
	}
	return 0;
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
	
	int count = 0;
	for ( std::vector<LookupRow>::iterator it=table.rows.begin() ; it!=table.rows.end() ; it++ ) {
		if ( count==max_count ) {
			fprintf(stderr, "Unable to return all positions\n");
			break;
		}
		if ( table.checkFilters(it->name)==0 ) {
			strncpy(target + elem_size*count, it->name, NAME_LEN);
			count++;
		}
	}
	
	if ( count<max_count ) {
		// Need to append an end marker because NORD is not being passed by the gateway
		strcpy(target + elem_size*count, "END");
		// Have to include END in the count, or it is lost
		count++;
	}
	
	return count;
}
