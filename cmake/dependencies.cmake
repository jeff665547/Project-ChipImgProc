# nucleona 
hunter_add_package(Nucleona)
find_package(Nucleona CONFIG REQUIRED)

# boost 
hunter_add_package(Boost COMPONENTS 
    thread 
    system 
    filesystem
    # more boost module goes here
)
find_package(Boost CONFIG COMPONENTS 
    thread 
    system 
    filesystem 
    # more boost module goes here
    REQUIRED
)

# gtest
if(BUILD_TESTS)
    hunter_add_package(GTest)
    find_package(GTest CONFIG REQUIRED)
endif()

# json
hunter_add_package(nlohmann_json)
find_package(nlohmann_json CONFIG REQUIRED)

# opencv
hunter_add_package(OpenCV)
screw_get_msvc_name(OpenCV_RUNTIME)
find_package(OpenCV CONFIG REQUIRED)
message(STATUS "OpenCV_DIR: ${OpenCV_DIR}")
message(STATUS "OpenCV_CONFIG: ${OpenCV_CONFIG}")
message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
screw_get_bits(BITS)
list(APPEND BUNDLE_RT_DIRS ${OpenCV_DIR}/x${BITS}/${OpenCV_RUNTIME}/bin)
list(APPEND BUNDLE_RT_DIRS ${OpenCV_DIR}/x${BITS}/${OpenCV_RUNTIME}/lib)

# range v3
hunter_add_package(range-v3)
find_package(range-v3 CONFIG REQUIRED)
if(MSVC)
    target_compile_options(range-v3 INTERFACE /permissive-)
endif()

# fmt
hunter_add_package(fmt)
find_package(fmt CONFIG REQUIRED)

# spdlog
if(ENABLE_LOG)
    hunter_add_package(spdlog)
    find_package(spdlog CONFIG REQUIRED)
    target_compile_definitions(spdlog::spdlog INTERFACE 
        SPDLOG_FMT_EXTERNAL=1
    )
    target_link_libraries(spdlog::spdlog INTERFACE 
        fmt::fmt
    )
endif()

# threads
find_package(Threads REQUIRED)

# hunter root
include(${SCREW_DIR}/hunter_root.cmake)