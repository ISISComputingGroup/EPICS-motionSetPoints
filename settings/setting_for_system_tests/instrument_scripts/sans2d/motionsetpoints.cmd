
# Setting up the Sample Changer values
$(IFIOC_GALIL_03) epicsEnvSet "LOOKUPFILE2" "$(MOTIONSETPOINTS)/settings/setting_for_system_tests/instrument_scripts/sans2d/sample.txt"
$(IFIOC_GALIL_03) motionSetPointsConfigure("LOOKUPFILE2","LOOKUPFILE2", 2)
$(IFIOC_GALIL_03) dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPointsDoubleAxis.db","P=$(MYPVPREFIX)LKUP:SAMPLE:,AXIS0=$(MYPVPREFIX)MOT:SAMP:X,AXIS1=$(MYPVPREFIX)MOT:SAMP:Y,TOL=1,LOOKUP=LOOKUPFILE2")

$(IFIOC_GALIL_03) dbLoadRecordsLoop("$(MOTIONSETPOINTS)/db/inPos.db","P=$(MYPVPREFIX)LKUP:SAMPLE:,AXIS0=$(MYPVPREFIX)MOT:SAMP:X,AXIS1=$(MYPVPREFIX)MOT:SAMP:Y,TOL=1,LOOKUP=LOOKUPFILE2","NUMPOS", 0, 30)



# Setting up the DLS Sample Changer values
$(IFIOC_GALIL_06) epicsEnvSet "LOOKUPFILE3" "$(MOTIONSETPOINTS)/settings/setting_for_system_tests/instrument_scripts/sans2d/dls_sample_changer.txt"
$(IFIOC_GALIL_06) motionSetPointsConfigure("LOOKUPFILE3","LOOKUPFILE3", 2)
$(IFIOC_GALIL_06) dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPointsDoubleAxis.db","P=$(MYPVPREFIX)LKUP:DLS:,AXIS0=$(MYPVPREFIX)MOT:SAMP:X,AXIS1=$(MYPVPREFIX)MOT:SAMP:Y,TOL=1,LOOKUP=LOOKUPFILE3")
        
$(IFIOC_GALIL_06) dbLoadRecordsLoop("$(MOTIONSETPOINTS)/db/inPos.db","P=$(MYPVPREFIX)LKUP:DLS:,AXIS0=$(MYPVPREFIX)MOT:MOT:SAMP:X,AXIS1=$(MYPVPREFIX)MOT:SAMP:Y,TOL=1,LOOKUP=LOOKUPFILE3","NUMPOS", 0, 5)


$(IFIOC_GALILMUL_02) epicsEnvSet "LOOKUPFILE1" "$(ICPCONFIGROOT)/motionSetPoints/scraper_aperture.txt"

$(IFIOC_GALILMUL_02) motionSetPointsConfigure("LOOKUPFILE1","LOOKUPFILE1", 1)

# The tolerance must be large to make sure that LOCN always points to the closest setpoint
$(IFIOC_GALILMUL_02) dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPointsSingleAxis.db","P=$(MYPVPREFIX)LKUP:SCRAPER:,NAME1=SCRAPER,AXIS0=$(MYPVPREFIX)MOT:SCRAPER,TOL=1,LOOKUP=LOOKUPFILE1")

