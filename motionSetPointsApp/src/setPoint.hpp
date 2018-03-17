#ifndef SETPOINT_HPP
#define SETPOINT_HPP 1

#include <string>
#include <vector>

#define NAME_LEN 40

// A row in the lookup file
typedef struct LookupRow {
	char name[NAME_LEN];	// Position name
	double x;				// Coordinate
	double y;				// y coord - optional
	LookupRow() : x(0.0), y(0.0) { name[0] = '\0'; }
} LookupRow;

// A lookup table
typedef struct LookupTable {
	std::vector<LookupRow> rows;	// The rows

	LookupRow *pRowRBV;		// The requested row to move to
	LookupRow *pRowCurr;	// The current row we are at as we move to above requested position
	
	LookupTable() : pRowRBV(NULL), pRowCurr(NULL) {}
} LookupTable;

void loadFile(const char *fname, const char *env_fname);
LookupTable &getTable(const char *env_fname);

void checkLoadFile(const char* env_fname);
void loadDefFile(const char* env_fname);
int name2posn(const char *name, const char* env_fname);
int posn2name(double x, double tol, const char* env_fname, double& pos_diff);
int posn2name(double x, double y, double tol, const char* env_fname, double& pos_diff);
int getPosn(int bFirst, int isRBV, const char* env_fname, double& position);
int getPosnName(char *target, int isRBV, const char* env_fname);
int getPositions(char *target, int elem_size, int max_count, const char* env_fname);
int getNumCoords(const char *env_fname);
void setNumCoords(const char *env_fname, int numCoords);
std::string getPositionByIndex(int pos, const char* env_fname);
int getPositionIndexByName(const char* name, const char* env_fname);
size_t numPositions(const char* env_fname);

#endif
