/* Utility functions for setpoint lookup */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <numeric>

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
    virtual void Open(const char* filename) {
        m_file.open(filename);
    }

    virtual bool ReadLine(std::string &str) {
        return (bool)std::getline(m_file, str);
    }

    virtual void Close() {
        m_file.close();
    }

    virtual bool isOpen() {
        return m_file.is_open();
    }

private:
    std::ifstream m_file;
};


// Global map of lookup tables, keyed by their environment vars
static std::map<const std::string, LookupTable> gTables;
static epicsMutex g_lock;

// implementation of std::less<> for caseless string comparison
struct CaselessCompare {
    bool operator() (const std::string& s1, const std::string& s2) const {
        return (epicsStrCaseCmp(s1.c_str(), s2.c_str()) < 0 ? true : false);
	}
};

/* Load the lookup file named by the environment variable.
   Arguments:
     const char *env_fname              [in] The environment variable that contains the path to the file
            int  expectedNumberOfCoords [in] The number of coordinates expected in the file
 */
void loadDefFile(const char* env_fname, int expectedNumberOfCoords) {
    FileIO fileIO;
	const char *fname = getenv(env_fname);
	if ( fname==NULL ) {
		errlogSevPrintf(errlogMajor, "motionSetPoints: Environment variable \"%s\" not set\n", env_fname);
		return;
	}
	loadFile(&fileIO, fname, env_fname, expectedNumberOfCoords);
}

/* Get the table structure for a key.
   Arguments:
     const char *env_fname [in] Key to identify file
   Return:
     A reference to the lookup table
 */
LookupTable& getTable(const char *env_fname) {
	std::string key(env_fname);
    epicsGuard<epicsMutex> _lock(g_lock); // need to protect map as this may create entry	
	LookupTable& table = gTables[key];
	return table;
}

/* Create a table row from a line in the lookup file.
   Arguments:
     string fileLine [in] Line from file
   Return:
     The created row
 */
LookupRow createRowFromFileLine(std::string fileLine) {
    LookupRow row;
    std::stringstream ss(fileLine);
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> vstrings(begin, end);
    std::strcpy(row.name, vstrings[0].c_str());
    std::transform(std::next(vstrings.begin()), vstrings.end(), std::back_inserter(row.coordinates),
        [](std::string const& val) {return std::stod(val); });
    return row;
}

/* Load a lookup file
   Arguments:
     FileIOInterface *fileIO                  [in] Interface to interact with the file system
     const char      *fname                   [in] File to load
     const char      *env_fname               [in] Key to identify file
     int              expectedNumberOfCoords  [in] Expected number of coordinates
 */
void loadFile(FileIOInterface *fileIO, const char *fname, const char *env_fname, int expectedNumberOfCoords) {
	std::set<std::string, CaselessCompare> read_names; // for checking uniqueness
	fileIO->Open(fname);
	int rowCount = 0;
	std::string line;
	if ( !fileIO->isOpen() ) {
		errlogSevPrintf(errlogMajor, "motionSetPoints: Unable to open lookup file \"%s\"\n", fname);
		return;
	}
	LookupTable& table = getTable(env_fname);
    epicsGuard<epicsMutex> _lock(g_lock); // need to protect write access to map	
	table.rows.clear();

	while ( fileIO->ReadLine(line) ) {
		if ( line.rfind('#', 0) != 0 && line.length() > 0 ) {
			LookupRow row = createRowFromFileLine(line);
            int numberOfCoordsInLine = row.coordinates.size();
			if (numberOfCoordsInLine == 0) {
				errlogSevPrintf(errlogMajor, "motionSetPoints: Error parsing %s line %d: %s\n", fname, table.rows.size()+1, line);
				fileIO->Close();
				table.rows.clear();
				return;
			}
            if ( numberOfCoordsInLine != expectedNumberOfCoords) {
				errlogSevPrintf(errlogMajor, "motionSetPoints: Unexpected number of columns in %s line %d\n", fname, table.rows.size()+1);
                fileIO->Close();
				table.rows.clear();
				return;
            }
			if (read_names.count(row.name) != 0)
			{
				errlogSevPrintf(errlogMajor, "motionSetPoints: duplicate name \"%s\" in %s line %d\n", row.name, fname, table.rows.size()+1);
                fileIO->Close();
				table.rows.clear();
				return;
			}
			for(int i = 0; i < table.rows.size(); ++i)
			{
                bool rows_same = true;
                for (int j = 0; j < row.coordinates.size(); ++j) {
                    rows_same &= row.coordinates[j] == table.rows[i].coordinates[j];
                }
				if (rows_same)
				{
					errlogSevPrintf(errlogMajor, "motionSetPoints: duplicate coordinates for name \"%s\" in %s line %d\n", row.name, fname, table.rows.size()+1);
                    fileIO->Close();
					table.rows.clear();
					return;
				}
			}
			table.rows.push_back(row);
			read_names.insert(row.name);
		}
	}
    fileIO->Close();

	if ( table.rows.size()==0 ) {
		errlogSevPrintf(errlogMinor, "motionSetPoints: Lookup file %s contains no rows\n", fname);
	} else {
	    errlogSevPrintf(errlogInfo, "motionSetPoints: Table %s, %d rows\n", env_fname, table.rows.size());
	}
}

