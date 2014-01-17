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
static long reset_read_ai(aiRecord *pao);

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

static long reset_read_ai(aiRecord *pao)
{
	loadDefFile();
#ifdef LKU_DEBUG
	printf("Reset Done!\n");
#endif
	return 0;
};

/* PosnSp */
static long posnSp_write_stringout(stringoutRecord *pao);

struct {
  long num;
  DEVSUPFUN  report;
  DEVSUPFUN  init;
  DEVSUPFUN  init_record;
  DEVSUPFUN  get_ioint_info;
  DEVSUPFUN  write_stringout;
  DEVSUPFUN  special_linconv;
} devSoPosnSp = {
  6, /* space for 6 functions */
  NULL,
  NULL,
  NULL,
  NULL,
  posnSp_write_stringout,
  NULL
};
epicsExportAddress(dset,devSoPosnSp);

static long posnSp_write_stringout(stringoutRecord *pao)
{
	int status;
	
	checkLoadFile();
	
#ifdef LKU_DEBUG
	printf("Looking up %s\n", pao->val);
#endif	
	
	status = name2posn(pao->val, &gXSP, &gYSP, &gRowRBV);
	
#ifdef LKU_DEBUG
	printf("Got %f %f\n", gXSP, gYSP, gRowCurr);
#endif
	return status;
}

/* Coord */
static long coord_read_ai(aiRecord *pao);

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

static long coord_read_ai(aiRecord *pao)
{
	size_t i;

	i = strlen(pao->name);
	if ( pao->name[i-1]=='1' ) {
		pao->rval = gXSP;
	}
	else {
		pao->rval = gYSP;
	}

#ifdef LKU_DEBUG
	printf("Returning y %f\n", pao->rval);
#endif
	return 0;
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
	
	checkLoadFile();
	
	status = posn2name(&gRowCurr, pao->val, 1e10);
	
#ifdef LKU_DEBUG
	printf("Got %d\n", gRowCurr);
#endif	
	return status;
};

/* PosnSpRbv */
static long posnSpRbv_read_stringin(stringinRecord *pao);

struct {
  long num;
  DEVSUPFUN  report;
  DEVSUPFUN  init;
  DEVSUPFUN  init_record;
  DEVSUPFUN  get_ioint_info;
  DEVSUPFUN  read_stringin;
  DEVSUPFUN  special_linconv;
} devSiPosnSpRbv = {
  6, /* space for 6 functions */
  NULL,
  NULL,
  NULL,
  NULL,
  posnSpRbv_read_stringin,
  NULL
};
epicsExportAddress(dset,devSiPosnSpRbv);

static long posnSpRbv_read_stringin(stringinRecord *pao)
{
	int row;
	
	checkLoadFile();
	
	if (strstr(pao->name, ":RBV")!=NULL ) {
		row = gRowRBV;
	}
	else {
		row = gRowCurr;
	}
	
#ifdef LKU_DEBUG
	printf("gRowRBV=%d gRowCurr=%d getting=%d\n", gRowRBV, gRowCurr, row);
#endif
	if ( row<0 ) {
		strcpy(pao->val, "Unknown");
	}
	else {
		strcpy(pao->val, gpRows[row].name);
	}
	
	return 0;
}

/* Positions */
static long positions_read_wf(waveformRecord *pao);

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

static long positions_read_wf(waveformRecord *pao)
{
	int i;
	int entries;
	
	checkLoadFile();

	entries = gNumRows;
	if ( gNumRows>pao->nelm ) {
		fprintf(stderr, "Too many positions for POSITIONS PV (%d). Increase $(NP) (%d)\n", gNumRows, pao->nelm);
		entries = pao->nelm;
	}
	
	for ( i=0 ; i<entries ; i++ ) {
		strcpy((char *)pao->bptr + MAX_STRING_SIZE*i, gpRows[i].name);
	}
	
#ifdef LKU_DEBUG
	printf("Copied %d bptr=%x val=%x\n", entries, pao->bptr, pao->val);
#endif
	
	pao->nord = entries;
	
	return 0;
}
