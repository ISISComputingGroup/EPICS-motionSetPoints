#ifndef SETPOINT_HPP
#define SETPOINT_HPP 1

#include <string>
#include <vector>

#define NAME_LEN 40

class FileIOInterface {
    // An interface for interacting with the file system so that we can mock the interaction
public:
    virtual void Open(const char* filename) = 0;
    virtual bool ReadLine(std::string &str) = 0;
    virtual void Close() = 0;
    virtual bool isOpen() = 0;
};

// A row in the lookup file
typedef struct LookupRow {
	char name[NAME_LEN];	// Position name
	std::vector<double> coordinates;				// Coordinates
	LookupRow() { name[0] = '\0'; }
} LookupRow;

// A lookup table
typedef struct LookupTable {
	std::vector<LookupRow> rows;	// The rows

	LookupRow *pRowRBV;		// The requested row to move to
	LookupRow *pRowCurr;	// The current row we are at as we move to above requested position
	
	LookupTable() : pRowRBV(NULL), pRowCurr(NULL) {}
} LookupTable;

LookupRow    createRowFromFileLine(std::string fileLine);
void loadFile(FileIOInterface *fileIO, const char *fname, const char *env_fname, int expectedNumberOfCoords);
LookupTable &getTable(const char *env_fname);

void loadDefFile(const char* env_fname, int expectedNumberOfCoords);
int name2posn(const char *name, const char* env_fname);
int posn2name(std::vector<double> coordinates, double tol, const char* env_fname, double& pos_diff);
int getPosn(int coordinate, bool isRBV, const char* env_fname, double& position);
int getPosnName(char *target, bool isRBV, const char* env_fname);
void getPositions(std::string *target, const char* env_fname);
std::string getPositionByIndex(int pos, const char* env_fname);
int getPositionIndexByName(const char* name, const char* env_fname);
size_t numPositions(const char* env_fname);

#endif
