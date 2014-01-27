/* Utility functions for setpoint lookup */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <map>
#include <string>

#include "setPoint.hpp"

#define ROW_LEN 200

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

/* Load a lookup file */
void loadFile(const char *fname, const char *env_fname) {
	FILE *fptr = fopen(fname, "r");
	int rowCount = 0;
	char buff[ROW_LEN];
	if ( fptr==NULL ) {
		fprintf(stderr, "Unable to open lookup file %s\n", fname);
		exit(1);
	}
	
	//printf("Creating table: %s[%s]\n", env_fname, fname);
	
	std::string key(env_fname);
	LookupTable &table = gTables[key];
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
	//printf("size %d\n", table.rows.size());
	if ( table.rows.size()==0 ) {
		fprintf(stderr, "Lookup file %s contains no rows\n", fname);
		exit(1);
	}
	
	fclose(fptr);
}



/* Get the position for a name 
 * Arguments:
 *   const char   *name [in]  Name to look up
 *         double *pX   [out] X coord
 *         double *pY   [out] Y coord
 *		   double *row  [out] Selected row. Unchanged if not found
 * Return:
 *   int - 0=OK, 1=Name not found
 */
int name2posn(const char *name, const char* env_fname) {
	std::string key(env_fname);
	LookupTable &table = gTables[key];
	//printf("Posn %s, Table %s, rows %d\n", name, env_fname, table.rows.size());
	for ( std::vector<LookupRow>::iterator it=table.rows.begin() ; it!=table.rows.end() ; it++ ) {
		//printf("name %s cf row %s cmp=%d\n", name, it->name, strcmp(name, it->name));
		if ( table.checkFilters(name)==0 && strcmp(name, it->name)==0 ) {
			table.pRowRBV = &(*it);
			//printf("Set rbv %s %ld %s\n", it->name, table.pRowRBV, table.pRowRBV->name);
			return 0;
		}
	}
	return 1;
}

/* Get the name that best corresponds to the current position 
 * Arguments:
 *   int    *pRow [out] Nearest row (or -1 if no match)
 *   double  x    [in]  Coord
 *   double  tol  [in]  Tolerence for match
 */
int posn2name(double x, double tol, const char* env_fname) {
	double best = tol;
	std::string key(env_fname);
	LookupTable &table = gTables[key];

	//printf("Reverse lookup %f (%f) %s\n", x, tol, env_fname);

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

int setFilter(const char *name, const char *value, const char* env_fname) {
	std::string key(env_fname);
	LookupTable &table = gTables[key];
	std::string &filter = strstr(name, "FILTER1") ? table.filter1 : table.filter2;

	if ( strlen(value)==0 || value[0]=='*' ) {
		filter = "";
	}
	else {
		filter = value;
	}
	
	return 0;
}

int checkFilter(const char *name, std::string filter) {
	//printf("Filter=(%s) cf %s\n", filter, name);
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
// Ret 0=passes
int LookupTable::checkFilters(const char *name) {
	int ret = checkFilter(name, filter1) || checkFilter(name, filter2);
	//printf("%s 1=%s 2=%s R=%d\n", name, filter1, filter2, ret);
	return ret;
}

// Return the requested coordinate
// If bFirst, return x
double currentPosn(int bFirst, const char* env_fname) {
	std::string key(env_fname);
	LookupTable &table = gTables[key];
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
int getFilterOut(char *target, const char *env_fname) {
	std::string key(env_fname);
	LookupTable &table = gTables[key];
	if ( table.pRowRBV==NULL ) {
		target[0] = '\0';
	}
	else {
		strncpy(target, table.pRowRBV->filter, NAME_LEN);
	}
	return 0;
}

// Return the position name
int getPosnName(char *target, int isRBV, const char* env_fname) {
	std::string key(env_fname);
	LookupTable &table = gTables[key];
	//printf("Get posn name %s, %d %ld %ld\n", env_fname, isRBV, table.pRowRBV, table.pRowCurr);
	LookupRow *pRow = isRBV ? table.pRowRBV : table.pRowCurr;
	if ( pRow==NULL ) {
		strcpy(target, "Unknown");
	}
	else {
		//if ( isRBV==1 ) printf("Got %d %s\n", isRBV, pRow->name);
		strncpy(target, pRow->name, NAME_LEN);
	}
	return 0;
}

// Return a list of available positions
int getPositions(char *target, int elem_size, const char* env_fname) {
	//printf("Positions for %s\n", env_fname);
	std::string key(env_fname);
	LookupTable &table = gTables[key];
	
	int count = 0;
	for ( std::vector<LookupRow>::iterator it=table.rows.begin() ; it!=table.rows.end() ; it++ ) {
		//printf("%s\n", it->name);
		if ( table.checkFilters(it->name)==0 ) {
			strncpy(target + elem_size*count, it->name, NAME_LEN);
			//printf("Posn %d\n", count);
			count++;
		}
	}
	strcpy(target + elem_size*count, "END");
	count++;
	
	return count;
}
