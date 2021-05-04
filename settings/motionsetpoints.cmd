$(IFIOC_GALIL_01) epicsEnvSet("LOOKUPFILE1DAXIS","${MOTIONSETPOINTS}/settings/lookup1D.txt")
$(IFIOC_GALIL_01) motionSetPointsConfigure("LOOKUPFILE1DAXIS","LOOKUPFILE1DAXIS", 1)

$(IFIOC_LINMOT_01) epicsEnvSet("LINMOT_1D","${MOTIONSETPOINTS}/settings/lookup1D.txt")
$(IFIOC_LINMOT_01) motionSetPointsConfigure("LINMOT_1D","LINMOT_1D", 1)

$(IFIOC_GALIL_01) epicsEnvSet("LOOKUPFILE2DAXIS","${MOTIONSETPOINTS}/settings/lookup2D.txt")
$(IFIOC_GALIL_01) motionSetPointsConfigure("LOOKUPFILE2DAXIS","LOOKUPFILE2DAXIS", 2)

$(IFIOC_GALIL_02) epicsEnvSet("LOOKUPFILE10DAXIS","${MOTIONSETPOINTS}/settings/lookup10D.txt")
$(IFIOC_GALIL_02) motionSetPointsConfigure("LOOKUPFILE10DAXIS","LOOKUPFILE10DAXIS", 10)

$(IFIOC_GALIL_01) epicsEnvSet("LOOKUPFILEDN","${MOTIONSETPOINTS}/settings/duplicate_names.txt")
$(IFIOC_GALIL_01) motionSetPointsConfigure("LOOKUPFILEDN","LOOKUPFILEDN", 1)

$(IFIOC_GALIL_01) epicsEnvSet("LOOKUPFILEDP","${MOTIONSETPOINTS}/settings/duplicate_positions.txt")
$(IFIOC_GALIL_01) motionSetPointsConfigure("LOOKUPFILEDP","LOOKUPFILEDP", 2)

# 1D with axis
$(IFIOC_GALIL_01) dbLoadRecordsLoop("$(MOTIONSETPOINTS)/db/inPos.db","P=$(MYPVPREFIX)LKUP1D:,AXIS0=$(MYPVPREFIX)AXIS0,TOL=1,LOOKUP=LOOKUPFILE1DAXIS","NUMPOS", 0, 30)
$(IFIOC_GALIL_01) dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPointsSingleAxis.db","P=$(MYPVPREFIX)LKUP1D:,AXIS0=$(MYPVPREFIX)AXIS0,TOL=1,LOOKUP=LOOKUPFILE1DAXIS")

$(IFIOC_LINMOT_01) dbLoadRecordsLoop("$(MOTIONSETPOINTS)/db/inPos.db","P=$(MYPVPREFIX)LIN_LKUP1D:,AXIS0=$(MYPVPREFIX)LIN_AXIS0,TOL=1,LOOKUP=LINMOT_1D","NUMPOS", 0, 30)
$(IFIOC_LINMOT_01) dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPointsSingleAxis.db","P=$(MYPVPREFIX)LIN_LKUP1D:,AXIS0=$(MYPVPREFIX)LIN_AXIS0,TOL=1,LOOKUP=LINMOT_1D")

# 2D with axis
$(IFIOC_GALIL_01) dbLoadRecordsLoop("$(MOTIONSETPOINTS)/db/inPos.db","P=$(MYPVPREFIX)LKUP2D:,AXIS0=$(MYPVPREFIX)AXIS0,AXIS1=$(MYPVPREFIX)AXIS1,TOL=1,LOOKUP=LOOKUPFILE2DAXIS","NUMPOS", 0, 30)
$(IFIOC_GALIL_01) dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPointsDoubleAxis.db","P=$(MYPVPREFIX)LKUP2D:,AXIS0=$(MYPVPREFIX)AXIS0,AXIS1=$(MYPVPREFIX)AXIS1,TOL=1,LOOKUP=LOOKUPFILE2DAXIS")

# 10D with axis
$(IFIOC_GALIL_02) dbLoadRecordsLoop("$(MOTIONSETPOINTS)/db/inPos.db","P=$(MYPVPREFIX)LKUP10D:,AXIS0=$(MYPVPREFIX)AXIS0,AXIS1=$(MYPVPREFIX)AXIS1,AXIS2=$(MYPVPREFIX)AXIS2,AXIS3=$(MYPVPREFIX)AXIS3,AXIS4=$(MYPVPREFIX)AXIS4,AXIS5=$(MYPVPREFIX)AXIS5,AXIS6=$(MYPVPREFIX)AXIS6,AXIS7=$(MYPVPREFIX)AXIS7,AXIS8=$(MYPVPREFIX)AXIS8,AXIS9=$(MYPVPREFIX)AXIS9,TOL=1,LOOKUP=LOOKUPFILE10DAXIS","NUMPOS", 0, 30)
$(IFIOC_GALIL_02) dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPoints10Axis.db","P=$(MYPVPREFIX)LKUP10D:,AXIS0=$(MYPVPREFIX)AXIS0,AXIS1=$(MYPVPREFIX)AXIS1,AXIS2=$(MYPVPREFIX)AXIS2,AXIS3=$(MYPVPREFIX)AXIS3,AXIS4=$(MYPVPREFIX)AXIS4,AXIS5=$(MYPVPREFIX)AXIS5,AXIS6=$(MYPVPREFIX)AXIS6,AXIS7=$(MYPVPREFIX)AXIS7,AXIS8=$(MYPVPREFIX)AXIS8,AXIS9=$(MYPVPREFIX)AXIS9,TOL=1,LOOKUP=LOOKUPFILE10DAXIS")

# duplicate names
$(IFIOC_GALIL_01) dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPointsSingleAxis.db","P=$(MYPVPREFIX)LKUPDN:,AXIS0=$(MYPVPREFIX)AXIS0,TOL=1,LOOKUP=LOOKUPFILEDN")

# duplicate positions
$(IFIOC_GALIL_01) dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPointsDoubleAxis.db","P=$(MYPVPREFIX)LKUPDP:,AXIS0=$(MYPVPREFIX)AXIS0,AXIS1=$(MYPVPREFIX)AXIS1,TOL=1,LOOKUP=LOOKUPFILEDP")
