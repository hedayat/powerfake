#  Distributed under the Boost Software License, Version 1.0.
#       (See accompanying file LICENSE_1_0.txt or copy at
#             http://www.boost.org/LICENSE_1_0.txt)

include(CMakeFindDependencyMacro)
find_dependency(Boost)

include("${CMAKE_CURRENT_LIST_DIR}/PowerFakeTargets.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/PowerFakeFunctions.cmake")
