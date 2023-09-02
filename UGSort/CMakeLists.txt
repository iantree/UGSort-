# CMakeList.txt : CMake project for UGSort, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (UGSort "UGSort.cpp" "UGSort.h" "IStats.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET UGSort PROPERTY CXX_STANDARD 20)
endif()

#  Old Linux Compat
if (CMAKE_VERSION VERSION_LESS 3.19)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -O2")
endif()

#  Build and Install
install (TARGETS UGSort DESTINATION "${PROJECT_SOURCE_DIR}/rt/bin")
