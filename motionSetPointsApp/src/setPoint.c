/* Utility functions for setpoint lookup */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "setPoint.h"

#define ROW_LEN 200

int gNumRows = 0;	/* Number of lookup rows. 0=File not loaded */
LookupRow *gpRows = NULL;	/* Lookup rows */

double gXSP;	/* X position for selected row */
double gYSP;	/* Y position for selected row */
int gRowRBV = -1;	/* Row requested */
int gRowCurr = -1;	/* Row closest to current position */

char gFilter1[NAME_LEN];
char gFilter2[NAME_LEN];

/* Load the lookup file if it is not already loaded */
void checkLoadFile() {
	if ( gNumRows==0 ) {
		loadDefFile();
	}
}

/* Load the lookup file named by the environment variable */
void loadDefFile() {
	char *fname = getenv(ENV_FNAME);
	if ( fname==NULL ) {
		fprintf(stderr, "Environment variable %s not set\n", ENV_FNAME);
		exit(1);
	}
	loadFile(fname);
}

/* Load a lookup file */
void loadFile(const char *fname) {
	FILE *fptr = fopen(fname, "r");
	int rowCount = 0;
	char buff[ROW_LEN];
	if ( fptr==NULL ) {
		fprintf(stderr, "Unable to open lookup file %s\n", fname);
		exit(1);
	}
	
	while ( fgets(buff, ROW_LEN, fptr) ) {
		if ( buff[0]!='#' ) {
			rowCount++;
		}
	}
	
	if ( rowCount==0 ) {
		fprintf(stderr, "Lookup file %s contains no rows\n", fname);
		exit(1);
	}
	
	if ( gpRows!=NULL ) {
		free(gpRows);
	}
	if ( (gpRows = (LookupRow*)calloc(rowCount, sizeof(LookupRow)))==NULL ) {
		fprintf(stderr, "Unable to allocate memory\n", fname);
		exit(1);
	}
	
	rewind(fptr);
	gNumRows = 0;
	while ( fgets(buff, ROW_LEN, fptr) ) {
		if ( buff[0]!='#' ) {
			int count = sscanf(buff, "%39s %lf %lf %s39", gpRows[gNumRows].name, &gpRows[gNumRows].x, 
									&gpRows[gNumRows].y, gpRows[gNumRows].filter);
			gNumRows++;
			if ( count<2 ) {
				fprintf(stderr, "Error parsing %s line %d\n", fname, gNumRows);
				exit(1);
			}
		}
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
int name2posn(const char *name, double *pX, double *pY, int *row) {
	int i;
	for ( i=0 ; i<gNumRows ; i++ ) {
		if ( checkFilters(gpRows[i].name)==0 && strcmp(name, gpRows[i].name)==0 ) {
			*pX = gpRows[i].x;
			*pY = gpRows[i].y;
			*row = i;
#ifdef LKU_DEBUG
			printf("Set %d\n", i);
#endif
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
int posn2name(int *pRow, double x, double tol) {
	int i;
	double best = tol;
#ifdef LKU_DEBUG
	printf("Reverse lookup %f (%f)\n", x, tol);
#endif
	*pRow = -1;
	for ( i=0 ; i<gNumRows ; i++ ) {
		if ( checkFilters(gpRows[i].name)==0 ) {
			double diff = fabs(x - gpRows[i].x);
			if ( diff<best ) {
				best = diff;
				*pRow = i;
			}
		}
	}
#ifdef LKU_DEBUG
	printf("Got %d\n", *pRow);
#endif
	return 0;
}

int setFilter(const char *name, const char *value) {
	char *target;
	if ( strstr(name, "FILTER1") ) {
		target = &gFilter1[0];
	}
	else {
		target = &gFilter2[0];
	}
	
	if ( strlen(value)==0 || value[0]=='*' ) {
		target[0] = '\0';
	}
	else {
		strncpy(target, value, NAME_LEN);
	}
	
	return 0;
}

int checkFilter(const char *name, const char *filter) {
	/*printf("Filter=(%s)\n", filter);*/
	if ( filter[0]=='\0' ) {
		/* No filter */
		return 0;
	}
	else if ( strstr(filter, "|")==NULL ) {
		return strncmp(filter, name, strlen(filter));
	}
	else {
		/* Filter has several parts */
		char *copy;
		char *p1;
		char *part;
		int ret = 1; /* Assume no part matches */
	
		copy = strdup(filter);
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

int checkFilters(const char *name) {
	return checkFilter(name, gFilter1) || checkFilter(name, gFilter2);
}
