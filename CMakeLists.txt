cmake_minimum_required(VERSION 3.3)

# THIS HAS TO COME BEFORE THE PROJECT LINE
#set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

find_program(ARM_NONE_EABI_RANLIB arm-none-eabi-ranlib)
find_program(ARM_NONE_EABI_AR arm-none-eabi-ar)
find_program(ARM_NONE_EABI_GCC arm-none-eabi-gcc)
find_program(ARM_NONE_EABI_GPP arm-none-eabi-g++)
find_program(ARM_NONE_EABI_OBJCOPY arm-none-eabi-objcopy)

set(LIB_DEST "libraries")

if(CMAKE_VERSION VERSION_LESS "3.5.0")
    message("FORCING COMPILER")
    include(CMakeForceCompiler)
    cmake_force_c_compiler("${ARM_NONE_EABI_GCC}" GNU)
    cmake_force_cxx_compiler("${ARM_NONE_EABI_GPP}" GNU)
else()
    # from 3.5 the force_compiler macro is deprecated: CMake can detect
    # arm-none-eabi-gcc as being a GNU compiler automatically
    set(CMAKE_C_COMPILER "${ARM_NONE_EABI_GCC}")
    set(CMAKE_CXX_COMPILER "${ARM_NONE_EABI_GPP}")
endif()
message("before: |${CMAKE_AR}|${CMAKE_RANLIB}|")
message("setting to: ${ARM_NONE_EABI_AR} ${ARM_NONE_EABI_RANLIB}")
SET(CMAKE_AR "${ARM_NONE_EABI_AR}" CACHE FILEPATH "Archiver")
SET(CMAKE_RANLIB "${ARM_NONE_EABI_RANLIB}" CACHE FILEPATH "rlib")
set(CMAKE_CXX_OUTPUT_EXTENSION ".o")

message("C COMPILER : ${CMAKE_C_COMPILER} C++ COMPILER: ${CMAKE_CXX_COMPILER}")

project(my_project VERSION 0.0.0 LANGUAGES C CXX ASM)

include(cmake/JSONParser.cmake)
include(cmake/util/util.cmake)

file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/build")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/build")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build")

file(READ "${PROJECT_SOURCE_DIR}/codal.json" codal_json)
file(READ "${PROJECT_SOURCE_DIR}/device.json" device_json)

sbeParseJson(codal codal_json)

foreach(var ${codal})
    message("${var} = ${${var}}")
endforeach()

sbeParseJson(device device_json)

foreach(var ${device})
    message("${var} = ${${var}}")
endforeach()

#include platform specific includes
include_directories("${PROJECT_SOURCE_DIR}/target/${device.processor}")
include_directories("${PROJECT_SOURCE_DIR}/target/${device.processor}/${device.device}")

message("ADDING SOURCE FILEs")
SOURCE_FILES(SOURCE "${PROJECT_SOURCE_DIR}/target/${device.processor}")
SOURCE_FILES(SOURCE "${PROJECT_SOURCE_DIR}/target/${device.processor}/${device.device}")


#define any additional symbols specified by the target.
if("${device.definitions}" STRGREATER "")
    message("Adding defs: ${device.definitions}")
    add_definitions("${device.definitions}")
endif()

#define any additional symbols specified by codal.
if("${codal.definitions}" STRGREATER "")
    message("Adding defs: ${codal.definitions}")
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
    file(READ "${PROJECT_SOURCE_DIR}/cmake/base/${device.extends}" extended)
endif()

# Define any cmake variables the user has requested
if("${device.cmake_definitions}" STRGREATER "")
    set(CMAKE_DEFS ${device.cmake_definitions})
    foreach(i ${CMAKE_DEFS})
        set(${device.cmake_definitions_${i}.variable} "${device.cmake_definitions_${i}.value}")
    endforeach()
endif()

# "import" and build any specified libraries
if("${device.libraries}" STRGREATER "")
    message("Installing dependencies...")
    set(DEVICE_LIBS ${device.libraries})
    foreach(i ${DEVICE_LIBS})
        INSTALL_DEPENDENCY(${LIB_DEST} ${device.libraries_${i}})
    endforeach()

    SUB_DIRS(CODAL_LIB_SOURCE "${LIB_DEST}")
    foreach(dir ${CODAL_LIB_SOURCE})
        message("Adding library: ${dir}")
        add_subdirectory("${dir}")
        include_directories("${dir}")
    endforeach()
endif()

# Find headers within Codal and include them
HEADER_DIRECTORIES(INCLUDES "${PROJECT_SOURCE_DIR}/${codal.includes}")
include_directories("${INCLUDES}")

# add our source files using the CMake in ./source
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

add_executable(codal_${device.device} "${PROJECT_SOURCE_DIR}/${codal.executables}/main.cpp")
target_link_libraries(codal_${device.device} codal)

execute_process(
    COMMAND arm-none-eabi-objcopy -O ihex codal_${device.device} codal_${device.device}.hex
)
