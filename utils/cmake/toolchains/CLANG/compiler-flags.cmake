# See toolchain.cmake before modifying this.

# This file is a copy from ../ARM_GCC, building with Clang/LLVM however will not immediatly work,
# some flags are not accepted by both compilers (e.g -Wl,--no-wchar-size-warning, defined in target.json), handle conflicts, 
# and Clang will require the --target=arm-none-eabi flag. We also need Clang to include extra header files:
# "-I/lib/arm-none-eabi/include -I/etc/alternatives/gcc-arm-none-eabi-include -I/usr/include/newlib/c++/10.3.1 -I/usr/include/newlib/c++/10.3.1/arm-none-eabi"
# Note: The version of newlib may have changed. These changes *should* allow you too build upto the final link.

# The arm-embedded linker is auto configured, and specifically searches for corresponding libraries/startup files, given
# the architecture on the command line, Clang does not do this and as such linking is a much more difficult step, it remains easiest
# to run the arm-embedded linker, view its command using "$ python(3) build.py -v", further verbose this command, copy it and invoke lld directly.  

# This file hasn't been changed to reflect any of this, modifying target.json is likely the easier route.
# Note: The above build process assumes that arm-embedded is already installed, which brings in all of the dependencies.

set(EXPLICIT_INCLUDES "")
if((CMAKE_VERSION VERSION_GREATER "3.4.0") OR (CMAKE_VERSION VERSION_EQUAL "3.4.0"))
    # from CMake 3.4 <INCLUDES> are separate to <FLAGS> in the
    # CMAKE_<LANG>_COMPILE_OBJECT, CMAKE_<LANG>_CREATE_ASSEMBLY_SOURCE, and
    # CMAKE_<LANG>_CREATE_PREPROCESSED_SOURCE commands
    set(EXPLICIT_INCLUDES "<INCLUDES> ")
endif()

# Override the link rules:
set(CMAKE_C_CREATE_SHARED_LIBRARY "echo 'shared libraries not supported' && 1")
set(CMAKE_C_CREATE_SHARED_MODULE  "echo 'shared modules not supported' && 1")
set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_AR> -cr <LINK_FLAGS> <TARGET> <OBJECTS>")
set(CMAKE_C_COMPILE_OBJECT        "<CMAKE_C_COMPILER> <DEFINES> ${EXPLICIT_INCLUDES}<FLAGS> -o <OBJECT> -c <SOURCE>")

set(CMAKE_C_LINK_EXECUTABLE       "<CMAKE_C_COMPILER> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> -Wl,-Map,<TARGET>.map -Wl,--start-group <OBJECTS> <LINK_LIBRARIES> -lm -lc -lgcc -lm -lc -lgcc -Wl,--end-group --specs=nano.specs -o <TARGET>")

set(CMAKE_CXX_OUTPUT_EXTENSION ".o")
set(CMAKE_DEPFILE_FLAGS_CXX "-MMD -MT <OBJECT> -MF <DEPFILE>")
set(CMAKE_C_OUTPUT_EXTENSION ".o")
set(CMAKE_DEPFILE_FLAGS_C "-MMD -MT <OBJECT> -MF <DEPFILE>")

set(CMAKE_C_FLAGS_DEBUG_INIT          "-g -gdwarf-3")
set(CMAKE_C_FLAGS_MINSIZEREL_INIT     "-Os -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE_INIT        "-Os -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "-Os -g -gdwarf-3 -DNDEBUG")
set(CMAKE_INCLUDE_SYSTEM_FLAG_C "-isystem ")


set(CMAKE_ASM_FLAGS_DEBUG_INIT          "-g -gdwarf-3")
set(CMAKE_ASM_FLAGS_MINSIZEREL_INIT     "-Os -DNDEBUG")
set(CMAKE_ASM_FLAGS_RELEASE_INIT        "-Os -DNDEBUG")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO_INIT "-Os -g -gdwarf-3 -DNDEBUG")
set(CMAKE_INCLUDE_SYSTEM_FLAG_ASM  "-isystem ")

set(CMAKE_CXX_CREATE_STATIC_LIBRARY "<CMAKE_AR> -cr <LINK_FLAGS> <TARGET> <OBJECTS>")

set(CMAKE_CXX_LINK_EXECUTABLE       "<CMAKE_CXX_COMPILER> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> -Wl,-Map,<TARGET>.map -Wl,--start-group <OBJECTS> <LINK_LIBRARIES> -lnosys -lstdc++ -lsupc++ -lm -lc -lgcc -lstdc++ -lsupc++ -lm -lc -lgcc -Wl,--end-group  --specs=nano.specs -o <TARGET>")

set(CMAKE_CXX_FLAGS_DEBUG_INIT          "-g -gdwarf-3")
set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE_INIT        "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-Os -g -gdwarf-3 -DNDEBUG")
set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")

if (CMAKE_C_COMPILER_VERSION VERSION_GREATER "7.1.0" OR CMAKE_C_COMPILER_VERSION VERSION_EQUAL "7.1.0")
    message("${BoldRed}Supressing -Wexpansion-to-defined.${ColourReset}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-expansion-to-defined")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-expansion-to-defined")
endif ()