if( BUILD_TESTS)
    enable_testing()
    screw_add_targets_in("unit_test" __exe_targets __lib_targets)
    set(lib_targets ${lib_targets} ${__lib_targets})
    set(exe_targets ${exe_targets} ${__exe_targets})

    include(ExternalProject)
    ExternalProject_Add(TestData
        GIT_REPOSITORY 
            "ssh://git@gitlab.centrilliontech.com.tw:10022/centrillion/ChipImgProcTestData.git" 
        GIT_TAG
            "900c1caba9f4955eab112624154d6c1f9c6e239d"
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/unit_test
    )
endif()