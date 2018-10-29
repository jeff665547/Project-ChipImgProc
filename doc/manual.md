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
    SHA1 b06f89371cee76e7055f1e0e583c82e9cb583e93
)
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
    SHA1 b06f89371cee76e7055f1e0e583c82e9cb583e93
)                                           # Download Hunter with specific version(commit).
project(ChipImgProc-example)

hunter_add_package(ChipImgProc)             # Download ChipImgProc package.
find_package(ChipImgProc CONFIG REQUIRED)   # Find and configure the package.
add_executable(foo foo.cpp)                 # Create an executable target.
target_link_libraries(
    foo PUBLIC ChipImgProc::ChipImgProc
)                                           # Link the target to ChipImgProc.
# Now you can access all ChipImgProc and dependent features in "foo" target, include Nucleona, Boost, OpenCV etc.
```
http://gitlab.centrilliontech.com.tw:10080/centrillion/submodule-ci/blob/ChipImgProc/CMakeLists.txt