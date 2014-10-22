TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

LIBRARY_IOC = sampleChanger

# sampleChanger.dbd will be created and installed
DBD += sampleChanger.dbd

# Add all the support libraries needed by this IOC
sampleChanger_LIBS += asubFunctions asyn TinyXML

sampleChanger_SRCS += converter.cpp sampleChanger.cpp

# Add support from base/src/vxWorks if needed
#motionSetPoints_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
sampleChanger_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

