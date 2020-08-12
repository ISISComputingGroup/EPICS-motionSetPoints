#!../../bin/windows-x64-debug/motionSetPointsTest

## You may have to change motionSetPointsTest to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/motionSetPointsTest.dbd"
motionSetPointsTest_registerRecordDeviceDriver pdbbase

## create 2 simulated motors on one controller 
motorSimCreateController("test", 2) 
motorSimConfigAxis("test", 0, 32000, -320000,  0, 0) 
motorSimConfigAxis("test", 1, 32000, -320000,  0, 0) 

## setpoint lookup files
epicsEnvSet("LOOKUPFILE1","${TOP}/iocBoot/${IOC}/analyser.txt")
motionSetPointsConfigure("LOOKUPFILE1","LOOKUPFILE1", 2)

epicsEnvSet("LOOKUPFILE1D","${TOP}/iocBoot/${IOC}/lookup1D.txt")
motionSetPointsConfigure("LOOKUPFILE1D","LOOKUPFILE1D", 1)

epicsEnvSet("LOOKUPFILE1DAXIS","${TOP}/iocBoot/${IOC}/lookup1D.txt")
motionSetPointsConfigure("LOOKUPFILE1DAXIS","LOOKUPFILE1DAXIS", 1)

epicsEnvSet("LOOKUPFILE2D","${TOP}/iocBoot/${IOC}/lookup2D.txt")
motionSetPointsConfigure("LOOKUPFILE2D","LOOKUPFILE2D", 2)

epicsEnvSet("LOOKUPFILE2DAXIS","${TOP}/iocBoot/${IOC}/lookup2D.txt")
motionSetPointsConfigure("LOOKUPFILE2DAXIS","LOOKUPFILE2DAXIS", 2)

epicsEnvSet("LOOKUPFILEDN","${TOP}/iocBoot/${IOC}/duplicate_names.txt")
motionSetPointsConfigure("LOOKUPFILEDN","LOOKUPFILEDN", 1)

epicsEnvSet("LOOKUPFILEDP","${TOP}/iocBoot/${IOC}/duplicate_positions.txt")
motionSetPointsConfigure("LOOKUPFILEDP","LOOKUPFILEDP", 2)

## Load our record instances
dbLoadRecords("${TOP}/db/motionSetPointsTest.db","P=$(MYPVPREFIX)MSP:")
dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX),AXIS=AXIS1,mAXIS=MSP:MTR0")
dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX),AXIS=AXIS2,mAXIS=MSP:MTR1")

# Analyser
dbLoadRecords("$(TOP)/db/motionSetPoints.db","P=$(MYPVPREFIX)LKUP:MSP:,TARGET_PV1=$(MYPVPREFIX)MSP:MTR0,TARGET_RBV1=$(MYPVPREFIX)MSP:MTR0.RBV,TARGET_PV2=$(MYPVPREFIX)MSP:MTR1,TARGET_RBV2=$(MYPVPREFIX)MSP:MTR1.RBV,TARGET_DONE=$(MYPVPREFIX)MSP:MTR0.DMOV,TARGET_DONE2=$(MYPVPREFIX)MSP:MTR1.DMOV,TOL=1,LOOKUP=LOOKUPFILE1")
dbLoadRecords("$(TOP)/db/single_axis.db","P=$(MYPVPREFIX)LKUP:MSP:,AXIS_NUM=1,TARGET_PV=$(MYPVPREFIX)MSP:MTR1,TARGET_RBV=$(MYPVPREFIX)MSP:MTR1.RBV,LOOKUP=LOOKUPFILE1")

# 1D no axis
dbLoadRecords("$(TOP)/db/motionSetPoints.db","P=$(MYPVPREFIX)LKUP:1D:,TARGET_PV1=$(MYPVPREFIX)MSP:MTR0,TARGET_RBV1=$(MYPVPREFIX)MSP:MTR0.RBV,TARGET_DONE=$(MYPVPREFIX)MSP:MTR0.DMOV,TOL=1,LOOKUP=LOOKUPFILE1D")

# 2D no axis
dbLoadRecords("$(TOP)/db/motionSetPoints.db","P=$(MYPVPREFIX)LKUP:2D:,TARGET_PV1=$(MYPVPREFIX)MSP:MTR0,TARGET_RBV1=$(MYPVPREFIX)MSP:MTR0.RBV,TARGET_PV2=$(MYPVPREFIX)MSP:MTR1,TARGET_RBV2=$(MYPVPREFIX)MSP:MTR1.RBV,TARGET_DONE=$(MYPVPREFIX)MSP:MTR0.DMOV,TARGET_DONE2=$(MYPVPREFIX)MSP:MTR1.DMOV,TOL=1,LOOKUP=LOOKUPFILE2D")
dbLoadRecords("$(TOP)/db/single_axis.db","P=$(MYPVPREFIX)LKUP:2D:,AXIS_NUM=1,TARGET_PV=$(MYPVPREFIX)MSP:MTR1,TARGET_RBV=$(MYPVPREFIX)MSP:MTR1.RBV,LOOKUP=LOOKUPFILE2D")

# 1D with axis
dbLoadRecords("$(TOP)/db/motionSetPoints.db","P=$(MYPVPREFIX)LKUP:1DAXIS:,AXIS1=$(MYPVPREFIX)AXIS1,TOL=1,LOOKUP=LOOKUPFILE1DAXIS")

# 2D with axis
dbLoadRecords("$(TOP)/db/motionSetPoints.db","P=$(MYPVPREFIX)LKUP:2DAXIS:,AXIS1=$(MYPVPREFIX)AXIS1,AXIS2=$(MYPVPREFIX)AXIS2,TOL=1,LOOKUP=LOOKUPFILE2DAXIS")
dbLoadRecords("$(TOP)/db/single_axis.db","P=$(MYPVPREFIX)LKUP:2DAXIS:,AXIS_NUM=1,AXIS=$(MYPVPREFIX)AXIS2,LOOKUP=LOOKUPFILE2DAXIS")

# duplicate names
dbLoadRecords("$(TOP)/db/motionSetPoints.db","P=$(MYPVPREFIX)LKUP:DN:,AXIS1=$(MYPVPREFIX)AXIS1,AXIS2=$(MYPVPREFIX)AXIS2,TOL=1,LOOKUP=LOOKUPFILEDN")

# duplicate positions
dbLoadRecords("$(TOP)/db/motionSetPoints.db","P=$(MYPVPREFIX)LKUP:DP:,AXIS1=$(MYPVPREFIX)AXIS1,AXIS2=$(MYPVPREFIX)AXIS2,TOL=1,LOOKUP=LOOKUPFILEDP")

cd ${TOP}/iocBoot/${IOC}

iocInit
