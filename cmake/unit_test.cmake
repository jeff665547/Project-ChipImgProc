if( BUILD_TESTS)
    enable_testing()
    screw_add_targets_in("unit_test" __exe_targets __lib_targets)
    set(lib_targets ${lib_targets} ${__lib_targets})
    set(exe_targets ${exe_targets} ${__exe_targets})

    message(STATUS "CI_JOB_TOKEN: $ENV{CI_JOB_TOKEN}")
    include(ExternalProject)
    # ExternalProject_Add(TestData
    #     URL "http://gitlab.centrilliontech.com.tw:10080/centrillion/ChipImgProcTestData/repository/v0.0.1/archive.tar.gz?private_token=$ENV{CI_JOB_TOKEN}"
    #     CMAKE_ARGS
    #         -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/unit_test
    # )
    ExternalProject_Add(TestData
        GIT_REPOSITORY 
            "https://gitlab-ci-token:Zbx3JboF4u3kMAWSLjSS@gitlab.centrilliontech.com.tw:10022/centrillion/ChipImgProcTestData" 
        GIT_TAG
            "900c1caba9f4955eab112624154d6c1f9c6e239d"
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/unit_test
    )
endif()