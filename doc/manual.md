Getting start {#mainpage}
===

# Index {#index}

* @ref import-chipimgproc-by-hunter   "Import ChipImgProc by Hunter(recommended)"
* @ref build-chipimgproc-from-source  "Build ChipImgProc from source"
* @ref manually-import-chipimgproc    "Manually import ChipImgProc"
* @ref a-basic-example                "A basic example from FOV to heatmap"

## Import ChipImgProc by Hunter(recommended) {#import-chipimgproc-by-hunter}

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

### Requirement

* Your project must be a CMake project
* GCC >= 7.3 (7.3 is recommended and well tested)
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

## Build ChipImgProc from source by CMake on Windows MinGW {#build-chipimgproc-from-source}

If you don't want to use Hunter in your client project, you may build the ChipImgProc manually.

In this case, ChipImgProc still use Hunter to maintain its self's upstream but
there wouldn't be any Hunter code in client project and the library link to ChipImgProc may be a struggle.

### ChipImgProc build requirements

* GCC >= 7.3 (7.3 is recommended and well tested)
* CMake >= 3.13
* MSVC <= v141 build tool

The main library compiler is GCC, MSVC is required by Boost build flow.

### Client project requirements

* GCC same as ChipImgProc build

The ChipImgProc is built from GCC, therefore you must use GCC to link them.

We are still work on MSVC supporting.
In this case wChipImgProc should be compiled in MSVC, but during the work on MSVC,
we found there are many syntax difference between GCC and MSVC even use Visual Studio 2019.
It probably standard support issue or just compiler bugs.

### ChipImgProc build steps

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

### About Gitlab download icon link

@image html gitlab-download-icon-link.png width=300px
@image latex gitlab-download-icon-link.png

We assume the users visit the download icon link or the release tags just want to use the library but run the unit test, which means the users download project in such way should use an the alternative configure command:

```bat
build\> cmake .. -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX="..\stage" -DINSTALL_DEPS=ON -DCMAKE_BUILD_TYPE="Release" -DCOPY_ALL_TP=ON -DBUILD_TESTS=OFF
```

In this case, Build script will not need test data and of course, no test code will be built.
Only library source will be compiled into binary.

## Manually import ChipImgProc {#manually-import-chipimgproc}

Import ChipImgProc may need several compiler flags and definitions,
These compiler options not only from ChipImgProc, but OpenCV and other upstream.

Here we list all client's upstream and compiler options, the client project
should find a way to link these binary manually.

* C++ flags (assuming use g++ compiler):
  * -std=c++17
* Include directory:
  * ChipImgProc\\stage\\include
  * ChipImgProc\\stage\\third_party\\include
  * ChipImgProc\\stage\\third_party\\include\\opencv
* C++ Define flags (nessesary):
  * -DNUCLEONA_RANGE_USE_V3
  * -DOPENCV_TRAITS_ENABLE_DEPRECATED
* Link directory:
  * ChipImgProc\\stage\\lib
  * ChipImgProc\\stage\\third_party\\lib
  * ChipImgProc\\stage\\third_party\\x64\\mingw\\staticlib
* Link libraries:
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
    * libopencv_ml340
    * libopencv_objdetect340
    * libopencv_shape340
    * libopencv_stitching340
    * libopencv_superres340
    * libopencv_videostab340
    * libopencv_calib3d340
    * libopencv_features2d340
    * libopencv_flann340
    * libopencv_highgui340
    * libopencv_photo340
    * libopencv_video340
    * libopencv_videoio340
    * libopencv_imgcodecs340
    * libopencv_imgproc340
    * libopencv_core340
  * OpenCV upstream dependent
    * liblibjasper
    * libjpeg
    * liblibwebp
    * libpng
    * liblibtiff
  * Boost
    * libboost_filesystem-mt-X64
    * more boost library goes here...
  * Win32 and other utility library
    * libzlib
    * comctl32
    * gdi32
    * setupapi
    * ws2_32
    * m
    * avifil32
    * avicap32
    * winmm
    * msvfw32
    * kernel32
    * user32
    * gdi32
    * winspool
    * shell32
    * ole32
    * oleaut32
    * uuid
    * comdlg32
    * advapi32

### Trouble shooting

* Missing link library

    The library link is highly depend on user code, your code may require alternative link order or use more library beyond the following list.
    We put all dependencies in the \<*ChipImgProc install prefix*>/lib and \<*ChipImgProc install prefix*>/third_party.
    You should be able to find any missing library and add to your link command.

Manually include and link OpenCV & Boost are really painful, so we suggest to use CMake & Hunter to do such link works.

## A basic example from FOV to heatmap {#a-basic-example}

@snippet ChipImgProc/multi_tiled_mat_test.cpp usage
