/* Setpoint lookup PVs */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <epicsExport.h>
#include <dbAccess.h>
#include <devSup.h>
#include <recGbl.h>

#include <aiRecord.h>
#include <aoRecord.h>
#include <stringoutRecord.h>
#include <stringinRecord.h>
#include <waveformRecord.h>

#include "setPoint.h"

/* Reset */
static long reset_read_ai(aiRecord *pai);

struct {
  long num;
  DEVSUPFUN  report;
  DEVSUPFUN  init;
  DEVSUPFUN  init_record;
  DEVSUPFUN  get_ioint_info;
  DEVSUPFUN  read_ai;
  DEVSUPFUN  special_linconv;
} devAiReset = {
  6, /* space for 6 functions */
  NULL,
  NULL,
  NULL,
  NULL,
  reset_read_ai,
  NULL
};
epicsExportAddress(dset,devAiReset);

static long reset_read_ai(aiRecord *pai)
{
	loadDefFile(pai->inp.value.instio.string);
#ifdef LKU_DEBUG
	printf("Reset Done!\n");
#endif
	return 0;
};

/* SoSetPoint */
static long posnSp_write_stringout(stringoutRecord *pso);

struct {
  long num;
  DEVSUPFUN  report;
  DEVSUPFUN  init;
  DEVSUPFUN  init_record;
  DEVSUPFUN  get_ioint_info;
  DEVSUPFUN  write_stringout;
  DEVSUPFUN  special_linconv;
} devSoSetPoint = {
  6, /* space for 6 functions */
  NULL,
  NULL,
  NULL,
  NULL,
  posnSp_write_stringout,
  NULL
};
epicsExportAddress(dset,devSoSetPoint);

static long posnSp_write_stringout(stringoutRecord *pso)
{
	int status;
	
	if ( strstr(pso->name, "POSN:SP")!=NULL ) {
		checkLoadFile(pso->out.value.instio.string);
	
#ifdef LKU_DEBUG
		printf("Looking up %s\n", pao->val);
#endif	
	
		status = name2posn(pso->val, &gXSP, &gYSP, &gRowRBV);
	
#ifdef LKU_DEBUG
		printf("Got %f %f\n", gXSP, gYSP, gRowCurr);
#endif
	}
	else {
		/*printf("Setting %s to %s\n", pao->name, pao->val);*/
		status = setFilter(pso->name, pso->val);
	}
	
	return status;
}

/* Coord */
static long coord_read_ai(aiRecord *pai);

struct {
  long num;
  DEVSUPFUN  report;
  DEVSUPFUN  init;
  DEVSUPFUN  init_record;
  DEVSUPFUN  get_ioint_info;
  DEVSUPFUN  read_ai;
  DEVSUPFUN  special_linconv;
} devAiCoord = {
  6, /* space for 6 functions */
  NULL,
  NULL,
  NULL,
  NULL,
  coord_read_ai,
  NULL
};
epicsExportAddress(dset,devAiCoord);

static long coord_read_ai(aiRecord *pai)
{
	size_t i;

	i = strlen(pai->name);
	if ( pai->name[i-1]=='1' ) {
		pai->val = gXSP;
	}
	else {
		pai->val = gYSP;
	}

#ifdef LKU_DEBUG
	printf("Returning y %f\n", pai->val);
#endif
	return 2;
};

/* CoordRbv */
static long coordRbv_write_ao(aoRecord *pao);

struct {
  long num;
  DEVSUPFUN  report;
  DEVSUPFUN  init;
  DEVSUPFUN  init_record;
  DEVSUPFUN  get_ioint_info;
  DEVSUPFUN  write_ao;
  DEVSUPFUN  special_linconv;
} devAoCoordRbv = {
  6, /* space for 6 functions */
  NULL,
  NULL,
  NULL,
  NULL,
  coordRbv_write_ao,
  NULL
};
epicsExportAddress(dset,devAoCoordRbv);

static long coordRbv_write_ao(aoRecord *pao)
{
	int status;
	
	checkLoadFile(pao->out.value.instio.string);
	
	status = posn2name(&gRowCurr, pao->val, 1e10);
	
#ifdef LKU_DEBUG
	printf("Got %d\n", gRowCurr);
#endif	
	return status;
};

/* SiSetPoint */
static long setPoint_read_stringin(stringinRecord *psi);

struct {
  long num;
  DEVSUPFUN  report;
  DEVSUPFUN  init;
  DEVSUPFUN  init_record;
  DEVSUPFUN  get_ioint_info;
  DEVSUPFUN  read_stringin;
  DEVSUPFUN  special_linconv;
} devSiSetPoint = {
  6, /* space for 6 functions */
  NULL,
  NULL,
  NULL,
  NULL,
  setPoint_read_stringin,
  NULL
};
epicsExportAddress(dset,devSiSetPoint);

static long setPoint_read_stringin(stringinRecord *psi)
{
	int row;
	
	checkLoadFile(psi->inp.value.instio.string);
	
	if (strstr(psi->name, "FILTER:OUT")!=NULL ) {
		if ( gRowRBV>=0 ) {
			strcpy(psi->val, gpRows[gRowRBV].filter);
		}
		return 0;
	}
	if (strstr(psi->name, ":RBV")!=NULL ) {
		row = gRowRBV;
	}
	else {
		row = gRowCurr;
	}
	
#ifdef LKU_DEBUG
	printf("gRowRBV=%d gRowCurr=%d getting=%d\n", gRowRBV, gRowCurr, row);
#endif
	if ( row<0 ) {
		strcpy(psi->val, "Unknown");
	}
	else {
		strcpy(psi->val, gpRows[row].name);
	}
	
	return 0;
}

/* Positions */
static long positions_read_wf(waveformRecord *pwf);

struct {
  long num;
  DEVSUPFUN  report;
  DEVSUPFUN  init;
  DEVSUPFUN  init_record;
  DEVSUPFUN  get_ioint_info;
  DEVSUPFUN  read_wf;
  DEVSUPFUN  special_linconv;
} devWfPositions = {
  6, /* space for 6 functions */
  NULL,
  NULL,
  NULL,
  NULL,
  positions_read_wf,
  NULL
};
epicsExportAddress(dset,devWfPositions);

static long positions_read_wf(waveformRecord *pwf)
{
	int i;
	int entries;
	
	checkLoadFile(pwf->inp.value.instio.string);

	entries = 0;
	for ( i=0 ; i<gNumRows ; i++ ) {
		if ( checkFilters(gpRows[i].name)==0 ) {
			if ( entries > pwf->nelm ) {
				fprintf(stderr, "Too many positions for POSITIONS PV (%d). Increase $(NP) (%d)\n", 
									gNumRows, pwf->nelm);
				break;
			}
			/*printf("Accepted %s\n", gpRows[i].name);*/
			strcpy((char *)pwf->bptr + MAX_STRING_SIZE*entries, gpRows[i].name);
			entries++;
		}
		else {
			/*printf("Rejected %s\n", gpRows[i].name);*/
		}
	}
	
#ifdef LKU_DEBUG
	printf("Copied %d bptr=%x val=%x\n", entries, pwf->bptr, pwf->val);
#endif
	
	if ( entries<pwf->nelm ) {
		/* The gateway is not passing NORD, so need to add an end marker */
		strcpy((char *)pwf->bptr + MAX_STRING_SIZE*entries, "END");
		entries++;
	}
	
	pwf->nord = entries;
	
	return 0;
}
