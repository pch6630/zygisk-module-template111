cmake_minimum_required(VERSION 3.22.1)

project(zygisk-test LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

# =========================
# Global compile options
# =========================
add_compile_options(
    -fvisibility=hidden
    -fno-exceptions
    -fno-rtti
)

add_compile_definitions(
    DOBBY_DISABLE_ARM64_ASM
)

# =========================
# Fetch Dobby
# =========================
include(FetchContent)

FetchContent_Declare(
    dobby
    GIT_REPOSITORY https://github.com/jmpews/Dobby.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(dobby)

# =========================
# IMPORTANT: Dobby options FIX
# =========================
set(DOBBY_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(DOBBY_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(DOBBY_GENERATE_SHARED OFF CACHE BOOL "" FORCE)

# (옵션 이름 수정: Plugin.SymbolResolver → 실제는 DOBBY 옵션 아님)
set(DOBBY_SYMBOL_RESOLVER OFF CACHE BOOL "" FORCE)

# =========================
# Source Files
# =========================
file(GLOB SRC_FILES CONFIGURE_DEPENDS
    *.cpp
)

# =========================
# Module
# =========================
add_library(${MODULE_NAME} SHARED ${SRC_FILES})

target_include_directories(${MODULE_NAME} PRIVATE
    ${dobby_SOURCE_DIR}/include
    ${dobby_SOURCE_DIR}/source
)

target_link_libraries(${MODULE_NAME}
    dobby
    log
)

target_compile_options(${MODULE_NAME} PRIVATE
    -Wno-unused-value
    -Wno-unused-variable
)

set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
