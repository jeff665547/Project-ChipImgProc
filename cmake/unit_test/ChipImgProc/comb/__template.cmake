screw_extend_template()
target_link_libraries(${__screw_target}
    ChipImgProc-hough_transform
    ChipImgProc-stitch
    cpp_base64-base64
)
if(PROFILER)
    target_link_libraries(${__screw_target}
        profiler
        -Wl,--no-as-needed
    )
    target_compile_definitions(${__screw_target} PUBLIC
        PROFILER
    )
#     if(GNU)
#         target_link_options(${__screw_target} PUBLIC LINKER:--no-as-needed)
#     endif()
#     target_compile_options(${__screw_target} PUBLIC
#         -Wl,--no-as-needed
#     )
endif()
