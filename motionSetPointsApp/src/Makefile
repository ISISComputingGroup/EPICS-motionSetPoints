TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

LIBRARY_IOC = motionSetPoints

# motionSetPoints.dbd will be created and installed
DBD += motionSetPoints.dbd

# Add all the support libraries needed by this IOC
motionSetPoints_LIBS += asubFunctions asyn

motionSetPoints_SRCS += setPoint.cpp motionSetPoints.cpp

# Add support from base/src/vxWorks if needed
#motionSetPoints_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
motionSetPoints_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

