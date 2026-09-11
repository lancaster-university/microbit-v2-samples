add_custom_command(
    OUTPUT "${PROJECT_SOURCE_DIR}/${codal_output_folder}/${device_device}.hex"
    COMMAND "${ARM_NONE_EABI_OBJCOPY}" -O ihex "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${device_device}" "${PROJECT_SOURCE_DIR}/${codal_output_folder}/${device_device}.hex"
    DEPENDS  ${device_device}
    COMMENT "converting to hex file."
)

#specify a dependency on the elf file so that hex is automatically rebuilt when elf is changed.
add_custom_target(${device_device}_hex ALL DEPENDS "${PROJECT_SOURCE_DIR}/${codal_output_folder}/${device_device}.hex")
