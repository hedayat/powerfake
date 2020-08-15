# Note: this file will be executed once per enabled language
if (CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_CXX_FLAGS_INIT MATCHES "Wall")
    string(APPEND CMAKE_CXX_FLAGS_INIT " -Wall -Wextra -Woverloaded-virtual"
            " -Wpedantic -fstack-protector-strong")
    string(APPEND CMAKE_CXX_FLAGS_RELEASE_INIT " -Wp,-D_FORTIFY_SOURCE=2")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG_INIT " -Og")
endif()
