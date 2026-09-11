add_custom_command(
    TARGET ${device_device}
    COMMAND "${ARM_NONE_EABI_SIZE}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${device_device}"
    DEPENDS  ${device_device}
    COMMENT "Print total size info:"
)
