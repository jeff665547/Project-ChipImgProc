
Installation
============

[TOC]

Use package manager
===================

To use the Hunter package manager, all upstream will be built,
user no need to build ChipImgProc before user project develop.

It may takes several minutes to build upstream dependencies,
but it only happened in the first time build.

The reason we use CMake + MinGW + Hunter package manager is because
ChipImgProc as a library package it is required to support multiple platform, include:

* Linux x86 64/32bit
* Linux ARM
* Windows x84/64

And of course cross compiling.
In this case, we need a way to lock upstream which should not be bother by platform owned library
and use GNU compiler which is most portable in syntax level.

Requirements
------------

* Your project must be a CMake project
* GCC >= 7.3 (7.3 is recommended and well tested)
* CMake >= 3.13
* MSVC <= v141 build tool

The main library compiler is GCC, MSVC is required by Boost build flow.

Hunter configuration
--------------------

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

A full example
--------------

cmake/packages.cmake

```cmake
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
```

CMakeLists.txt

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

foo.cpp

```cpp
#include <ChipImgProc.h>
int main() {
    // You can start use ChipImgProc now
    std::cout << chipimgproc::get_version() << std::endl;
}

```

Manually build (MinGW)
=======================

If you don't want to use Hunter in your client project, you may build the ChipImgProc manually.

In this case, ChipImgProc still use Hunter to maintain its self's upstream but
there wouldn't be any Hunter code in client project but because ChipImgProc use a plenty of packages, the library link of client project probably complicates.

Build requirements (MinGW)
------------------

* GCC >= 7.3 (7.3 is recommended and well tested)
* CMake >= 3.13
* MSVC <= v14.1 build tool

The main library compiler is GCC, MSVC is required by Boost build flow.

Client project requirements (MinGW)
---------------------------

* GCC same as ChipImgProc build

The ChipImgProc is built from GCC, therefore you must use GCC to link them.

Build steps (MinGW)
-----------------------

Assume the project is download from git clone.

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

Gitlab download icon link
---

@image html gitlab-download-icon-link.png width=300px
@image latex gitlab-download-icon-link.png

We assume the users visit the download icon link or the release tags just want to use the library but run the unit test, which means the users download project in such way should use an the alternative configure command:

```bat
ChipImgProc\> cmake .. -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX="..\stage" -DINSTALL_DEPS=ON -DCMAKE_BUILD_TYPE="Release" -DCOPY_ALL_TP=ON -DBUILD_TESTS=OFF
```

In this case, Build script will not need test data and of course, no test code will be built.
Only library source will be compiled into binary.

Manually build (MSVC - experimental)
====================================

The MSVC build is currently an experimental feature which is less test.

Build requirements (MSVC)
------------------

* CMake >= 3.13
* MSVC >= v15.9 and <= v14.1 (both needed)

The MSVC 14.1 is required by Boost 1.69 build flow.
By hacking the Boost build code, we found:

* Boost build a build engine and use the build engine to build the Boost code.
* The build tool used to build build engine and Boost code can be different.
* The build script used to build the Boost build engine is unable to detect the MSVC > v14.1

Therefore, we use v14.1 to build the build engine
and the build engine call the v15.9 to build Boost code.

Client project requirements (MSVC)
----------------------------------

* MSVC same as ChipImgProc build.

Different version of MSVC may not compatible in library binary interface (ABI),
which may cause link fail.

Build steps (MSVC)
-----------

Assume the project is download from git clone.

```bat
:: Download submodules (ChipImgProcTestData)
ChipImgProc\> git submodule init
ChipImgProc\> git submodule update

:: Prepare build
ChipImgProc\> mkdir build
ChipImgProc\> cd build

:: Configure build, will build all upstream dependencies
build\> cmake .. -G "Visual Studio 15 2017 Win64" -DCMAKE_INSTALL_PREFIX="..\stage" -DINSTALL_DEPS=ON -DCMAKE_BUILD_TYPE="Release" -DCOPY_ALL_TP=ON

:: Build project, and install
build\> cmake --build . --target install

:: Now the result will put in ChipImgProc\stage
:: All upstream library will put in ChipImgProc\stage\third_party
```

Manually import
===============

Import ChipImgProc may need several compiler flags and definitions,
These compiler options not only from ChipImgProc, but OpenCV and other upstream.

Here we list all client's upstream and compiler options, the client project
should find a way to link these binary manually.

