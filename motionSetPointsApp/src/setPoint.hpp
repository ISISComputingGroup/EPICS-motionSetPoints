#ifndef SETPOINT_HPP
#define SETPOINT_HPP 1

#include <string>
#include <vector>

#define NAME_LEN 40

class FileIOInterface {
    // An interface for interacting with the file system so that we can mock the interaction
public:
    virtual FILE* Open(const char* filename, const char* mode) = 0;
    virtual char* Read(char *str, int n, FILE *stream) = 0;
    virtual int Close(FILE* file) = 0;
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

void loadFile(FileIOInterface *fileIO, const char *fname, const char *env_fname);
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
