if( BUILD_TESTS)
    enable_testing()
    screw_add_targets_in("unit_test" __exe_targets __lib_targets)
    set(lib_targets ${lib_targets} ${__lib_targets})
    set(exe_targets ${exe_targets} ${__exe_targets})

    include(ExternalProject)
    ExternalProject_Add(TestData
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/lib/ChipImgProcTestData
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/unit_test
    )
endif()