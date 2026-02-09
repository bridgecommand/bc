# Chart-to-world converter tool (requires GDAL)
if(GDAL_FOUND)
    add_executable(bc-chart-converter
        main.cpp
        ${CMAKE_SOURCE_DIR}/ChartReader.cpp
        ${CMAKE_SOURCE_DIR}/HeightmapGenerator.cpp
        ${CMAKE_SOURCE_DIR}/WorldGenerator.cpp
    )

    target_include_directories(bc-chart-converter PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${GDAL_INCLUDE_DIRS}
    )

    target_compile_definitions(bc-chart-converter PRIVATE WITH_GDAL)

    target_link_libraries(bc-chart-converter PRIVATE ${GDAL_LIBRARIES})

    message(STATUS "bc-chart-converter will be built (GDAL found)")
else()
    message(STATUS "bc-chart-converter skipped (GDAL not found)")
endif()
