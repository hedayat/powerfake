include(CMakeFindDependencyMacro)
find_dependency(Boost)

include("${CMAKE_CURRENT_LIST_DIR}/PowerFakeTargets.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/PowerFakeFunctions.cmake")