/* Set the row RBV to the specified position, if the position exists.
   Arguments:
     const char *name      [in] Name to look up
     const char *env_fname [in] Key to identify file
   Return:
     int - 0=OK, -1=Name not found
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

/* Return the index of the position with the given name
   Arguments:
     const char *name      [in]  The name of the position to search for
     const char *env_fname [in]  Key to identify file
   Return:
         position index if found, -1 if none found
 */
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

/* Get the name that best corresponds to the given position and sets the current position row pointer to this
   Arguments:
     vector<double>  searchCoords    [in]  The coordinates to find the position for
     double          tol             [in]  Tolerence for match
     const char *    env_fname       [in]  Key to identify file
     double          pos_diff        [out] The difference between the searched for coordinates and the best match
   Return:
     0 if found, -1 if not
 */
int posn2name(std::vector<double> searchCoords, double tol, const char* env_fname, double& pos_diff) {
    double best = tol * tol;
    LookupTable& table = getTable(env_fname);

    table.pRowCurr = NULL;
    for (std::vector<LookupRow>::iterator it = table.rows.begin(); it != table.rows.end(); it++) {
        std::vector<double> squaredDiffs;
        std::transform(it->coordinates.begin(), it->coordinates.end(), searchCoords.begin(), std::back_inserter(squaredDiffs), 
                       [](double val, double searchVal) -> double {return pow(searchVal - val, 2);});
        double totalDiff = std::accumulate(squaredDiffs.begin(), squaredDiffs.end(), 0.0);

        if (totalDiff < best) {
            best = totalDiff;
            table.pRowCurr = &(*it);
        }
    }
    if (table.pRowCurr != NULL) {
        pos_diff = sqrt(best);
        return 0;
    }
    else {
        return -1;
    }
}

/* Return the requested coordinate for the current or readback row.
   Arguments:
           int   coordinate [in] Which co-ordinate to get
           bool   isRBV     [in] if true requesting the readback, else the current row
     const char *env_fname  [in] Key to identify file
        double   position   [out] The position of the requested co-ordinate
   Return:
     0 if found and position set, -1 if not
 */   
int getPosn(int coordinate, bool isRBV, const char* env_fname, double& position) {
	LookupTable& table = getTable(env_fname);
	LookupRow *pRow = (isRBV ? table.pRowRBV : table.pRowCurr);
	if ( pRow == NULL ) {
		return -1;
	}
	else {
		position = pRow->coordinates[coordinate];
		return 0;
	}
}

/* Return the position name
   Arguments:
           char *target    [out] Char array to which to write
           bool  isRBV     [in]  True to return readback, False to return current
     const char *env_fname [in]  Key to identify file
   Return: 
     0 if found, -1 if not
 */
int getPosnName(char *target, bool isRBV, const char* env_fname) {
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

/* Return a list of available positions
   Arguments:
    std::string *target    [out] String to write the available positions into
     const char *env_fname [in]  Key to identify file
 */
void getPositions(std::string *target, const char* env_fname) {
	LookupTable &table = getTable(env_fname);

	for ( std::vector<LookupRow>::iterator it = table.rows.begin() ; it != table.rows.end() ; ++it ) {
        std::string name = std::string(it->name);
        name += std::string(40-name.size(), ' ');
        target->append(name);
	}
}

/* Return the name of the position at the given index
   Arguments:
            int pos        [in]  The index of the position that you want the name for
     const char *env_fname [in]  Key to identify file
   Return:
         string, name of the position at requested index 
 */
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

/* Return the number of positions available
   Arguments:
     const char *env_fname [in]  Key to identify file
   Return:
     size_t, the number of positions
 */
size_t numPositions(const char* env_fname)
{
	LookupTable &table = getTable(env_fname);
    return table.rows.size();
}
