screw_path_to_target_name(__screw_rel_src_file __screw_target)
screw_add_executable(${__screw_target} ${__screw_src_file})
target_link_libraries(
    ${__screw_target}
    GTest::gtest 
    GTest::main
    Nucleona::Nucleona
    ChipImgProc-utils
    ${CMAKE_THREAD_LIBS_INIT}
)
target_include_directories(${__screw_target} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
screw_add_launch_task(${__screw_target})
add_test(
    NAME ${__screw_target}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/workdir 
    COMMAND ${CMAKE_INSTALL_PREFIX}/bin/${__screw_target}
)