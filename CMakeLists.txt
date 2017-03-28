cmake_minimum_required(VERSION 3.3)

find_program(ARM_NONE_EABI_RANLIB arm-none-eabi-ranlib)
find_program(ARM_NONE_EABI_AR arm-none-eabi-ar)
find_program(ARM_NONE_EABI_GCC arm-none-eabi-gcc)
find_program(ARM_NONE_EABI_GPP arm-none-eabi-g++)
find_program(ARM_NONE_EABI_OBJCOPY arm-none-eabi-objcopy)

set(LIB_DEST "libraries")

if(CMAKE_VERSION VERSION_LESS "3.5.0")
    include(CMakeForceCompiler)
    cmake_force_c_compiler("${ARM_NONE_EABI_GCC}" GNU)
    cmake_force_cxx_compiler("${ARM_NONE_EABI_GPP}" GNU)
else()
    # from 3.5 the force_compiler macro is deprecated: CMake can detect
    # arm-none-eabi-gcc as being a GNU compiler automatically
    set(CMAKE_C_COMPILER "${ARM_NONE_EABI_GCC}")
    set(CMAKE_CXX_COMPILER "${ARM_NONE_EABI_GPP}")
endif()

SET(CMAKE_AR "${ARM_NONE_EABI_AR}" CACHE FILEPATH "Archiver")
SET(CMAKE_RANLIB "${ARM_NONE_EABI_RANLIB}" CACHE FILEPATH "rlib")
set(CMAKE_CXX_OUTPUT_EXTENSION ".o")

# required to force settings...
project(codal)

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

#foreach(var ${codal})
#    message("${var} = ${${var}}")
#endforeach()

if("${codal.target.platform}" STRGREATER "")
    message("${BoldBlue}Targeting ${codal.target.device} which is a ${codal.target.processor}, built on ${codal.target.platform}. ${ColourReset}")
    file(READ "${PROJECT_SOURCE_DIR}/target/${codal.target.platform}/${codal.target.processor}/${codal.target.device}/device.json" device_json)
else()
    message("${BoldBlue}Targeting ${codal.target.device} which is a ${codal.target.processor}, built natively. ${ColourReset}")
    file(READ "${PROJECT_SOURCE_DIR}/target/${codal.target.manufacturer}/${codal.target.processor}/${codal.target.device}/device.json" device_json)
endif()

sbeParseJson(device device_json)

#foreach(var ${device})
#    message("${var} = ${${var}}")
#endforeach()

set(TARGET_PROCESSOR_DIRECTORY "${PROJECT_SOURCE_DIR}/target/${device.manufacturer}/${device.processor}")

#include processor specific files
include_directories("${TARGET_PROCESSOR_DIRECTORY}")
SOURCE_FILES(SOURCE ${TARGET_PROCESSOR_DIRECTORY})

# if built on something...
if("${codal.target.platform}" STRGREATER "")
    set(TARGET_SOFTWARE_DIRECTORY "${PROJECT_SOURCE_DIR}/target/${codal.target.platform}/${device.processor}/${device.device}")
# otherwise use the native implementation
else()
    set(TARGET_SOFTWARE_DIRECTORY "${PROJECT_SOURCE_DIR}/target/${device.manufacturer}/${device.processor}/${device.device}")
endif()

#include software libs
RECURSIVE_FIND_DIR(INCLUDE_DIRS "${TARGET_SOFTWARE_DIRECTORY}" "*.h")
RECURSIVE_FIND_FILE(SOURCE_FILES "${TARGET_SOFTWARE_DIRECTORY}" "*.c??")

SOURCE_FILES(SOURCE ${SOURCE_FILES})
include_directories("${INCLUDE_DIRS}")

# Find headers within Codal and include them
RECURSIVE_FIND_DIR(INCLUDES "${PROJECT_SOURCE_DIR}/${codal.includes}" "*.h")
include_directories("${INCLUDES}")


#define any additional symbols specified by the target.
if("${device.definitions}" STRGREATER "")
    #message("Adding defs: ${device.definitions}")
    add_definitions("${device.definitions}")
endif()

#define any additional symbols specified by codal.
if("${codal.definitions}" STRGREATER "")
    #message("Adding defs: ${codal.definitions}")
    add_definitions("${codal.definitions}")
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
endif()

# cpp
if("${device.cpp_flags}" STRGREATER "")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${device.cpp_flags}")
endif()

