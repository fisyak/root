# Copyright (C) 1995-2019, Rene Brun and Fons Rademakers.
# All rights reserved.
#
# For the licensing terms see $ROOTSYS/LICENSE.
# For the list of contributors see $ROOTSYS/README/CREDITS.

ROOT_ADD_GTEST(testMake_unique testMake_unique.cxx LIBRARIES Core)
ROOT_ADD_GTEST(testTypeTraits testTypeTraits.cxx LIBRARIES Core)
ROOT_ADD_GTEST(testNotFn testNotFn.cxx LIBRARIES Core)
ROOT_ADD_GTEST(testClassEdit testClassEdit.cxx  LIBRARIES Core RIO COPY_TO_BUILDDIR ${CMAKE_CURRENT_SOURCE_DIR}/file_16199.C)
ROOT_ADD_GTEST(testException testException.cxx LIBRARIES Core GTest::gmock)
ROOT_ADD_GTEST(testLogger testLogger.cxx LIBRARIES Core)
ROOT_ADD_GTEST(testRRangeCast testRRangeCast.cxx LIBRARIES Core)
ROOT_ADD_GTEST(testStringUtils testStringUtils.cxx LIBRARIES Core)
ROOT_ADD_GTEST(FoundationUtilsTests FoundationUtilsTests.cxx LIBRARIES Core INCLUDE_DIRS ../res)
