
set(IRRLICHT_HOME "${CMAKE_CURRENT_SOURCE_DIR}/irrlicht-svn")

include(ExternalProject)
ExternalProject_Add(bc-irrlicht-internal
    SOURCE_DIR "${IRRLICHT_HOME}/source/Irrlicht/"
    BINARY_DIR "${IRRLICHT_HOME}/source/Irrlicht/"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${MAKE}
    INSTALL_COMMAND ""
)

add_library(bc-irrlicht INTERFACE)
add_dependencies(bc-irrlicht bc-irrlicht-internal)

target_include_directories(bc-irrlicht INTERFACE
    "${IRRLICHT_HOME}/include/"
)

link_directories(
    /usr/X11R6/lib64 # or lib32
)

target_link_libraries(bc-irrlicht INTERFACE
    "${IRRLICHT_HOME}/source/Irrlicht/libIrrlicht.a"
    GL
    Xxf86vm
    Xext
    X11
    Xcursor
)
