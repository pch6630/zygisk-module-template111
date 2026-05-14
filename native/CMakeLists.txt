cmake_minimum_required(VERSION 3.22.1)

project(zygisk-test LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================
# 기본 옵션
# =========================
add_compile_options(
    -fvisibility=hidden
    -fPIC
)

add_compile_definitions(
    MODULE_NAME="${MODULE_NAME}"
)

# =========================
# FetchContent
# =========================
include(FetchContent)

FetchContent_Declare(
    dobby
    GIT_REPOSITORY https://github.com/jmpews/Dobby.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

FetchContent_Populate(dobby)

# =========================
# Dobby Source
# =========================
file(GLOB_RECURSE DOBBY_SRC
    ${dobby_SOURCE_DIR}/source/*.cc
    ${dobby_SOURCE_DIR}/source/*.cpp
)

# ❗ asm 파일만 제거
list(FILTER DOBBY_SRC EXCLUDE REGEX ".*\\.asm$")

# =========================
# Module Source
# =========================
file(GLOB SRC_FILES CONFIGURE_DEPENDS
    *.cpp
)

# =========================
# Shared Library
# =========================
add_library(${MODULE_NAME} SHARED
    ${SRC_FILES}
    ${DOBBY_SRC}
)

# =========================
# Include
# =========================
target_include_directories(${MODULE_NAME} PRIVATE
    ${dobby_SOURCE_DIR}/include
    ${dobby_SOURCE_DIR}/source
    ${dobby_SOURCE_DIR}
)

# =========================
# Link
# =========================
target_link_libraries(${MODULE_NAME}
    log
)

# =========================
# Compile Options
# =========================
target_compile_options(${MODULE_NAME} PRIVATE
    -O3
    -fno-exceptions
    -fno-rtti
    -Wno-unused-variable
    -Wno-unused-value
)

# =========================
# Visibility
# =========================
set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
