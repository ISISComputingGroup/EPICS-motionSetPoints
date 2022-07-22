$(IFIOC_GALIL_01) epicsEnvSet "LOOKUPFILE1" "$(MOTIONSETPOINTS/settings/setting_for_system_tests/instrument_scripts/loq/aperture.txt"

$(IFIOC_GALIL_01) epicsEnvSet "LOOKUPFILE2" "$(MOTIONSETPOINTS)/settings/setting_for_system_tests/instrument_scripts/loq/sample_changer.txt"

$(IFIOC_GALIL_01) epicsEnvSet "LOOKUPFILE4" "$(MOTIONSETPOINTS)/settings/setting_for_system_tests/instrument_scripts/loq/transmission_monitor.txt"

$(IFIOC_GALIL_01) epicsEnvSet "LOOKUPFILE5" "$(MOTIONSETPOINTS)/settings/setting_for_system_tests/instrument_scripts/loq/dls_sample_changer.txt"

$(IFIOC_GALIL_01) epicsEnvSet "LOOKUPFILE6" "$(MOTIONSETPOINTS)/settings/setting_for_system_tests/instrument_scripts/loq/sample.txt"

$(IFIOC_GALIL_01) motionSetPointsConfigure("LOOKUPFILE1","LOOKUPFILE1", 1)

$(IFIOC_GALIL_01) motionSetPointsConfigure("LOOKUPFILE2","LOOKUPFILE2", 1)

$(IFIOC_GALIL_01) motionSetPointsConfigure("LOOKUPFILE4","LOOKUPFILE4", 1)

$(IFIOC_GALIL_01) motionSetPointsConfigure("LOOKUPFILE5","LOOKUPFILE5", 1)

$(IFIOC_GALIL_01) motionSetPointsConfigure("LOOKUPFILE6","LOOKUPFILE6", 2)

# The tolerance must be large to make sure that LOCN always points to the closest setpoint
$(IFIOC_GALIL_01) dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPointsSingleAxis.db","P=$(MYPVPREFIX)LKUP:APERTURE:,NAME0=APERTURE,AXIS0=$(MYPVPREFIX)MOT:APERTURE,TOL=10,LOOKUP=LOOKUPFILE1")

# Load the records which control closing the aperture
$(IFIOC_GALIL_01) dbLoadRecords("$(MOTOREXT)/db/loqAperture.db", "P=$(MYPVPREFIX), SETPTAXIS=$(MYPVPREFIX)LKUP:APERTURE:")

$(IFIOC_GALIL_01) dbLoadRecordsLoop("$(MOTIONSETPOINTS)/db/inPos.db","P=$(MYPVPREFIX)LKUP:APERTURE:,NAME0=APERTURE,AXIS0=$(MYPVPREFIX)MOT:APERTURE,TOL=10,LOOKUP=LOOKUPFILE1","NUMPOS", 0, 30)

# Set up the other axes
$(IFIOC_GALIL_01) dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPointsSingleAxis.db","P=$(MYPVPREFIX)LKUP:SAMPLE_OLD:,NAME0=SAMPLE,AXIS0=$(MYPVPREFIX)MOT:SAMPLE,TOL=1,LOOKUP=LOOKUPFILE2")

$(IFIOC_GALIL_01) dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPointsDoubleAxis.db","P=$(MYPVPREFIX)LKUP:SAMPLE:,NAME0=SAMPLE,AXIS0=$(MYPVPREFIX)MOT:SAMPLE,NAME1=HEIGHT,AXIS1=$(MYPVPREFIX)MOT:HEIGHT,TOL=1,LOOKUP=LOOKUPFILE6")

$(IFIOC_GALIL_01) dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPointsSingleAxis.db","P=$(MYPVPREFIX)LKUP:TRANS_MON:,NAME0=TRANS_MON,AXIS0=$(MYPVPREFIX)MOT:TRANS_MON,TOL=1,LOOKUP=LOOKUPFILE4")


$(IFIOC_GALIL_01) dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPointsSingleAxis.db","P=$(MYPVPREFIX)LKUP:DLS:,NAME0=DLS,AXIS0=$(MYPVPREFIX)MOT:DLS,TOL=1,LOOKUP=LOOKUPFILE5")

$(IFIOC_GALIL_01) dbLoadRecordsLoop("$(MOTIONSETPOINTS)/db/inPos.db","P=$(MYPVPREFIX)LKUP:TRANS_MON:,NAME0=TRANS_MON,AXIS0=$(MYPVPREFIX)MOT:TRANS_MON,TOL=1,LOOKUP=LOOKUPFILE4", "NUMPOS", 0, 2)
