if( BUILD_TESTS)
    screw_add_targets_in("unit_test" __exe_targets __lib_targets)
    set(lib_targets ${lib_targets} ${__lib_targets})
endif()