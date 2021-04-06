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
    VERSION "4.5.1"
    CMAKE_ARGS 
        BUILD_SHARED_LIBS=OFF
        WITH_PROTOBUF=OFF
        ENABLE_PRECOMPILED_HEADERS=${OpenCV_ENABLE_PRECOMPILED_HEADERS}
)
hunter_config(
    OpenCV-Extra
    VERSION "4.5.1"
)
hunter_config(
    range-v3
    VERSION "0.5.0"
)
hunter_config(
    fitpackpp
    VERSION ${HUNTER_fitpackpp_VERSION}
    KEEP_PACKAGE_SOURCES
)
