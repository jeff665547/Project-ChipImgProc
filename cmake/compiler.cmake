set( _GLIBCXX_USE_CXX11_ABI 0 )
set( CMAKE_CXX_STANDARD 14 )
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAG} -Wno-deprecated-declarations")
    # list(APPEND cxx_debug_flag -O0)
    # list(APPEND cxx_flag -Wno-deprecated-declarations)
else()
    # set(cxx_debug_flag)
endif()