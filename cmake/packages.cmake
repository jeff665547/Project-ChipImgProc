if( MINGW )
    set(ZLIB_BUILD_SHARED_LIBS OFF)
    set(OpenCV_ENABLE_PRECOMPILED_HEADERS OFF)
else()
    set(ZLIB_BUILD_SHARED_LIBS ON)
    set(OpenCV_ENABLE_PRECOMPILED_HEADERS ON)
endif()
if( MSVC )
    hunter_config(
        GTest
        VERSION ${HUNTER_GTest_VERSION}
        CMAKE_ARGS 
            CMAKE_CXX_FLAGS=/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
    )
else()
    hunter_config(
        GTest
        VERSION ${HUNTER_GTest_VERSION}
    )
endif()
hunter_config(
    OpenCV
    VERSION "3.4.0-p0"
    CMAKE_ARGS 
        BUILD_SHARED_LIBS=OFF
        ENABLE_PRECOMPILED_HEADERS=${OpenCV_ENABLE_PRECOMPILED_HEADERS}
)
hunter_config(
    OpenCV-Extra
    VERSION "3.4.0"
)
