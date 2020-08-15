#!../../bin/windows-x64-debug/motionSetPointsTest

## You may have to change motionSetPointsTest to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/motionSetPointsTest.dbd"
motionSetPointsTest_registerRecordDeviceDriver pdbbase

## create 10 simulated motors on one controller 
motorSimCreateController("test", 10) 
motorSimConfigAxis("test", 0, 32000, -320000,  0, 0) 
motorSimConfigAxis("test", 1, 32000, -320000,  0, 0) 
motorSimConfigAxis("test", 2, 32000, -320000,  0, 0) 
motorSimConfigAxis("test", 3, 32000, -320000,  0, 0) 
motorSimConfigAxis("test", 4, 32000, -320000,  0, 0)
motorSimConfigAxis("test", 5, 32000, -320000,  0, 0) 
motorSimConfigAxis("test", 6, 32000, -320000,  0, 0) 
motorSimConfigAxis("test", 7, 32000, -320000,  0, 0) 
motorSimConfigAxis("test", 8, 32000, -320000,  0, 0) 
motorSimConfigAxis("test", 9, 32000, -320000,  0, 0) 

epicsEnvSet("LOOKUPFILE1DAXIS","${TOP}/iocBoot/${IOC}/lookup1D.txt")
motionSetPointsConfigure("LOOKUPFILE1DAXIS","LOOKUPFILE1DAXIS", 1)

epicsEnvSet("LOOKUPFILE2DAXIS","${TOP}/iocBoot/${IOC}/lookup2D.txt")
motionSetPointsConfigure("LOOKUPFILE2DAXIS","LOOKUPFILE2DAXIS", 2)

epicsEnvSet("LOOKUPFILE10DAXIS","${TOP}/iocBoot/${IOC}/lookup10D.txt")
motionSetPointsConfigure("LOOKUPFILE10DAXIS","LOOKUPFILE10DAXIS", 10)

epicsEnvSet("LOOKUPFILEDN","${TOP}/iocBoot/${IOC}/duplicate_names.txt")
motionSetPointsConfigure("LOOKUPFILEDN","LOOKUPFILEDN", 1)

epicsEnvSet("LOOKUPFILEDP","${TOP}/iocBoot/${IOC}/duplicate_positions.txt")
motionSetPointsConfigure("LOOKUPFILEDP","LOOKUPFILEDP", 2)

## Load our record instances
dbLoadRecords("${TOP}/db/motionSetPointsTest.db","P=$(MYPVPREFIX)MSP:")
dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX),AXIS=AXIS1,mAXIS=MSP:MTR0")
dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX),AXIS=AXIS2,mAXIS=MSP:MTR1")
dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX),AXIS=AXIS3,mAXIS=MSP:MTR2")
dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX),AXIS=AXIS4,mAXIS=MSP:MTR3")
dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX),AXIS=AXIS5,mAXIS=MSP:MTR4")
dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX),AXIS=AXIS6,mAXIS=MSP:MTR5")
dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX),AXIS=AXIS7,mAXIS=MSP:MTR6")
dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX),AXIS=AXIS8,mAXIS=MSP:MTR7")
dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX),AXIS=AXIS9,mAXIS=MSP:MTR8")
dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX),AXIS=AXIS10,mAXIS=MSP:MTR9")

# 1D with axis
dbLoadRecords("$(TOP)/db/motionSetPointsSingleAxis.db","P=$(MYPVPREFIX)LKUP:1DAXIS:,AXIS1=$(MYPVPREFIX)AXIS1,TOL=1,LOOKUP=LOOKUPFILE1DAXIS")

# 2D with axis
dbLoadRecords("$(TOP)/db/motionSetPointsDoubleAxis.db","P=$(MYPVPREFIX)LKUP:2DAXIS:,AXIS1=$(MYPVPREFIX)AXIS1,AXIS2=$(MYPVPREFIX)AXIS2,TOL=1,LOOKUP=LOOKUPFILE2DAXIS")

# 10D with axis
dbLoadRecords("$(TOP)/db/motionSetPoints10Axis.db","P=$(MYPVPREFIX)LKUP:10DAXIS:,AXIS1=$(MYPVPREFIX)AXIS1,AXIS2=$(MYPVPREFIX)AXIS2,AXIS3=$(MYPVPREFIX)AXIS3,AXIS4=$(MYPVPREFIX)AXIS4,AXIS5=$(MYPVPREFIX)AXIS5,AXIS6=$(MYPVPREFIX)AXIS6,AXIS7=$(MYPVPREFIX)AXIS7,AXIS8=$(MYPVPREFIX)AXIS8,AXIS9=$(MYPVPREFIX)AXIS9,AXIS10=$(MYPVPREFIX)AXIS10,TOL=1,LOOKUP=LOOKUPFILE10DAXIS")

# duplicate names
dbLoadRecords("$(TOP)/db/motionSetPointsSingleAxis.db","P=$(MYPVPREFIX)LKUP:DN:,AXIS1=$(MYPVPREFIX)AXIS1,AXIS2=$(MYPVPREFIX)AXIS2,TOL=1,LOOKUP=LOOKUPFILEDN")

# duplicate positions
dbLoadRecords("$(TOP)/db/motionSetPointsSingleAxis.db","P=$(MYPVPREFIX)LKUP:DP:,AXIS1=$(MYPVPREFIX)AXIS1,AXIS2=$(MYPVPREFIX)AXIS2,TOL=1,LOOKUP=LOOKUPFILEDP")

cd ${TOP}/iocBoot/${IOC}

iocInit
