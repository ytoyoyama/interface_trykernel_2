# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 1.5.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(hyb_sns C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(hyb_sns
        hyb_sns.c
        core_0/main_c0.c
        core_1/application/sns.c
        core_1/boot/start_c1.c
        core_1/boot/vector_tbl.c
        core_1/device/adc/adc_rp2040.c
        core_1/device/devmgr/device_tbl.c
        core_1/device/devmgr/device.c
        core_1/device/i2c/i2c_rp2040.c
        core_1/kernel/context.c
        core_1/kernel/eventflag.c
        core_1/kernel/gpio.c
        core_1/kernel/inittsk.c
        core_1/kernel/interrupt.c
        core_1/kernel/messagebuf.c
        core_1/kernel/scheduler.c
        core_1/kernel/semaphore.c
        core_1/kernel/syslib.c
        core_1/kernel/systimer.c
        core_1/kernel/task_mange.c
        core_1/kernel/task_queue.c
        core_1/kernel/task_sync.c
        core_1/kernel/dispatch.S
)

pico_set_program_name(hyb_sns "hyb_sns")
pico_set_program_version(hyb_sns "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(hyb_sns 1)
pico_enable_stdio_usb(hyb_sns 0)

# Add the standard library to the build
target_link_libraries(hyb_sns
        pico_stdlib
        pico_multicore
)

# Add the standard include files to the build
target_include_directories(hyb_sns PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
  core_1/include
)

# Add any user requested libraries
target_link_libraries(hyb_sns 
        pico_cyw43_arch_lwip_poll
        )

pico_add_extra_outputs(hyb_sns)

