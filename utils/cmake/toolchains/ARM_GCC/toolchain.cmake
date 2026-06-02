# Official ARM GNU Embedded Toolchain path (macOS)
if(CMAKE_HOST_APPLE) # Set location of ARM GNU Embedded Toolchain for macOS
    set(ARM_TOOLCHAIN_PATH "/Applications/ArmGNUToolchain/15.2.rel1/arm-none-eabi")
endif()

# Look for toolchain binaries in the official installation first, then system PATH
find_program(ARM_NONE_EABI_RANLIB arm-none-eabi-ranlib PATHS "${ARM_TOOLCHAIN_PATH}/bin" NO_DEFAULT_PATH)
find_program(ARM_NONE_EABI_AR arm-none-eabi-ar PATHS "${ARM_TOOLCHAIN_PATH}/bin" NO_DEFAULT_PATH)
find_program(ARM_NONE_EABI_GCC arm-none-eabi-gcc PATHS "${ARM_TOOLCHAIN_PATH}/bin" NO_DEFAULT_PATH)
find_program(ARM_NONE_EABI_GPP arm-none-eabi-g++ PATHS "${ARM_TOOLCHAIN_PATH}/bin" NO_DEFAULT_PATH)
find_program(ARM_NONE_EABI_OBJCOPY arm-none-eabi-objcopy PATHS "${ARM_TOOLCHAIN_PATH}/bin" NO_DEFAULT_PATH)
find_program(ARM_NONE_EABI_SIZE arm-none-eabi-size PATHS "${ARM_TOOLCHAIN_PATH}/bin" NO_DEFAULT_PATH)

# If not found in the official path, try system PATH
if(NOT ARM_NONE_EABI_GCC)
    find_program(ARM_NONE_EABI_RANLIB arm-none-eabi-ranlib)
    find_program(ARM_NONE_EABI_AR arm-none-eabi-ar)
    find_program(ARM_NONE_EABI_GCC arm-none-eabi-gcc)
    find_program(ARM_NONE_EABI_GPP arm-none-eabi-g++)
    find_program(ARM_NONE_EABI_OBJCOPY arm-none-eabi-objcopy)
    find_program(ARM_NONE_EABI_SIZE arm-none-eabi-size)
endif()

# For cross-compilation on macOS, disable macOS-specific settings
# CMAKE_OSX_SYSROOT should not be set for ARM cross-compilation
set(CMAKE_OSX_SYSROOT "")
set(CMAKE_OSX_DEPLOYMENT_TARGET "")

set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_SYSTEM_VERSION "2.0.0")

# Set sysroot for cross-compilation (empty for ARM bare-metal)
set(CMAKE_SYSROOT "")

set(CODAL_TOOLCHAIN "ARM_GCC")

if(CMAKE_VERSION VERSION_LESS "3.5.0")
    include(CMakeForceCompiler)
    cmake_force_c_compiler("${ARM_NONE_EABI_GCC}" GNU)
    cmake_force_cxx_compiler("${ARM_NONE_EABI_GPP}" GNU)
else()
    # from 3.5 the force_compiler macro is deprecated: CMake can detect
    # arm-none-eabi-gcc as being a GNU compiler automatically
    set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
    set(CMAKE_C_COMPILER "${ARM_NONE_EABI_GCC}")
    set(CMAKE_CXX_COMPILER "${ARM_NONE_EABI_GPP}")
endif()

SET(CMAKE_AR "${ARM_NONE_EABI_AR}" CACHE FILEPATH "Archiver")
SET(CMAKE_RANLIB "${ARM_NONE_EABI_RANLIB}" CACHE FILEPATH "rlib")
set(CMAKE_CXX_OUTPUT_EXTENSION ".o")
