
add_library(bc-aisparser
    c/src/access.c
    c/src/imo.c
    c/src/nmea.c
    c/src/seaway.c
    c/src/sixbit.c
    c/src/vdm_parse.c
)

target_include_directories(bc-aisparser PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/c/src/"
)
