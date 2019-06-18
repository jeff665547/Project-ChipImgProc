screw_path_to_target_name(__screw_rel_src_file __screw_target)
screw_add_library(${__screw_target} ${__screw_src_file})
target_include_directories(${__screw_target} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
if(NOT "${__screw_target}" STREQUAL ChipImgProc-utils)
    target_link_libraries(${__screw_target} PUBLIC
        ChipImgProc-utils
    )
endif()
if(NOT "${__screw_target}" STREQUAL ChipImgProc-logger)
    target_link_libraries(${__screw_target} PUBLIC
        ChipImgProc-logger
    )
endif()

screw_show_var(__screw_target)