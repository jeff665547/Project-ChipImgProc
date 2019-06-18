screw_extend_template()
if(ENABLE_LOG)
    target_compile_definitions(${__screw_target} PUBLIC 
        CHIPIMGPROC_ENABLE_LOG
    )
endif()