# linker opts
if("${device.linker_flags}" STRGREATER "")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${device.linker_flags}")
endif()
####################

if("${device.extends}" STRGREATER "")
    message("Opening file: ${PROJECT_SOURCE_DIR}/cmake/base/${device.extends}.json")
    #file(READ "${PROJECT_SOURCE_DIR}/utils/cmake/base/${device.extends}" extended)
endif()

# Define any cmake variables the user has requested
if("${device.cmake_definitions}" STRGREATER "")
    set(CMAKE_DEFS ${device.cmake_definitions})
    foreach(i ${CMAKE_DEFS})
        set(${device.cmake_definitions_${i}.variable} "${device.cmake_definitions_${i}.value}")
    endforeach()
endif()

# this variable is used in the linking step of CODAL.
set(CODAL_DEPS "")

# if we're building on top of a platform, import files and build. The library name must
# be the same as the platform name specified in JSON.
if("${codal.target.platform}" STRGREATER "")
    add_subdirectory("${PROJECT_SOURCE_DIR}/target/${codal.target.platform}/")
    list(APPEND CODAL_DEPS "${codal.target.platform}")
endif()

set(LIB_FOLDERS "")

# "import" and add any specified libraries to the build list
if("${device.libraries}" STRGREATER "")
    message("${Magenta}Installing dependencies...${ColourReset}")
    set(DEVICE_LIBS ${device.libraries})
    foreach(i ${DEVICE_LIBS})
        SET(BRANCH "NONE")
        if("${device.libraries_${i}.branch}" STRGREATER "")
            SET(BRANCH "${device.libraries_${i}.branch}")
        endif()
        INSTALL_DEPENDENCY(${LIB_DEST} ${device.libraries_${i}.name} ${device.libraries_${i}.url} ${BRANCH} ${device.libraries_${i}.type})
        list(APPEND CODAL_DEPS "${device.libraries_${i}.name}")
        list(APPEND LIB_FOLDERS "${PROJECT_SOURCE_DIR}/${LIB_DEST}/${device.libraries_${i}.name}")
    endforeach()

    SUB_DIRS(CODAL_LIB_SOURCE "${LIB_DEST}")
    foreach(dir ${CODAL_LIB_SOURCE})
        message("${Magenta}Adding library: ${dir}${ColourReset}")
        add_subdirectory("${dir}")
    endforeach()
endif()

# add our source files last, using the CMake in ./source
# the cmake here will use CODAL_DEPS when linking
add_subdirectory("${codal.source}")

set(CMAKE_CXX_CREATE_STATIC_LIBRARY "<CMAKE_AR> -cr <LINK_FLAGS> <TARGET> <OBJECTS>")
set(CMAKE_CXX_COMPILE_OBJECT        "<CMAKE_CXX_COMPILER> <DEFINES> ${EXPLICIT_INCLUDES}<FLAGS> -o <OBJECT> -c <SOURCE>")
# <LINK_LIBRARIES> is grouped with system libraries so that system library
# functions (e.g. malloc) can be overridden by symbols in <LINK_LIBRARIES>
set(CMAKE_CXX_LINK_EXECUTABLE       "<CMAKE_CXX_COMPILER> ${CMAKE_LINKER_FLAGS} -Wl,-Map,<TARGET>.map -Wl,--start-group <OBJECTS> <LINK_LIBRARIES> ${GLOBALLY_LINKED_TARGET_LIBS} -lstdc++ -lsupc++ -lm -lc -lgcc -lstdc++ -lsupc++ -lm -lc -lgcc -Wl,--end-group  --specs=nano.specs -o <TARGET>")

set(CMAKE_CXX_FLAGS_DEBUG_INIT          "-g -gdwarf-3")
set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE_INIT        "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-Os -g -gdwarf-3 -DNDEBUG")
set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")

add_executable(codal_${device.device} "${PROJECT_SOURCE_DIR}/${codal.application}/main.cpp")
target_link_libraries(codal_${device.device} codal)

# turn our bin to a hex.
add_custom_command(
    OUTPUT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/codal_${device.device}.hex"
    COMMAND "${ARM_NONE_EABI_OBJCOPY}" -O ihex "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/codal_${device.device}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/codal_${device.device}.hex"
    DEPENDS codal_${device.device}
    COMMENT "converting to hex file."
)

#specify a dependency on the binary file so that hex is automatically rebuilt when bin is changed.
add_custom_target(codal_${device.device}_hex ALL DEPENDS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/codal_${device.device}.hex")
