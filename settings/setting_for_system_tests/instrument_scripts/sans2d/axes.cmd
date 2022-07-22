$(IFIOC_GALIL_04) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=REARBAFFLEZ,mAXIS=MTR0408")
$(IFIOC_GALIL_05) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=FRONTBAFFLEZ,mAXIS=MTR0503")

$(IFIOC_GALIL_05) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=FRONTDETZ,mAXIS=MTR0501")
$(IFIOC_GALIL_05) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=FRONTDETX,mAXIS=MTR0505")
$(IFIOC_GALIL_05) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=FRONTDETROT,mAXIS=MTR0506")
$(IFIOC_GALIL_04) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=REARDETX,mAXIS=MTR0405")
$(IFIOC_GALIL_04) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=REARDETZ,mAXIS=MTR0401")

$(IFIOC_GALIL_04) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=BEAMSTOPX,mAXIS=MTR0402")
$(IFIOC_GALIL_04) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=BEAMSTOP2Y,mAXIS=MTR0403")
$(IFIOC_GALIL_04) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=BEAMSTOP1Y,mAXIS=MTR0406")
$(IFIOC_GALIL_04) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=BEAMSTOP3Y,mAXIS=MTR0407")
$(IFIOC_GALIL_05) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=FRONTBEAMSTOP,mAXIS=MTR0502")

$(IFIOC_GALIL_04) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=REARSTRIP,mAXIS=MTR0404")
$(IFIOC_GALIL_05) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=FRONTSTRIP,mAXIS=MTR0504")

$(IFIOC_GALIL_03) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=JAWRIGHT,mAXIS=MTR0301")
$(IFIOC_GALIL_03) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=JAWLEFT,mAXIS=MTR0302")
$(IFIOC_GALIL_03) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=JAWUP,mAXIS=MTR0303")
$(IFIOC_GALIL_03) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=JAWDOWN,mAXIS=MTR0304")

# Sample Changer PVs are called SAMP:X OR SAMP:Y, change it accordingly when huber stage is used
$(IFIOC_GALIL_03) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=SAMP:X,mAXIS=MTR0305")
$(IFIOC_GALIL_03) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=SAMP:Y,mAXIS=MTR0306")

$(IFIOC_GALIL_05) dbLoadRecords("$(AXIS)/db/axis.db","P=$(MYPVPREFIX)MOT:,AXIS=SCRAPER,mAXIS=MTR0507")
