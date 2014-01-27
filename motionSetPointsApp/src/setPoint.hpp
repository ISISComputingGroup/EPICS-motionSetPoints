#ifndef SETPOINT_HPP
#define SETPOINT_HPP 1

#include <string>
#include <vector>

extern "C" {
#include "setPoint.h"
}

#define NAME_LEN 40

// A row in the lookup file
typedef struct LookupRow {
	char name[NAME_LEN];	// Position name
	double x;				// Coordinate
	double y;				// y coord - optional, not used yet
	char filter[NAME_LEN];	// Output filter, optional
} LookupRow;

// A lookup table
typedef struct LookupTable {
	std::vector<LookupRow> rows;	// The rows

	LookupRow *pRowRBV;		// The requested row
	LookupRow *pRowCurr;	// The current row

	std::string filter1;	// Filters applied to position names
	std::string filter2;
	
	int checkFilters(const char *name);
	
	LookupTable() : pRowRBV(NULL), pRowCurr(NULL) {}
} LookupTable;

void loadFile(const char *fname, const char *env_fname);
LookupTable &getTable(const char *env_fname);

#endif
