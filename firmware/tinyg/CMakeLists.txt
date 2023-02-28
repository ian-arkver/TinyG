cmake_minimum_required(VERSION 3.10)
project(tinyg VERSION 0.1.0)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER avr-gcc)

add_executable(tinyg.elf
    canonical_machine.c
    config.c
    config_app.c
    controller.c
    cycle_homing.c
    cycle_jogging.c
    cycle_probing.c
    encoder.c
    gcode_parser.c
    gpio.c
    hardware.c
    help.c
    json_parser.c
    kinematics.c
    main.c
    network.c
    persistence.c
    planner.c
    plan_arc.c
    plan_exec.c
    plan_line.c
    plan_zoid.c
    pwm.c
    report.c
    spindle.c
    stepper.c
    switch.c
    test.c
    text_parser.c
    util.c
    xio.c
    xio/xio_file.c
    xio/xio_pgm.c
    xio/xio_rs485.c
    xio/xio_spi.c
    xio/xio_usart.c
    xio/xio_usb.c
    xmega/xmega_adc.c
    xmega/xmega_eeprom.c
    xmega/xmega_init.c
    xmega/xmega_interrupts.c
    xmega/xmega_rtc.c
)

target_compile_definitions(tinyg.elf PRIVATE
    F_CPU=32000000UL
    M115_BUILD_SYSTEM_STRING=", X-BUILD_SYSTEM:CMake"
)
target_compile_options(tinyg.elf PRIVATE
    -mmcu=atxmega192a3
    -O2 -g2 -Wall
    -funsigned-char
    -funsigned-bitfields
    -fno-align-functions
    -fno-align-jumps
    -fno-align-loops
    -fno-align-labels
    -fno-reorder-blocks
    -fno-reorder-blocks-and-partition
    -fno-prefetch-loop-arrays
    -fno-tree-vect-loop-version
    -ffunction-sections
    -fdata-sections
    -fpack-struct
    -fshort-enums
)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_C_COMPILER_VERSION VERSION_GREATER 12.0)
    message(STATUS "Addding workaround for avr-gcc 12 bug")
    target_compile_options(tinyg.elf PRIVATE
        --param=min-pagesize=0
    )
endif()

target_link_options(tinyg.elf PRIVATE -mmcu=atxmega192a3)

add_custom_command(
	TARGET tinyg.elf
	POST_BUILD
	COMMAND avr-objcopy
	ARGS -O ihex $<TARGET_FILE:tinyg.elf> ${PROJECT_BINARY_DIR}/tinyg.hex
)