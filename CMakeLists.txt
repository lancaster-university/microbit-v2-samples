cmake_minimum_required(VERSION 3.3)

find_program(ARM_NONE_EABI_RANLIB arm-none-eabi-ranlib)
find_program(ARM_NONE_EABI_AR arm-none-eabi-ar)
find_program(ARM_NONE_EABI_GCC arm-none-eabi-gcc)
find_program(ARM_NONE_EABI_GPP arm-none-eabi-g++)
find_program(ARM_NONE_EABI_OBJCOPY arm-none-eabi-objcopy)

set(CMAKE_OSX_SYSROOT "/")
set(CMAKE_OSX_DEPLOYMENT_TARGET "")

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
    message("Adding defs: ${device.definitions}")
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

add_executable(${device.device} "${PROJECT_SOURCE_DIR}/${codal.application}/main.cpp")
target_link_libraries(${device.device} ${CODAL_DEPS})

# turn our bin to a hex.
add_custom_command(
    OUTPUT "${PROJECT_SOURCE_DIR}/${codal.output_folder}/${device.device}.hex"
    COMMAND "${ARM_NONE_EABI_OBJCOPY}" -O ihex "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${device.device}" "${PROJECT_SOURCE_DIR}/${codal.output_folder}/${device.device}.hex"
    DEPENDS ${device.device}
    COMMENT "converting to hex file."
)

#specify a dependency on the binary file so that hex is automatically rebuilt when bin is changed.
add_custom_target(${device.device}_hex ALL DEPENDS "${PROJECT_SOURCE_DIR}/${codal.output_folder}/${device.device}.hex")
