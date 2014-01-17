#ifndef SETPOINT_H
#define SETPOINT_H 1

#define NAME_LEN 40
#define ENV_FNAME "LOOKUP_FILE_NAME"

typedef struct LookupRow {
	char name[NAME_LEN];
	double x;
	double y;
} LookupRow;

extern int gNumRows;
extern LookupRow *gpRows;

extern double gXSP;
extern double gYSP;
extern int gRowRBV;
extern int gRowCurr;

void checkLoadFile();
void loadDefFile();
void loadFile(const char *fname);
int name2posn(const char *name, double *pX, double *pY);
int posn2name(int *pRow, double x, double tol);

#endif
