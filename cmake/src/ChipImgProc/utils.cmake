screw_extend_template()
target_link_libraries(${__screw_target} PUBLIC
    Boost::system
    Boost::filesystem
    ${OpenCV_LIBS}
    cpp_base64-base64
)