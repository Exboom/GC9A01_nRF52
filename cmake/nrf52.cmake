set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR ARM)
# set(CMAKE_AR arm-none-eabi-ar)

set(TOOLCHAIN_PATH "/home/andrew/workfiles/gcc-arm-none-eabi-10-2020-q4-major")
set(TOOLCHAIN_TRIPLET "arm-none-eabi")
set(NRF5_SDK_PATH "/home/andrew/Nordic/SDK/nRF5_SDK_17.0.2_d674dde")

set(TOOLCHAIN_BIN_PATH "${TOOLCHAIN_PATH}/bin")
set(TOOLCHAIN_INC_PATH "${TOOLCHAIN_PATH}/${TOOLCHAIN_TRIPLET}/include")
set(TOOLCHAIN_LIB_PATH "${TOOLCHAIN_PATH}/${TOOLCHAIN_TRIPLET}/lib")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_COMPILER "${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_TRIPLET}-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_TRIPLET}-g++")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_TRIPLET}-gcc")
# set(SIZE arm-none-eabi-size)

string(CONCAT C_FLAGS_GCC
    " -mcpu=cortex-m4"
    " -mthumb" 
    " -mabi=aapcs"
    " -Wall" 
    " -Werror"
    " -mfloat-abi=hard" 
    " -mfpu=fpv4-sp-d16"
    " -ffunction-sections"
    " -fdata-sections"
    " -fno-strict-aliasing"
    " -fno-builtin" 
    " -fshort-enums"
)

string(CONCAT ASM_FLAGS_GCC
    " -mcpu=cortex-m4"
    " -mthumb" 
    " -mabi=aapcs"
    " -mfloat-abi=hard"
    " -mfpu=fpv4-sp-d16"
)

string(CONCAT CMAKE_EXE_LINKER_FLAGS
    " -O3 -g3 -Wl,--gc-sections"
    " --specs=nosys.specs"
    " -mcpu=cortex-m4"
    " --specs=nano.specs"
    " -mthumb -mabi=aapcs"
    )


# Default C compiler flags
set(CMAKE_C_FLAGS_DEBUG_INIT "${C_FLAGS_GCC}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG_INIT} -g3 -O3 -lm" CACHE STRING "" FORCE)

# Default C++ compiler flags
# set(CMAKE_CXX_FLAGS_DEBUG_INIT "${CXX_FLAGS_GCC}")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG_INIT} -g3 -Od" CACHE STRING "" FORCE)

# Default ASM compiler flags
set(CMAKE_ASM_FLAGS_DEBUG_INIT "${ASM_FLAGS_GCC}")
set(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG_INIT} -g3 -O3 -lm" CACHE STRING "" FORCE)