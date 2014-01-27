/* Setpoint lookup PVs */
#include "setPoint.h"

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

/* Reset - use by RESET PV */
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

/* Perform a reset - ie reload the lookup table and recalculate where we are */
static long reset_read_ai(aiRecord *pai)
{
	loadDefFile(pai->inp.value.instio.string);
#ifdef LKU_DEBUG
	printf("Reset Done!\n");
#endif
	return 0;
};

/* SoSetPoint - Used by FILTER1, FILTER2 and POSN:SP PVs */
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
		/* Set the new position */
		checkLoadFile(pso->out.value.instio.string);
	
		/*printf("Looking up %s\n", pso->val);*/
		status = name2posn(pso->val, pso->out.value.instio.string);
	}
	else {
		/* FILTER1 or FILTER2 */
		/*printf("Setting %s to %s\n", pao->name, pao->val);*/
		status = setFilter(pso->name, pso->val, pso->out.value.instio.string);
	}
	
	return status;
}

/* Coord - Used for COORD1 - ie to return the coordinate for the requested position name */
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
	pai->val = currentPosn(pai->name[i-1]=='1', pai->inp.value.instio.string);

#ifdef LKU_DEBUG
	printf("Returning y %f\n", pai->val);
#endif
	return 2;
};

/* CoordRbv - Used for COORD1:RBV - ie the current position of the motor */
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
	
	status = posn2name(pao->val, 1e10, pao->out.value.instio.string);
	
	return status;
};

/* SiSetPoint - used for POSN, POSN:SP:RBV and FILTER:OUT */
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
	checkLoadFile(psi->inp.value.instio.string);
	
	if (strstr(psi->name, "FILTER:OUT")!=NULL ) {
		/* Return the output filter for the requested position */
		return getFilterOut(psi->val, psi->inp.value.instio.string);
	}
	else {
		/* Return the name of the requested (POSN:SP:RBV) or current position (POSN) */
		return getPosnName(psi->val, strstr(psi->name, ":RBV")!=NULL, psi->inp.value.instio.string);
	}
}

/* Positions - used for POSITIONS - ie a list of available position names, given the current filters */
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
	checkLoadFile(pwf->inp.value.instio.string);

	pwf->nord = getPositions((char *)pwf->bptr, MAX_STRING_SIZE, pwf->nelm, pwf->inp.value.instio.string);
	
	return 0;
}
