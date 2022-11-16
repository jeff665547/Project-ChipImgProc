cmake_minimum_required(VERSION 3.11.0)
include(cmake/HunterGate.cmake)
set(
    HUNTER_CACHE_SERVERS
    "https://github.com/CHChang810716/hunter-binary-cache"
    CACHE
    STRING
    "Hunter cache servers"
)
HunterGate(
    URL "ssh://git@gitlab.centrilliontech.com.tw:10022/centrillion/hunter.git"
    SHA1 222439ce7e923b72729eb537fd6187523cc69286
    FILEPATH ${CMAKE_CURRENT_LIST_DIR}/cmake/packages.cmake
)
project(ChipImgProc LANGUAGES C CXX Fortran)

include(ExternalProject)
hunter_add_package(Screw)
find_package( Screw CONFIG REQUIRED )

include(cmake/version.cmake)
include(cmake/options.cmake)
include(cmake/dependencies.cmake)
include(cmake/compiler.cmake)
include(cmake/src.cmake)
include(cmake/unit_test.cmake)
include(cmake/ide.cmake)
include(cmake/doc.cmake)
include(cmake/install.cmake)
include(cmake/cpack.cmake)
