# This file is no longer auto-generated to make the repository builds with GCC
# and ARMCC no matter what.

cmake_minimum_required(VERSION 2.8.12)

enable_language(ASM)

set(YOTTA_AUTO_CODAL_CPP_FILES
    "core/MemberFunctionCallback.cpp"
    "core/CodalCompat.cpp"
    "core/CodalDmesg.cpp"
    "core/CodalDevice.cpp"
    "core/DeviceComponent.cpp"
    "core/DeviceFiber.cpp"
    "core/DeviceHeapAllocator.cpp"
    "core/DeviceListener.cpp"
    "core/DeviceSystemTimer.cpp"
    "core/CodalUSB.cpp"

    "types/ManagedString.cpp"
    "types/Matrix4.cpp"
    "types/DeviceEvent.cpp"

    "types/ManagedBuffer.cpp"
    "types/RefCounted.cpp"

    "device/SAMD21/DeviceTimer.cpp"
    "device/SAMD21/USB.cpp"
    "device/SAMD21/SAMD21DMAC.cpp"
    "device/SAMD21/SAMD21DAC.cpp"

    "drivers/DynamicPwm.cpp"
    "drivers/AbstractButton.cpp"
    "drivers/DeviceButton.cpp"
    "drivers/MultiButton.cpp"
    "drivers/TouchButton.cpp"
    "drivers/TouchSensor.cpp"
    "drivers/DevicePin.cpp"
    "drivers/TimedInterruptIn.cpp"
    "drivers/DeviceSerial.cpp"
    "drivers/DeviceMessageBus.cpp"
    "drivers/DeviceI2C.cpp"
    "drivers/HID.cpp"
    "drivers/LIS3DH.cpp"
    "drivers/AnalogSensor.cpp"
    "drivers/LinearAnalogSensor.cpp"
    "drivers/NonLinearAnalogSensor.cpp"
)

execute_process(WORKING_DIRECTORY "../../yotta_modules/${PROJECT_NAME}" COMMAND "git" "log" "--pretty=format:%h" "-n" "1" OUTPUT_VARIABLE git_hash)
execute_process(WORKING_DIRECTORY "../../yotta_modules/${PROJECT_NAME}" COMMAND "git" "rev-parse" "--abbrev-ref" "HEAD" OUTPUT_VARIABLE git_branch OUTPUT_STRIP_TRAILING_WHITESPACE)

if ("${git_branch}" STREQUAL "master")
    set(CODAL_VERSION_STRING "${YOTTA_CODAL_VERSION_STRING}")
else()
    set(CODAL_VERSION_STRING "${YOTTA_CODAL_VERSION_STRING}-${git_branch}-g${git_hash}")
endif()

set(CODAL_VERSION_FLAGS "-DCODAL_VERSION=\\\"${CODAL_VERSION_STRING}\\\"")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CODAL_VERSION_FLAGS}")

if(CMAKE_COMPILER_IS_GNUCC)
  file(REMOVE "asm/CortexContextSwitch.s")
  configure_file("asm/CortexContextSwitch.s.gcc" "asm/CortexContextSwitch.s" COPYONLY)
else()
  file(REMOVE "asm/CortexContextSwitch.s")
  configure_file("asm/CortexContextSwitch.s.armcc" "asm/CortexContextSwitch.s" COPYONLY)
endif()

set(YOTTA_AUTO_CODAL_S_FILES
    "asm/CortexContextSwitch.s"
)

add_library(codal
    ${YOTTA_AUTO_CODAL_CPP_FILES}
    ${YOTTA_AUTO_CODAL_S_FILES}
)

yotta_postprocess_target(LIBRARY codal)

target_link_libraries(codal
    mbed-classic
)

if(CMAKE_COMPILER_IS_GNUCC)
    message("suppressing ALL warnings from mbed-classic")
    target_compile_options(mbed-classic PRIVATE "-w")
endif()
