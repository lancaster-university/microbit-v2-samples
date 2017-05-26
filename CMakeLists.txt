cmake_minimum_required(VERSION 3.3)

# include toolchain file
include(utils/cmake/toolchains/arm-gcc.cmake)

# required to force settings...
project(codal)

set(CODAL_BUILD_SYSTEM TRUE)
set(CODAL_UTILS_LOCATION "${PROJECT_SOURCE_DIR}/utils/cmake/util.cmake")

# include additional cmake
include(utils/cmake/JSONParser.cmake)
include(utils/cmake/util.cmake)
include(utils/cmake/colours.cmake)

file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/build")

# configure output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/build")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build")

#read our config file...
file(READ "${PROJECT_SOURCE_DIR}/codal.json" codal_json)
sbeParseJson(codal codal_json)

if("${codal.target.platform}" STRGREATER "")
    message("${BoldBlue}Targeting ${codal.target.device} which is a ${codal.target.processor}, built on ${codal.target.platform}. ${ColourReset}")
    file(READ "${PROJECT_SOURCE_DIR}/targets/${codal.target.processor}_${codal.target.device}_${codal.target.platform}.json" device_json)
else()
    message("${BoldBlue}Targeting ${codal.target.device} which is a ${codal.target.processor}, built natively. ${ColourReset}")
    file(READ "${PROJECT_SOURCE_DIR}/targets/${codal.target.processor}_${codal.target.device}.json" device_json)
endif()

sbeParseJson(device device_json)

message("${EXPLICIT_INCLUDES}")

#define any additional symbols specified by the target.
if("${device.definitions}" STRGREATER "")
    add_definitions("${device.definitions}")
endif()


SET(CODAL_DEFINITIONS "")

#extract any definitions specified in the codal.json definitions object, and add them
foreach(var ${codal})

    #if it is not prefixed by codal.definitions, do not consider the key.
    if(NOT "${var}" MATCHES "codal\.definitions\.")
        continue()
    endif()

    string(REGEX MATCH "[^codal\.definitions\.]([A-Z,a-z,0-9,_,]+)" CODAL_DEFINITION "${var}")

    #check if string or number
    if(${${var}} MATCHES "[0-9]+")
        set(CODAL_DEFINITION_VALUE "${${var}}")
    else()
        set(CODAL_DEFINITION_VALUE "\"${${var}}\"")
    endif()

    set(CODAL_DEFINITIONS "${CODAL_DEFINITIONS} #define ${CODAL_DEFINITION}\t ${CODAL_DEFINITION_VALUE}\n")
endforeach()

# create a header file from the definitions specified in JSON
if("${CODAL_DEFINITIONS}" STRGREATER "")
    set(EXTRA_INCLUDES_PATH "${PROJECT_SOURCE_DIR}/build/codal_extra_definitions.h")
    file(WRITE "${EXTRA_INCLUDES_PATH}" ${CODAL_DEFINITIONS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -imacros ${EXTRA_INCLUDES_PATH}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -imacros ${EXTRA_INCLUDES_PATH}")
endif()

####################
# optional JSON flags for compilation + assembly
###################
if("${device.cpu_opts}" STRGREATER "")
    set(_CPU_COMPILATION_OPTIONS "${device.cpu_opts}")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${device.cpu_opts}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${device.cpu_opts}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${device.cpu_opts}")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${device.cpu_opts}")
endif()

set(_C_FAMILY_FLAGS_INIT "-fno-exceptions -fno-unwind-tables -ffunction-sections -fdata-sections -Wall -Wextra")

# asm
if("${device.asm_flags}" STRGREATER "")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${device.asm_flags}")
endif()

# c
if("${device.c_flags}" STRGREATER "")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${device.c_flags}")
    set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} ${device.c_flags}")
endif()

# cpp
if("${device.cpp_flags}" STRGREATER "")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${device.cpp_flags}")
    set(CMAKE_CXX_LINK_FLAGS " ${device.cpp_flags}")
endif()

# linker opts
if("${device.linker_flags}" STRGREATER "")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${device.linker_flags}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${device.linker_flags}")
endif()
####################

# Define any cmake variables the user has requested
if("${device.cmake_definitions}" STRGREATER "")
    set(CMAKE_DEFS ${device.cmake_definitions})
    foreach(i ${CMAKE_DEFS})
        set(${device.cmake_definitions_${i}.variable} "${device.cmake_definitions_${i}.value}")
    endforeach()
endif()

# this variable is used in the linking step of the final binary.
set(LIB_FOLDERS "")
set(CODAL_DEPS "")

if(NOT "${device.target.url}" STRGREATER "")
    message("${BoldRed}Target not correctly specified, exiting.${ColourReset}")
endif()

SET(CODAL_TARGET_NAME ${device.target.name})
SET(CODAL_OUTPUT_NAME ${device.device})
SET(CODAL_TARGET_PROCESSOR ${device.processor})
SET(CODAL_TARGET_CPU_ARCHITECTURE ${device.architecture})

