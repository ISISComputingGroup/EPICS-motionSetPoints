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
#include <callback.h>
#include <recSup.h>

/* Reset - use by RESET PV */
static long reset_read_ai(aiRecord *pai);
static long init_reset_record(struct aiRecord *pai);

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
  init_reset_record,
  NULL,
  reset_read_ai,
  NULL
};
epicsExportAddress(dset,devAiReset);

/* Asynchronous callback for RESET record */
static void resetCallback(CALLBACK *pcallback)
{
	struct dbCommon *precord;
	struct rset *prset;
	struct aiRecord *pai;

	callbackGetUser(precord,pcallback);
	
	pai = (struct aiRecord *)precord;
	loadDefFile(pai->inp.value.instio.string);

	prset=(struct rset *)(precord->rset);
	dbScanLock(precord);
	(*prset->process)(precord);
	dbScanUnlock(precord);
}

/* Add the asynchronous callback for RESET record */
static long init_reset_record(struct aiRecord *pai)
{
	CALLBACK *pcallback;
	pcallback = (CALLBACK *)(calloc(1,sizeof(CALLBACK)));
	callbackSetCallback(resetCallback, pcallback);
	callbackSetUser(pai,pcallback);
	pai->dpvt = (void *)pcallback;

	return 0;
}

/* Perform a reset - ie reload the lookup table and recalculate where we are */
static long reset_read_ai(aiRecord *pai)
{
	CALLBACK *pcallback = (CALLBACK *)pai->dpvt;
	if(pai->pact) {
		/*printf("Completed asynchronous processing: %s\n",pai->name);*/
		pai->pact = FALSE;
		return 0;
	}
	else {
		/*printf("Starting asynchronous processing: %s\n",pai->name);*/
		pai->pact = TRUE;
		callbackRequest(pcallback);
		return 0;
	}
};

/* Asynchronous callback for POSN:SP record */
static void stringoutCallback(CALLBACK *pcallback)
{
	struct dbCommon *precord;
	struct rset *prset;
	struct stringoutRecord *pso;

	callbackGetUser(precord,pcallback);
	
	pso = (struct stringoutRecord *)precord;
	checkLoadFile(pso->out.value.instio.string);

	prset=(struct rset *)(precord->rset);
	dbScanLock(precord);
	(*prset->process)(precord);
	dbScanUnlock(precord);
}

/* SoSetPoint - Used by FILTER1, FILTER2 and POSN:SP PVs */
static long posnSp_write_stringout(stringoutRecord *pso);
static long init_posnSp_record(stringoutRecord *pso);

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
  init_posnSp_record,
  NULL,
  posnSp_write_stringout,
  NULL
};
epicsExportAddress(dset,devSoSetPoint);

/* Register asynchronous callback for POSN:SP record */
static long init_posnSp_record(stringoutRecord *pso)
{
	CALLBACK *pcallback;
	pcallback = (CALLBACK *)(calloc(1,sizeof(CALLBACK)));
	callbackSetCallback(stringoutCallback, pcallback);
	callbackSetUser(pso,pcallback);
	pso->dpvt = (void *)pcallback;

	return 0;
}

static long posnSp_write_stringout(stringoutRecord *pso)
{
	int status;
	
	CALLBACK *pcallback = (CALLBACK *)pso->dpvt;
	if(pso->pact) {
		/* Set the new position */
		/*printf("Looking up %s\n", pso->val);*/
		status = name2posn(pso->val, pso->out.value.instio.string);

		/*printf("Completed asynchronous processing: %s\n",pso->name);*/
	
		pso->pact = FALSE;
		return 0;
	}
	else if ( strstr(pso->name, "POSN:SP")!=NULL ) {
		/*printf("Starting asynchronous processing: %s\n",pso->name);*/
		pso->pact = TRUE;
		callbackRequest(pcallback);
		return 0;
	}
	else {
		/* FILTER1 or FILTER2 - trivial operation - do synchronously */
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

/* Asynchronous callback for COORD1:RBV record */
static void aoCallback(CALLBACK *pcallback)
{
	struct dbCommon *precord;
	struct rset *prset;
	struct aoRecord *pao;

	callbackGetUser(precord,pcallback);
	
	pao = (struct aoRecord *)precord;
	checkLoadFile(pao->out.value.instio.string);

	prset=(struct rset *)(precord->rset);
	dbScanLock(precord);
	(*prset->process)(precord);
	dbScanUnlock(precord);
}

/* CoordRbv - Used for COORD1:RBV - ie the current position of the motor */
static long coordRbv_write_ao(aoRecord *pao);
static long init_coordRbv_record(aoRecord *pao);

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
  init_coordRbv_record,
  NULL,
  coordRbv_write_ao,
  NULL
};
epicsExportAddress(dset,devAoCoordRbv);

/* Register asynchronous callback for COORD1:RBV record */
static long init_coordRbv_record(aoRecord *pao)
{
	CALLBACK *pcallback;
	pcallback = (CALLBACK *)(calloc(1,sizeof(CALLBACK)));
	callbackSetCallback(aoCallback, pcallback);
	callbackSetUser(pao,pcallback);
	pao->dpvt = (void *)pcallback;

	return 0;
}

static long coordRbv_write_ao(aoRecord *pao)
{
	int status;
	
	CALLBACK *pcallback = (CALLBACK *)pao->dpvt;
	if(pao->pact) {
		/*printf("Completed asynchronous processing: %s\n",pao->name);*/
	
		status = posn2name(pao->val, 1e10, pao->out.value.instio.string);

		pao->pact = FALSE;
		return status;
	}
	else {
		/*printf("Starting asynchronous processing: %s\n",pao->name);*/
		pao->pact = TRUE;
		callbackRequest(pcallback);
		return 0;
	}
};

/* Asynchronous callback for POSN, POSN:SP:RBV and FILTER:OUT record */
static void siCallback(CALLBACK *pcallback)
{
	struct dbCommon *precord;
	struct rset *prset;
	struct stringinRecord *psi;

	callbackGetUser(precord,pcallback);
	
	psi = (struct stringinRecord *)precord;
	checkLoadFile(psi->inp.value.instio.string);

	prset=(struct rset *)(precord->rset);
	dbScanLock(precord);
	(*prset->process)(precord);
	dbScanUnlock(precord);
}

/* SiSetPoint - used for POSN, POSN:SP:RBV and FILTER:OUT */
static long setPoint_read_stringin(stringinRecord *psi);
static long init_stringin_record(stringinRecord *psi);

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
  init_stringin_record,
  NULL,
  setPoint_read_stringin,
  NULL
};
epicsExportAddress(dset,devSiSetPoint);

