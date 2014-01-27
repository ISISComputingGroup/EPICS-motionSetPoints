TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = motionSetPoints
# motionSetPoints.dbd will be created and installed
DBD += motionSetPoints.dbd

# motionSetPoints.dbd will be made up from these files:
motionSetPoints_DBD += base.dbd

# Include dbd files from all support applications:
motionSetPoints_DBD += setPoint.dbd

# Add all the support libraries needed by this IOC
#motionSetPoints_LIBS += xxx

# motionSetPoints_registerRecordDeviceDriver.cpp derives from motionSetPoints.dbd
motionSetPoints_SRCS += motionSetPoints_registerRecordDeviceDriver.cpp
motionSetPoints_SRCS += setPoint.cpp
motionSetPoints_SRCS += setPointPVs.c

# Build the main IOC entry point on workstation OSs.
motionSetPoints_SRCS_DEFAULT += motionSetPointsMain.cpp
motionSetPoints_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#motionSetPoints_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
motionSetPoints_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

