cmake_minimum_required(VERSION 3.16)

message("BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

project(remote C ASM)

# ====================================================================================== #

# Add modules
include(cmake/makebinary.cmake)

# Set linker scripts
get_filename_component(LINKER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ld ABSOLUTE)
get_filename_component(LINKER_SCRIPT ${LINKER_DIR}/linker.ld ABSOLUTE)

#Set Root SDK path
set(SDK_ROOT "/home/andrew/Nordic/SDK/nRF5_SDK_17.0.2_d674dde")

# ====================================================================================== #

# Application sources
set(APP_SRC
    ${PROJECT_SOURCE_DIR}/main.c
)

# Create targets elf bin hex and dumps
makebinarys(${PROJECT_NAME})

add_executable(${ELF_FILE} ${APP_SRC})

# Includes
target_include_directories(${ELF_FILE}
    PRIVATE
    ${LINKER_DIR}
    ${PROJECT_SOURCE_DIR}
)

# Linkage
target_link_libraries(${ELF_FILE} 
    PRIVATE 
        gc9a01
        m
)

target_link_options(${PROJECT_NAME}.elf
    PRIVATE
    -L${SDK_ROOT}/modules/nrfx/mdk
    -T${LINKER_SCRIPT}
    # -Wl,--gc-sections
    -Wl,-Map=${MAP_FILE},--cref,--no-warn-mismatch
    -g -Wl,--gc-sections
)

target_compile_definitions(${PROJECT_NAME}.elf
    PUBLIC 
        APP_TIMER_V2
        APP_TIMER_V2_RTC1_ENABLED
        BOARD_PCA10040
        CONFIG_GPIO_AS_PINRESET
        FLOAT_ABI_HARD
        NRF52
        NRF52832_XXAA
        NRF52_PAN_74
        __HEAP_SIZE=8192
        __STACK_SIZE=8192
)

# ====================================================================================== #

set(sdk_sources
    ${SDK_ROOT}/modules/nrfx/mdk/gcc_startup_nrf52.S
    ${SDK_ROOT}/components/libraries/log/src/nrf_log_backend_rtt.c 
    ${SDK_ROOT}/components/libraries/log/src/nrf_log_backend_serial.c 
    ${SDK_ROOT}/components/libraries/log/src/nrf_log_backend_uart.c 
    ${SDK_ROOT}/components/libraries/log/src/nrf_log_default_backends.c 
    ${SDK_ROOT}/components/libraries/log/src/nrf_log_frontend.c 
    ${SDK_ROOT}/components/libraries/log/src/nrf_log_str_formatter.c 
    ${SDK_ROOT}/components/boards/boards.c 
    ${SDK_ROOT}/components/libraries/button/app_button.c 
    ${SDK_ROOT}/components/libraries/util/app_error.c 
    ${SDK_ROOT}/components/libraries/util/app_error_handler_gcc.c 
    ${SDK_ROOT}/components/libraries/util/app_error_weak.c 
    ${SDK_ROOT}/components/libraries/scheduler/app_scheduler.c 
    ${SDK_ROOT}/components/libraries/timer/app_timer2.c 
    ${SDK_ROOT}/components/libraries/util/app_util_platform.c 
    ${SDK_ROOT}/components/libraries/timer/drv_rtc.c 
    ${SDK_ROOT}/components/libraries/util/nrf_assert.c 
    ${SDK_ROOT}/components/libraries/atomic_fifo/nrf_atfifo.c 
    ${SDK_ROOT}/components/libraries/atomic/nrf_atomic.c 
    ${SDK_ROOT}/components/libraries/balloc/nrf_balloc.c 
    ${SDK_ROOT}/external/fprintf/nrf_fprintf.c 
    ${SDK_ROOT}/external/fprintf/nrf_fprintf_format.c 
    ${SDK_ROOT}/components/libraries/memobj/nrf_memobj.c 
    ${SDK_ROOT}/components/libraries/ringbuf/nrf_ringbuf.c 
    ${SDK_ROOT}/components/libraries/sortlist/nrf_sortlist.c 
    ${SDK_ROOT}/components/libraries/strerror/nrf_strerror.c 
    ${SDK_ROOT}/integration/nrfx/legacy/nrf_drv_spi.c 
    ${SDK_ROOT}/integration/nrfx/legacy/nrf_drv_uart.c 
    ${SDK_ROOT}/modules/nrfx/soc/nrfx_atomic.c 
    ${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_gpiote.c 
    ${SDK_ROOT}/modules/nrfx/drivers/src/prs/nrfx_prs.c 
    ${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_spi.c 
    ${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_spim.c 
    ${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_uart.c 
    ${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_uarte.c 
    ${SDK_ROOT}/components/libraries/bsp/bsp.c 
    ${SDK_ROOT}/external/segger_rtt/SEGGER_RTT.c 
    ${SDK_ROOT}/external/segger_rtt/SEGGER_RTT_Syscalls_GCC.c 
    ${SDK_ROOT}/external/segger_rtt/SEGGER_RTT_printf.c 
    ${SDK_ROOT}/modules/nrfx/mdk/system_nrf52.c 
)

add_library(sdk ${sdk_sources})
target_include_directories(sdk
    PUBLIC
    ${SDK_ROOT}/components 
    ${SDK_ROOT}/modules/nrfx/mdk 
    ${SDK_ROOT}/components/libraries/scheduler 
    ${SDK_ROOT}/components/libraries/timer 
    ${SDK_ROOT}/components/boards 
    ${SDK_ROOT}/components/libraries/strerror 
    ${SDK_ROOT}/components/toolchain/cmsis/include 
    ${SDK_ROOT}/components/libraries/util 
    ${PROJECT_SOURCE_DIR}/config 
    ${SDK_ROOT}/components/libraries/balloc 
    ${SDK_ROOT}/components/libraries/ringbuf 
    ${SDK_ROOT}/modules/nrfx/hal 
    ${SDK_ROOT}/components/libraries/bsp 
    ${SDK_ROOT}/components/libraries/log 
    ${SDK_ROOT}/components/libraries/button 
    ${SDK_ROOT}/modules/nrfx 
    ${SDK_ROOT}/components/libraries/experimental_section_vars 
    ${SDK_ROOT}/integration/nrfx/legacy 
    ${SDK_ROOT}/components/libraries/delay 
    ${SDK_ROOT}/external/segger_rtt 
    ${SDK_ROOT}/components/libraries/atomic_fifo 
    ${SDK_ROOT}/components/drivers_nrf/nrf_soc_nosd 
    ${SDK_ROOT}/components/libraries/atomic 
    ${SDK_ROOT}/components/libraries/sortlist 
    ${SDK_ROOT}/components/libraries/memobj 
    ${SDK_ROOT}/integration/nrfx 
    ${SDK_ROOT}/modules/nrfx/drivers/include 
    ${SDK_ROOT}/external/fprintf 
    ${SDK_ROOT}/components/libraries/log/src 
)

target_compile_definitions(sdk
    PUBLIC 
        APP_TIMER_V2
        APP_TIMER_V2_RTC1_ENABLED
        BOARD_PCA10040
        CONFIG_GPIO_AS_PINRESET
        FLOAT_ABI_HARD
        NRF52
        NRF52832_XXAA
        NRF52_PAN_74
        __HEAP_SIZE=8192
        __STACK_SIZE=8192
)

# ====================================================================================== #
add_subdirectory(gc9a01)