Use gcc
---

C++ flags:

* -std=c++17
* -Wa,-mbig-obj

Include directory:

* ChipImgProc\\stage\\include
* ChipImgProc\\stage\\third_party\\include
* ChipImgProc\\stage\\third_party\\include\\opencv

C++ Define flags (nessesary):

* CHIPIMGPROC_ENABLE_LOG
* NUCLEONA_RANGE_USE_V3
* OPENCV_TRAITS_ENABLE_DEPRECATED
* SPDLOG_FMT_EXTERNAL=1

Link directories:

* ChipImgProc\\stage\\lib
* ChipImgProc\\stage\\third_party\\lib
* ChipImgProc\\stage\\third_party\\x64\\mingw\\staticlib

Link libraries:

* ChipImgProc
  * libChipImgProc-logger
  * libChipImgProc-utils
  * libcpp_base64-base64
* Nucleona
  * libNucleona
  * libNucleona-sys-executable_dir
  * libNucleona-stream-null_buffer
  * libNucleona-parallel-thread_pool
  * libNucleona-util
* OpenCV
  * libopencv_*
  * (too much of them, not list all)
* OpenCV upstream dependent
  * liblibjasper
  * libjpeg
  * liblibwebp
  * libpng
  * liblibtiff
* Boost
  * libboost_iostreams-mt-x64
  * libboost_filesystem-mt-x64
  * libboost_system-mt-x64
* Other library
  * libzlib
  * libfmt
* Win32 API probably used
  * GUI
    * comctl32, gdi32, user32
    * comdlg32
  * IO
    * setupapi, ws2_32
  * math
    * m
  * media
    * avifil32, avicap32, winmm
    * msvfw32
  * OS API
    * kernel32, shell32, ole32
    * oleaut32
    * advapi32

The win32 API may optionally used by some module in opencv, user probably (not always) need to link them.

Use MSVC
--------

C++ flags:

* /bigobj
* /permissive-
* /std:c++17

Include directory:

* ChipImgProc\\stage\\include
* ChipImgProc\\stage\\third_party\\include
* ChipImgProc\\stage\\third_party\\include\\opencv

C++ Define flags (nessesary):

* BOOST_ALL_NO_LIB=1
* CHIPIMGPROC_ENABLE_LOG
* NUCLEONA_RANGE_USE_V3
* OPENCV_TRAITS_ENABLE_DEPRECATED
* SPDLOG_FMT_EXTERNAL=1

Link directories:

* ChipImgProc\\stage\\lib
* ChipImgProc\\stage\\third_party\\lib
* ChipImgProc\\stage\\third_party\\x64\\vc15\\lib

Link libraries:

* ChipImgProc
  * ChipImgProc-logger
  * ChipImgProc-utils
  * cpp_base64-base64
* Nucleona
  * Nucleona
  * Nucleona-sys-executable_dir
  * Nucleona-stream-null_buffer
  * Nucleona-parallel-thread_pool
  * Nucleona-util
* OpenCV
  * opencv_*
  * (too much of them, not list all)
* OpenCV upstream dependent
  * libjasper
  * jpeg
  * libwebp
  * png
  * libtiff
* Boost
  * libboost_iostreams-mt-x64
  * libboost_filesystem-mt-x64
  * libboost_system-mt-x64
* Other library
  * zlib
  * fmt
* Win32 API probably used
  * GUI
    * comctl32, gdi32, user32
    * comdlg32
  * IO
    * setupapi, ws2_32
  * math
    * m
  * media
    * avifil32, avicap32, winmm
    * msvfw32
  * OS API
    * kernel32, shell32, ole32
    * oleaut32
    * advapi32

The win32 API may optionally used by some module in opencv, user probably (not always) need to link them.

Trouble shooting
---

* We not list all libraries

    The components in Nucleona and ChipImgProc is actually much more than the list,
    Here we only list the highest used parts, user may just import all components in library directories
    to avoid some missing link problem.

* Missing link library

    The library link is highly depend on user code, your code may require alternative link order or use more library beyond the following list.
    We put all dependencies in the \<*ChipImgProc install prefix*>/lib and \<*ChipImgProc install prefix*>/third_party.
    You should be able to find any missing library and add to your link command.

Manually include and link OpenCV & Boost are really painful, so we suggest to use CMake & Hunter to do such link works.
