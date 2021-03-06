#!../../bin/windows-x64/motionSetPoints

## You may have to change motionSetPoints to something else
## everywhere it appears in this file

< envPaths

epicsEnvSet "LOOKUPFILE1" "sample_x.txt"

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/motionSetPoints.dbd"
motionSetPoints_registerRecordDeviceDriver pdbbase

## Load record instances
dbLoadRecords("db/setPoint.db","P=$(MYPVPREFIX)LKUP:SAMPX:,NP=100,TARGET_PV1=$(MYPVPREFIX)MTR03,TARGET_RBV1=$(MYPVPREFIX)MTR03.RBV,TARGET_DONE=$(MYPVPREFIX)MTR03.DMOV,FILTER_SRC=$(MYPVPREFIX)LKUP:SAMPY:FILTER:OUT,TOL=1,LOOKUP=LOOKUPFILE1")

cd ${TOP}/iocBoot/${IOC}
iocInit

## Start any sequence programs
#seq sncxxx,"user=sjb99183Host"
