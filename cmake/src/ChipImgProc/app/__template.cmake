screw_path_to_target_name(__screw_rel_src_file __screw_target)
screw_add_executable(${__screw_target} ${__screw_src_file})
target_include_directories(${__screw_target} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(${__screw_target} PUBLIC
    ChipImgProc-utils
    ChipImgProc-sharpness
    Boost::program_options
    Nucleona::Nucleona
    ${CMAKE_THREAD_LIBS_INIT}
)
target_compile_definitions(${__screw_target} PRIVATE NUCLEONA_RANGE_USE_V3)
screw_add_launch_task(${__screw_target})