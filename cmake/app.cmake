file( GLOB_RECURSE app_cmakes ${CMAKE_SOURCE_DIR}/cmake/app/*.cmake )
foreach ( ac ${app_cmakes} )
    include ( ${ac} )
endforeach()