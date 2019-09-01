cmake_minimum_required(VERSION 3.11.0)
include(cmake/HunterGate.cmake)
set(
    HUNTER_CACHE_SERVERS
    "https://github.com/elucideye/hunter-cache;https://github.com/ingenue/hunter-cache"
    CACHE
    STRING
    "Hunter cache servers"
)
HunterGate(
    URL "http://gitlab.centrilliontech.com.tw:10080/centrillion/hunter.git"
    SHA1 5e77997222f7e2c6b3d43a3301f36ae3f8448fa8
    FILEPATH ${CMAKE_CURRENT_LIST_DIR}/cmake/packages.cmake
)
project(ChipImgProc LANGUAGES C CXX)

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
include(cmake/install.cmake)
include(cmake/cpack.cmake)
