
add_library(bc-asio INTERFACE)

target_include_directories(bc-asio INTERFACE
    ${CMAKE_SOURCE_DIR}/libs/asio/include/
)

target_compile_definitions(bc-asio INTERFACE
    -DASIO_STANDALONE
    -DASIO_HAS_STD_THREAD
)
