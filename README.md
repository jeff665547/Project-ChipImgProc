# Import ChipImgProc to your project

## Import with CMake Hunter (recommanded)
### Requirment
* Your project must be a CMake project
* GCC >= 7.3
* CMake >= 3.10

### General hunter configuration

1. Download Hunter Gate from [here](http://gitlab.centrilliontech.com.tw:10080/centrillion/gate/blob/URL-git-commit/cmake/HunterGate.cmake).
2. Put HunterGate.cmake to (your_project)/cmake/ directory.
3. Add following code into CMakeLists.txt and before ```project(...)```.
```
include(cmake/HunterGate.cmake)
HunterGate(
    URL "ssh://git@gitlab.centrilliontech.com.tw:10022/centrillion/hunter.git"
    SHA1 9aeebe60cc9a04410cf8dad9ccdbf0e306bd37e1
)
```

### Add ChipImgProc dependencies to your project

Add following code after ```project(...)```.
```
hunter_add_package(ChipImgProc)
find_package(ChipImgProc CONFIG REQUIRED)
```

### A full example
(TODO)
