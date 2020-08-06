
set(IRRLICHT_HOME "${CMAKE_CURRENT_SOURCE_DIR}/irrlicht-svn")

add_library(bc-irrlicht INTERFACE)

if (MSVC)
    target_link_libraries(bc-irrlicht INTERFACE
        "${IRRLICHT_HOME}/lib/Win32-visualstudio/Irrlicht.lib"
    )
else (MSVC)
    include(ExternalProject)
    
    if (APPLE)
        ExternalProject_Add(bc-irrlicht-internal
            SOURCE_DIR "${IRRLICHT_HOME}/source/Irrlicht/"
            BINARY_DIR "${IRRLICHT_HOME}/source/Irrlicht/"
            CONFIGURE_COMMAND ""
            BUILD_COMMAND xcodebuild -project Irrlicht.xcodeproj
            INSTALL_COMMAND cp ${IRRLICHT_HOME}/lib/OSX/libIrrlicht.a ${IRRLICHT_HOME}/source/Irrlicht/
        )
    
        link_directories(
            /usr/X11R6/lib64 # or lib32
        )
        target_link_libraries(bc-irrlicht INTERFACE
            "${IRRLICHT_HOME}/source/Irrlicht/libIrrlicht.a"
            "-framework OpenGL"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
        )
    else (APPLE)
        ExternalProject_Add(bc-irrlicht-internal
            SOURCE_DIR "${IRRLICHT_HOME}/source/Irrlicht/"
            BINARY_DIR "${IRRLICHT_HOME}/source/Irrlicht/"
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${MAKE}
            INSTALL_COMMAND ""
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
    endif (APPLE)

    add_dependencies(bc-irrlicht bc-irrlicht-internal)
endif (MSVC)

target_include_directories(bc-irrlicht INTERFACE
    "${IRRLICHT_HOME}/include/"
)


