# CMakeList.txt : CMake project for UGSort, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (UGSort "UGSort.cpp" "UGSort.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET UGSort PROPERTY CXX_STANDARD 20)
endif()

#  Build and Install
install (TARGETS UGSort)
