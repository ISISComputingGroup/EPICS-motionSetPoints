#!../../bin/windows-x64/motionSetPoints

## You may have to change motionSetPoints to something else
## everywhere it appears in this file

< envPaths

epicsEnvSet "SETTINGS" "$(ICPSETTINGSDIR)/$(ICPCONFIGHOST)/motionSetPoints"

#epicsEnvSet "LOOKUPFILE1" "monitor3.txt"
#epicsEnvSet "LOOKUPFILE2" "monitor4.txt"
#epicsEnvSet "LOOKUPFILE3" "sample_x.txt"
#epicsEnvSet "LOOKUPFILE4" "sample_y.txt"

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/motionSetPoints.dbd"
motionSetPoints_registerRecordDeviceDriver pdbbase

## Load record instances
#dbLoadRecords("db/setPoint.db","P=$(MYPVPREFIX)LKUP:MON3:,TARGET_PV1=$(MYPVPREFIX)MTR01,TARGET_RBV1=$(MYPVPREFIX)MTR01.RBV,TARGET_DONE=$(MYPVPREFIX)MTR01.DMOV,TOL=1,LOOKUP=LOOKUPFILE1")
#dbLoadRecords("db/setPoint.db","P=$(MYPVPREFIX)LKUP:MON4:,TARGET_PV1=$(MYPVPREFIX)MTR02,TARGET_RBV1=$(MYPVPREFIX)MTR02.RBV,TARGET_DONE=$(MYPVPREFIX)MTR02.DMOV,TOL=1,LOOKUP=LOOKUPFILE2")
#dbLoadRecords("db/setPoint.db","P=$(MYPVPREFIX)LKUP:SAMPX:,TARGET_PV1=$(MYPVPREFIX)MTR03,TARGET_RBV1=$(MYPVPREFIX)MTR03.RBV,TARGET_DONE=$(MYPVPREFIX)MTR03.DMOV,FILTER_SRC=$(MYPVPREFIX)LKUP:SAMPY:FILTER:OUT,TOL=1,LOOKUP=LOOKUPFILE3")
#dbLoadRecords("db/setPoint.db","P=$(MYPVPREFIX)LKUP:SAMPY:,TARGET_PV1=$(MYPVPREFIX)MTR04,TARGET_RBV1=$(MYPVPREFIX)MTR04.RBV,TARGET_DONE=$(MYPVPREFIX)MTR04.DMOV,TOL=1,LOOKUP=LOOKUPFILE4")

< $(SETTINGS)/local.cmd

cd ${TOP}/iocBoot/${IOC}
iocInit

## Start any sequence programs
#seq sncxxx,"user=sjb99183Host"
