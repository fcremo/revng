#
# This file is distributed under the MIT License. See LICENSE.md for details.
#

enable_testing()

# Give control to the various subdirectories
include(${CMAKE_SOURCE_DIR}/tests/runtime/RuntimeTests.cmake)
include(${CMAKE_SOURCE_DIR}/tests/analysis/AnalysisTests.cmake)
include(${CMAKE_SOURCE_DIR}/tests/unit/UnitTests.cmake)
