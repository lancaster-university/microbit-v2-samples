# Clang/LLVM is NOT the officialy supported toolchain for building Micro:Bit V2 CODAL applications.
# This file serves to define an alternative compiler of which work has been put into making
# compatible, but milage may vary in getting a working build. It is recommended to use 
# ARM_GCC where possible, with that being said complete Clang are possible with some tweaking.
# See compiler-flags.cmake

find_program(LLVM_RANLIB llvm-ranlib)
find_program(LLVM_AR llvm-ar)
find_program(CLANG clang)
find_program(CLANG++ clang++)
find_program(LLVM_OBJCOPY llvm-objcopy)

set(CMAKE_OSX_SYSROOT "/")
set(CMAKE_OSX_DEPLOYMENT_TARGET "")

set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_SYSTEM_VERSION "2.0.0")

set(CODAL_TOOLCHAIN "CLANG")

if(CMAKE_VERSION VERSION_LESS "3.5.0")
    include(CMakeForceCompiler)
    cmake_force_c_compiler("${CLANG}" GNU)
    cmake_force_cxx_compiler("${CLANG++}" GNU)
else()
    # from 3.5 the force_compiler macro is deprecated: CMake can detect
    # llvm-gcc as being a GNU compiler automatically
	set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
    set(CMAKE_C_COMPILER "${CLANG}")
    set(CMAKE_CXX_COMPILER "${CLANG++}")
endif()

SET(CMAKE_AR "${LLVM_AR}" CACHE FILEPATH "Archiver")
SET(CMAKE_RANLIB "${LLVM_RANLIB}" CACHE FILEPATH "rlib")
set(CMAKE_CXX_OUTPUT_EXTENSION ".o")
