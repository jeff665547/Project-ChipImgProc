set(namespace "${PROJECT_NAME}::")

install( 
    TARGETS ${exe_targets} ${test_targets} ${lib_targets}
    EXPORT ${PROJECT_NAME}-targets
    BUNDLE DESTINATION . COMPONENT Runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Library
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Library
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
)

install( FILES ${PUBLIC_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Headers
)

# GNUInstallDirs "DATADIR" wrong here; CMake search path wants "share".
set(BIOVOLTRON_CMAKECONFIG_INSTALL_DIR "share/cmake/${PROJECT_NAME}" CACHE STRING "install path for summitConfig.cmake")

if (NOT CMAKE_VERSION VERSION_LESS 3.0)
  export(EXPORT ${PROJECT_NAME}-targets
         FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
endif()
configure_package_config_file(${CMAKE_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${BIOVOLTRON_CMAKECONFIG_INSTALL_DIR})
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${${PROJECT_NAME}_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(
    EXPORT ${PROJECT_NAME}-targets
    NAMESPACE ${namespace}
    DESTINATION ${BIOVOLTRON_CMAKECONFIG_INSTALL_DIR}
    COMPONENT TargetFiles
)
install(
    FILES 
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${BIOVOLTRON_CMAKECONFIG_INSTALL_DIR}
    COMPONENT ConfigFiles
)
screw_get_bits(BITS)
# bin and runtime dir setting goes here, this is for bundle
unset(BITS)
configure_file(cmake/bundle.cmake.in ${CMAKE_BINARY_DIR}/bundle.cmake @ONLY)
install(
    SCRIPT ${CMAKE_BINARY_DIR}/bundle.cmake
    COMPONENT Runtime
)
