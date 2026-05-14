cmake_minimum_required(VERSION 3.22.1)

project(zygisk-test LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ==========================================
# Zygisk module name
# ==========================================
add_compile_definitions(
    MODULE_NAME="${MODULE_NAME}"
)

# ==========================================
# Dobby (FULL SAFE BUILD)
# ==========================================
include(FetchContent)

set(DOBBY_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(DOBBY_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(DOBBY_GENERATE_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    dobby
    GIT_REPOSITORY https://github.com/jmpews/Dobby.git
    GIT_TAG master
)

FetchContent_MakeAvailable(dobby)

# ==========================================
# Source
# ==========================================
file(GLOB SRC_FILES CONFIGURE_DEPENDS
    *.cpp
)

# ==========================================
# Module
# ==========================================
add_library(${MODULE_NAME} SHARED
    ${SRC_FILES}
)

# ==========================================
# Include
# ==========================================
target_include_directories(${MODULE_NAME} PRIVATE
    ${dobby_SOURCE_DIR}/include
)

# Dobby target 연결 (핵심)
target_link_libraries(${MODULE_NAME}
    dobby
    log
)

# ==========================================
# Compile options
# ==========================================
target_compile_options(${MODULE_NAME} PRIVATE
    -O3
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -fno-exceptions
    -fno-rtti
    -Wno-unused-variable
    -Wno-unused-value
)

set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
