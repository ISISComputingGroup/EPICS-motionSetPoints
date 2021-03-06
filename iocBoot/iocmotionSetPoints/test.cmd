#!../../bin/windows-x64/motionSetPoints

## You may have to change motionSetPoints to something else
## everywhere it appears in this file

< envPaths

epicsEnvSet "LOOKUP_FILE_NAME" "test.txt"

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/motionSetPoints.dbd"
motionSetPoints_registerRecordDeviceDriver pdbbase

## Load record instances
dbLoadRecords("db/setPoint.db","P=$(MYPVPREFIX)LKUP:TEST:,NP=10,TARGET_PV1=$(MYPVPREFIX)MTR01,TARGET_RBV1=$(MYPVPREFIX)MTR01.RBV,TARGET_DONE=$(MYPVPREFIX)MTR01.DMOV,TOL=1,LOOKUP=LOOKUP_FILE_NAME")

cd ${TOP}/iocBoot/${IOC}
iocInit

## Start any sequence programs
#seq sncxxx,"user=sjb99183Host"
