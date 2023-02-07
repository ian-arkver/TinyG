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

target_compile_options(tinyg.elf PRIVATE -mmcu=atxmega192a3 -Os)
target_link_options(tinyg.elf PRIVATE -mmcu=atxmega192a3 -Os)