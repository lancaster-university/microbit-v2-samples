add_custom_command(
    TARGET ${device.device}
    COMMAND "${ARM_NONE_EABI_SIZE}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${device.device}"
    DEPENDS  ${device.device}
    COMMENT "Print total size info:"
)