/* Register asynchronous callback for POSN, POSN:SP:RBV and FILTER:OUT */
static long init_stringin_record(stringinRecord *psi)
{
	CALLBACK *pcallback;
	pcallback = (CALLBACK *)(calloc(1,sizeof(CALLBACK)));
	callbackSetCallback(siCallback, pcallback);
	callbackSetUser(psi,pcallback);
	psi->dpvt = (void *)pcallback;

	return 0;
}

static long setPoint_read_stringin(stringinRecord *psi)
{
	int status;
	CALLBACK *pcallback = (CALLBACK *)psi->dpvt;
	
	if(psi->pact) {
		/*printf("Completed asynchronous processing: %s\n",psi->name);*/
		if (strstr(psi->name, "FILTER:OUT")!=NULL ) {
			/* Return the output filter for the requested position */
			status = getFilterOut(psi->val, psi->inp.value.instio.string);
		}
		else {
			/* Return the name of the requested (POSN:SP:RBV) or current position (POSN) */
			status = getPosnName(psi->val, strstr(psi->name, ":RBV")!=NULL, psi->inp.value.instio.string);
		}
		psi->pact = FALSE;
		return status;
	}
	else {
		/*printf("Starting asynchronous processing: %s\n",psi->name);*/
		psi->pact = TRUE;
		callbackRequest(pcallback);
		return 0;
	}
}

/* Asynchronous callback for POSITIONS record */
static void wfCallback(CALLBACK *pcallback)
{
	struct dbCommon *precord;
	struct rset *prset;
	struct waveformRecord *pwf;

	callbackGetUser(precord,pcallback);
	
	pwf = (struct waveformRecord *)precord;
	checkLoadFile(pwf->inp.value.instio.string);

	prset=(struct rset *)(precord->rset);
	dbScanLock(precord);
	(*prset->process)(precord);
	dbScanUnlock(precord);
}

/* Positions - used for POSITIONS - ie a list of available position names, given the current filters */
static long positions_read_wf(waveformRecord *pwf);
static long init_wf_record(waveformRecord *pwf);

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
  init_wf_record,
  NULL,
  positions_read_wf,
  NULL
};
epicsExportAddress(dset,devWfPositions);

/* Register asynchronous callback for POSITIONS */
static long init_wf_record(waveformRecord *pwf)
{
	CALLBACK *pcallback;
	pcallback = (CALLBACK *)(calloc(1,sizeof(CALLBACK)));
	callbackSetCallback(wfCallback, pcallback);
	callbackSetUser(pwf,pcallback);
	pwf->dpvt = (void *)pcallback;

	return 0;
}

static long positions_read_wf(waveformRecord *pwf)
{
	CALLBACK *pcallback = (CALLBACK *)pwf->dpvt;
	if(pwf->pact) {
		pwf->nord = getPositions((char *)pwf->bptr, MAX_STRING_SIZE, pwf->nelm, pwf->inp.value.instio.string);
		pwf->pact = FALSE;
		/*printf("Completed asynchronous processing: %s\n",pwf->name);*/
		return 0;
	}
	else {
		/*printf("Starting asynchronous processing: %s\n",pwf->name);*/
		pwf->pact = TRUE;
		callbackRequest(pcallback);
		return 0;
	}
}
