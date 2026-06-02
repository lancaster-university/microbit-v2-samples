# macOS Compatible CODAL Compilation
This repo provides a guide to fixing compilation for [CODAL](https://github.com/lancaster-university/microbit-v2-samples) on macOS hosts.

❗️This guide assumes you are using the latest CODAL folder, and that you are working from within it. File paths that have no explicit root are relative to the CODAL folder.

# Fix building
Make sure the usual dependencies are installed first:
- [Git](https://git-scm.com) or `brew install git`
- [CMake](https://cmake.org/download/) or `brew install cmake`
- [Python 3](https://www.python.org/downloads/) or `brew install python3`

## Install Arm Toolchain
macOS requires the webpage GNU Arm Embedded Toolchain download, rather than the default `arm-none-eabi` brew package as this misses some built-in libraries.

- [GNU Arm Embedded Toolchain for macOS](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads#:~:text=arm%2Dgnu%2Dtoolchain%2D15.2.rel1%2Ddarwin%2Darm64%2Darm%2Dnone%2Deabi.tar.xz) 
    -  **Dont use the brew package**
    - Select correct architecture: Apple Silicon (arm64) or Intel (x86_64)
    - Use the **pkg** installer. ~1GB extracted in: `/Applications/ArmGNUToolchain/15.2.rel1/arm-none-eabi/`
        - Using the tarball is possible but requires additional setup to add the toolchain to your PATH and set up the necessary environment variables.

⚠️ Arm Toolchain 15.2.rel1 is the latest version at the time of writing. If you install a different version, make sure to change paths from `"/Applications/ArmGNUToolchain/15.2.rel1/arm-none-eabi"`
## Update toolchain file
*Already included in this repo*
1. `utils/cmake/toolchains/ARM_GCC/toolchain.cmake`
```json
# Define Official ARM GNU Embedded Toolchain path (macOS)
set(ARM_TOOLCHAIN_PATH "/Applications/ArmGNUToolchain/15.2.rel1/arm-none-eabi")

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

```

### Running `python3 build.py` should now work fine

# Developing
You will find a simple main.cpp in the `source` folder which you can edit. CODAL will also compile any other C/C++ header files our source files with the extension `.h .c .cpp` it finds in the source folder.

The `samples` folder contains a number of simple sample programs that utilise you may find useful.

