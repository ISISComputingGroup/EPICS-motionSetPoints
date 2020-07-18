#include "gtest/gtest.h"

#include <string.h>

#include <epicsUnitTest.h>
#include <testMain.h>

const char *valRight = "hello EPICS";
const char *valWrong = "hello google";
const char *expected = "hello EPICS";

//int main(int argc, char **argv) {
//    ::testing::InitGoogleTest( &argc, argv );
//    return RUN_ALL_TESTS();
//    }

//MAIN(motionSetPointsTest)
//{
//    testPlan(2);

    //testOk(!strcmp(expected, valRight), "expected equals valRight");
    //testOk(!!strcmp(expected, valWrong), "expected does not equal valWrong");
    //return testDone();
//}