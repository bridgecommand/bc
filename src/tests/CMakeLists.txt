include(FetchContent)
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# Build a static library of testable BC source files.
# These only need Irrlicht headers (for type aliases like irr::f32),
# not the full Irrlicht runtime library.
set(BC_TESTABLE_SOURCES
    ${CMAKE_SOURCE_DIR}/Angles.cpp
    ${CMAKE_SOURCE_DIR}/Utilities.cpp
    ${CMAKE_SOURCE_DIR}/IniFile.cpp
    ${CMAKE_SOURCE_DIR}/ScenarioDataStructure.cpp
    ${CMAKE_SOURCE_DIR}/MMGPhysicsModel.cpp
    ${CMAKE_SOURCE_DIR}/LegacyPhysicsModel.cpp
    test_stubs.cpp
)

set(BC_TEST_SOURCES
    test_angles.cpp
    test_utilities.cpp
    test_mmg_physics.cpp
    test_legacy_physics.cpp
)

# Conditionally add chart/world generation if GDAL is available
if(GDAL_FOUND)
    list(APPEND BC_TESTABLE_SOURCES
        ${CMAKE_SOURCE_DIR}/ChartReader.cpp
        ${CMAKE_SOURCE_DIR}/HeightmapGenerator.cpp
        ${CMAKE_SOURCE_DIR}/WorldGenerator.cpp
    )
    list(APPEND BC_TEST_SOURCES
        test_chart_reader.cpp
        test_heightmap_generator.cpp
        test_world_generator.cpp
    )
endif()

add_library(bc-testable STATIC ${BC_TESTABLE_SOURCES})
target_include_directories(bc-testable PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libs/Irrlicht/irrlicht-svn/include
)
target_compile_definitions(bc-testable PRIVATE _CRT_SECURE_NO_WARNINGS)
if (APPLE)
    target_link_libraries(bc-testable PRIVATE "-framework CoreServices")
endif()
if(GDAL_FOUND)
    target_compile_definitions(bc-testable PUBLIC WITH_GDAL)
    target_include_directories(bc-testable PUBLIC ${GDAL_INCLUDE_DIRS})
    target_link_libraries(bc-testable PUBLIC ${GDAL_LIBRARIES})
endif()

add_executable(bc-tests ${BC_TEST_SOURCES})
target_include_directories(bc-tests PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libs/Irrlicht/irrlicht-svn/include
)
target_link_libraries(bc-tests PRIVATE
    Catch2::Catch2WithMain
    bc-testable
)

include(CTest)
include(Catch)
catch_discover_tests(bc-tests)
