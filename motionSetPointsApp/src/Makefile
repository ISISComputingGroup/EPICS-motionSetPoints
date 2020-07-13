TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================

# Create test runner
ifeq ($(findstring 10.0,$(VCVERSION)),)
SRC_DIRS += $(TOP)/motionSetPointsApp/src/tests
TESTPROD_HOST += runner
USR_CPPFLAGS += -I$(GTEST)/googletest/include 
runner_SRCS += run_all_tests.cc testSetPoint.cc
runner_LIBS += gtest

#runner_INC += CRYOSMSDriver.h
#runner_LIBS += gtest $(EPICS_BASE_IOC_LIBS) asyn
#runner_SRCS += run_all_tests.cc CRYOSMSTests.cc CRYOSMSDriver.cpp
endif


# Build the IOC application

LIBRARY_IOC = motionSetPoints

# motionSetPoints.dbd will be created and installed
DBD += motionSetPoints.dbd

# Add all the support libraries needed by this IOC
motionSetPoints_LIBS += asubFunctions asyn
motionSetPoints_LIBS += calc

motionSetPoints_SRCS += setPoint.cpp motionSetPoints.cpp

# Add support from base/src/vxWorks if needed
#motionSetPoints_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
motionSetPoints_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

