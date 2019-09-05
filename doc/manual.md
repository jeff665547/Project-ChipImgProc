# Getting start {#mainpage}

## Build ChipImgProc from source by CMake on Windows MinGW

### Requirement

* Your project must be a CMake project
* GCC >= 7.3 (7.3 is recommanded and well tested)
* CMake >= 3.13
* MSVC <= v141 build tool

### Steps

```bat
:: Download submodules (ChipImgProcTestData)
ChipImgProc\> git submodule init
ChipImgProc\> git submodule update

:: Prepare build
ChipImgProc\> mkdir build
ChipImgProc\> cd build

:: Configure build, will build all upstream dependencies
build\> cmake .. -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX="..\stage" -DINSTALL_DEPS=ON -DCMAKE_BUILD_TYPE="Release" -DCOPY_ALL_TP=ON

:: Build project, and install
build\> cmake --build . --target install

:: Now the result will put in ChipImgProc\stage
:: All upstream library will put in ChipImgProc\stage\third_party
```

## Manually link ChipImgProc

Include directory:

* ChipImgProc\stage\include
* ChipImgProc\stage\third_party\include

Link directory:

* ChipImgProc\stage\lib
* ChipImgProc\stage\third_party\lib

Link libraries:

TODO:

## Import ChipImgProc by hunter

To use the hunter package manager, all upstream will be built.
User no need to build ChipImgProc manually.

### Requirment

* Your project must be a CMake project
* GCC >= 7.3 (7.3 is recommanded and well tested)
* CMake >= 3.13
* MSVC <= v141 build tool

### Hunter configuration

1. Download Hunter Gate from [our Gitlab](http://gitlab.centrilliontech.com.tw:10080/centrillion/gate/blob/URL-git-commit/cmake/HunterGate.cmake).
2. Put HunterGate.cmake to *\<your_project\>*/cmake/ directory.
3. Create and add following code to *\<your_project\>*/cmake/packages.cmake

        if( MINGW )
            set(OpenCV_ENABLE_PRECOMPILED_HEADERS OFF)
        else()
            set(OpenCV_ENABLE_PRECOMPILED_HEADERS ON)
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

4. Add following code into CMakeLists.txt and place before ```project(...)```.

        include(cmake/HunterGate.cmake)
        HunterGate(
            URL "http://gitlab.centrilliontech.com.tw:10080/centrillion/hunter.git"
            SHA1 7534d27dc7d7c18381f995a4ef48140d1e6279ed
            FILEPATH ${CMAKE_CURRENT_LIST_DIR}/cmake/packages.cmake
        )

5. Add following code after ```project(...)```.

        hunter_add_package(ChipImgProc)
        find_package(ChipImgProc CONFIG REQUIRED)

### A full example

```cmake
cmake_minimum_required(VERSION 3.13.0)
include(cmake/HunterGate.cmake)
HunterGate(
    URL "http://gitlab.centrilliontech.com.tw:10080/centrillion/hunter.git"
    SHA1 7534d27dc7d7c18381f995a4ef48140d1e6279ed
    FILEPATH ${CMAKE_CURRENT_LIST_DIR}/cmake/packages.cmake
)
project(ChipImgProc-example)

hunter_add_package(ChipImgProc)
find_package(ChipImgProc CONFIG REQUIRED)
add_executable(foo foo.cpp)
target_link_libraries(foo PUBLIC ChipImgProc::ChipImgProc)
```

## Feature Introduction

To extract the probe intensities from the image. 
There are several issues of the image we need to resolve:

1. The images are slanted
2. Noise
3. Grid recognition and segmentation
4. A chip sample is captured into multiple images, which need to be stitched.
5. The region of interest detection, remove the regions we don't need.
etc.

To solve these issues, we develop a pipeline with following steps:

* @subpage improc_image_rotation
* @subpage improc_gridding
* @subpage improc_min_cv_auto_margin
* @subpage improc_background_fix_sub_and_division01
* @subpage improc_stitiching
