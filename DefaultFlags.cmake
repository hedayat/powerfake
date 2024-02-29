#  Distributed under the Boost Software License, Version 1.0.
#       (See accompanying file LICENSE_1_0.txt or copy at
#             http://www.boost.org/LICENSE_1_0.txt)

# Note: this file will be executed once per enabled language
if (CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_CXX_FLAGS_INIT MATCHES "Wall")
    string(APPEND CMAKE_CXX_FLAGS_INIT " -Wall -Wextra -Woverloaded-virtual"
        " -Wpedantic -fstack-protector-strong -Wconversion")
    string(APPEND CMAKE_CXX_FLAGS_RELEASE_INIT " -Wp,-D_FORTIFY_SOURCE=2")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG_INIT " -Og")
endif()
