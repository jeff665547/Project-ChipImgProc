Manual {#mainpage}
===

# Import

## Import with CMake Hunter (recommanded)

### Requirment
* Your project must be a CMake project
* GCC >= 7.3
* CMake >= 3.10

### General hunter configuration

1. Download Hunter Gate from [our Gitlab](http://gitlab.centrilliontech.com.tw:10080/centrillion/gate/blob/URL-git-commit/cmake/HunterGate.cmake).
2. Put HunterGate.cmake to *\<your_project\>*/cmake/ directory.
3. Add following code into CMakeLists.txt and before ```project(...)```.

```
include(cmake/HunterGate.cmake)
HunterGate(
    URL "ssh://git@gitlab.centrilliontech.com.tw:10022/centrillion/hunter.git"
    SHA1 21a50417fc80b73bb8bcaa692cfdb1f5e4ac8076
)
```
4. Configure & build your project

For Windows:
```
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build . 
```

For Linux:
```
mkdir build
cd build
cmake ..
cmake --build . 
```
### Add ChipImgProc dependencies to your project

Add following code after ```project(...)```.
```
hunter_add_package(ChipImgProc)
find_package(ChipImgProc CONFIG REQUIRED)
```

### A full example

```
cmake_minimum_required(VERSION 3.10.0)
include(cmake/HunterGate.cmake)             # include HunterGate
HunterGate(
    URL "ssh://git@gitlab.centrilliontech.com.tw:10022/centrillion/hunter.git"
    SHA1 21a50417fc80b73bb8bcaa692cfdb1f5e4ac8076
)
project(ChipImgProc-example)

hunter_add_package(ChipImgProc)             # Download ChipImgProc package.
find_package(ChipImgProc CONFIG REQUIRED)   # Find and configure the package.
add_executable(foo foo.cpp)                 # Create an executable target.
target_link_libraries(
    foo PUBLIC ChipImgProc::ChipImgProc
)                                           # Link the target to ChipImgProc.
# Now you can access all ChipImgProc and dependent features in "foo" target, include Nucleona, Boost, OpenCV etc.
```
see http://gitlab.centrilliontech.com.tw:10080/centrillion/submodule-ci/blob/ChipImgProc/CMakeLists.txt for a runable project