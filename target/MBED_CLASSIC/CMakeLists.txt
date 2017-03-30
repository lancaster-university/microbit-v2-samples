project(MBED_CLASSIC)

include(../../utils/cmake/util.cmake)

RECURSIVE_FIND_DIR(INCLUDE_DIRS "inc" "*.h")
RECURSIVE_FIND_FILE(SOURCE_FILES "source" "*.c??")

include_directories("${INCLUDE_DIRS}")

add_library(MBED_CLASSIC "${SOURCE_FILES}")

target_link_libraries(
    MBED_CLASSIC
    mbed-classic
    codal
)

target_include_directories(MBED_CLASSIC PUBLIC ${INCLUDE_DIRS})
