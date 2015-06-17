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

## setpoint lookup file
epicsEnvSet("LOOKUPFILE1","${TOP}/iocBoot/${IOC}/analyser.txt")
motionSetPointsConfigure("LOOKUPFILE1","LOOKUPFILE1")

## Load our record instances
dbLoadRecords("${TOP}/db/motionSetPointsTest.db","P=$(MYPVPREFIX)MSP:")
dbLoadRecords("$(TOP)/db/motionSetPoints.db","P=$(MYPVPREFIX)LKUP:MSP:,TARGET_PV1=$(MYPVPREFIX)MSP:MTR0,TARGET_RBV1=$(MYPVPREFIX)MSP:MTR0.RBV,TARGET_PV2=$(MYPVPREFIX)MSP:MTR1,TARGET_RBV2=$(MYPVPREFIX)MSP:MTR1.RBV,TARGET_DONE=$(MYPVPREFIX)MSP:MTR0.DMOV,TARGET_DONE2=$(MYPVPREFIX)MSP:MTR1.DMOV,TOL=1,LOOKUP=LOOKUPFILE1")

cd ${TOP}/iocBoot/${IOC}
iocInit

