add_custom_command(
    OUTPUT "${PROJECT_SOURCE_DIR}/${codal_output_folder}/${device_device}.bin"
    COMMAND "${ARM_NONE_EABI_OBJCOPY}" -O binary "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${device_device}" "${PROJECT_SOURCE_DIR}/${codal_output_folder}/${device_device}.bin"
    DEPENDS  ${device_device}
    COMMENT "converting to bin file."
)

#specify a dependency on the elf file so that bin is automatically rebuilt when elf is changed.
add_custom_target(${device_device}_bin ALL DEPENDS "${PROJECT_SOURCE_DIR}/${codal_output_folder}/${device_device}.bin")