#install the target
INSTALL_DEPENDENCY(${LIB_DEST} ${device.target.name} ${device.target.url} ${device.target.branch} ${device.target.type})
message("${BoldMagenta}Set target: ${device.target.name} ${ColourReset}")
list(APPEND CODAL_DEPS ${device.target.name})

# "import" and add any specified libraries to the build list
if("${device.libraries}" STRGREATER "")
    message("Installing dependencies...")
    set(DEVICE_LIBS ${device.libraries})

    foreach(i ${DEVICE_LIBS})
        SET(BRANCH "NONE")
        if("${device.libraries_${i}.branch}" STRGREATER "")
            SET(BRANCH "${device.libraries_${i}.branch}")
        endif()

        INSTALL_DEPENDENCY(${LIB_DEST} ${device.libraries_${i}.name} ${device.libraries_${i}.url} ${BRANCH} ${device.libraries_${i}.type})
        list(APPEND CODAL_DEPS "${device.libraries_${i}.name}")
    endforeach()

    foreach(dep ${CODAL_DEPS})
        message("${BoldGreen}Using library: ${dep}${ColourReset}")
        add_subdirectory("${PROJECT_SOURCE_DIR}/${LIB_DEST}/${dep}")
    endforeach()
endif()

#finally, find sources and includes of the application, and create a target.
RECURSIVE_FIND_DIR(INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/${codal.application}" "*.h")
RECURSIVE_FIND_FILE(SOURCE_FILES "${PROJECT_SOURCE_DIR}/${codal.application}" "*.c??")

add_executable(
    ${device.device}
    ${SOURCE_FILES}
)

if("${INCLUDE_DIRS}" STRGREATER "")
    target_include_directories(${device.device} "${INCLUDE_DIRS}")
endif()


set_target_properties(${device.device} PROPERTIES SUFFIX "" ENABLE_EXPORTS ON)

# link the executable with supporting libraries.
target_link_libraries(
    ${device.device}
    ${CODAL_DEPS}
)

add_custom_command(
    OUTPUT "${PROJECT_SOURCE_DIR}/${codal.output_folder}/${device.device}.bin"
    COMMAND "${ARM_NONE_EABI_OBJCOPY}" -O binary "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${device.device}" "${PROJECT_SOURCE_DIR}/${codal.output_folder}/${device.device}.bin"
    DEPENDS  ${device.device}
    COMMENT "converting to bin file."
)

# turn our bin to a hex.
add_custom_command(
    OUTPUT "${PROJECT_SOURCE_DIR}/${codal.output_folder}/${device.device}.hex"
    COMMAND "${ARM_NONE_EABI_OBJCOPY}" -O ihex "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${device.device}" "${PROJECT_SOURCE_DIR}/${codal.output_folder}/${device.device}.hex"
    DEPENDS  ${device.device}
    COMMENT "converting to hex file."
)

#specify a dependency on the elf file so that hex & bin are automatically rebuilt when elf is changed.
add_custom_target(${device.device}_bin ALL DEPENDS "${PROJECT_SOURCE_DIR}/${codal.output_folder}/${device.device}.bin")
add_custom_target(${device.device}_hex ALL DEPENDS "${PROJECT_SOURCE_DIR}/${codal.output_folder}/${device.device}.hex")

# post process command hook, depends on the hex file generated by the build system.
if("${codal.post_process}" STRGREATER "")

    # replace specific strings in the command, this gives users flexibility, they don't have to manually specify the location of files
    string(REPLACE "<OUTPUT_HEX_LOCATION>" ${PROJECT_SOURCE_DIR}/${codal.output_folder}/${device.device}.hex  CODAL_POSTPROCESS_COMMAND ${codal.post_process})
    string(REPLACE "<OUTPUT_HEX_DESTINATION>" ${PROJECT_SOURCE_DIR}/${codal.output_folder} CODAL_POSTPROCESS_COMMAND ${CODAL_POSTPROCESS_COMMAND})
    string(REPLACE "<OUTPUT_HEX_NAME>" ${device.device} CODAL_POSTPROCESS_COMMAND ${CODAL_POSTPROCESS_COMMAND})

    string(REPLACE "<OUTPUT_BIN_LOCATION>" ${PROJECT_SOURCE_DIR}/${codal.output_folder}/${device.device}.bin  CODAL_POSTPROCESS_COMMAND ${CODAL_POSTPROCESS_COMMAND})
    string(REPLACE "<OUTPUT_BIN_DESTINATION>" ${PROJECT_SOURCE_DIR}/${codal.output_folder} CODAL_POSTPROCESS_COMMAND ${CODAL_POSTPROCESS_COMMAND})
    string(REPLACE "<OUTPUT_BIN_NAME>" ${device.device}.bin CODAL_POSTPROCESS_COMMAND ${CODAL_POSTPROCESS_COMMAND})

    #convert to a command
    separate_arguments(FINAL_COMMAND UNIX_COMMAND ${CODAL_POSTPROCESS_COMMAND})

    # execute
    add_custom_command(
        TARGET ${device.device}_bin
        COMMAND ${FINAL_COMMAND}
        DEPENDS  ${device.device}_bin
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Executing post process command"
    )
endif()
