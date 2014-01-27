#ifndef SETPOINT_HPP
#define SETPOINT_HPP 1

#include <string>
#include <vector>

extern "C" {
#include "setPoint.h"
}

#define NAME_LEN 40

typedef struct LookupRow {
	char name[NAME_LEN];
	double x;
	double y;
	char filter[NAME_LEN];
} LookupRow;

typedef struct LookupTable {
	//int numRows;
	std::vector<LookupRow> rows;

	//double xSP;
	//double ySP;
	LookupRow *pRowRBV;
	LookupRow *pRowCurr;

	std::string filter1;
	std::string filter2;
	
	int checkFilters(const char *name);
	
	LookupTable() : pRowRBV(NULL), pRowCurr(NULL) {}
} LookupTable;

void loadFile(const char *fname, const char *env_fname);

#endif
