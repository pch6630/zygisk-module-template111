cmake_minimum_required(VERSION 3.22.1)

project(zygisk-test LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

# =========================
# 기본 컴파일 옵션
# =========================
add_compile_options(
    -fvisibility=hidden
    -fPIC
)

add_compile_definitions(
    MODULE_NAME="${MODULE_NAME}"
)

# =========================
# FetchContent Dobby
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
# ❗ 핵심: ARM64 ASM 제거 (진짜 해결)
# =========================

# Dobby 소스 중 arm64 asm 제거
file(GLOB DOBBY_SRC
    ${dobby_SOURCE_DIR}/source/**/*.cpp
    ${dobby_SOURCE_DIR}/source/**/*.cc
)

# arm64 asm 파일 완전 제외
list(FILTER DOBBY_SRC EXCLUDE REGEX ".*arm64.*\\.asm")

# =========================
# Module Source
# =========================
file(GLOB SRC_FILES CONFIGURE_DEPENDS
    *.cpp
)

# =========================
# Library
# =========================
add_library(${MODULE_NAME} SHARED
    ${SRC_FILES}
    ${DOBBY_SRC}
)

target_include_directories(${MODULE_NAME} PRIVATE
    ${dobby_SOURCE_DIR}/include
    ${dobby_SOURCE_DIR}/source
)

target_link_libraries(${MODULE_NAME}
    log
)

target_compile_options(${MODULE_NAME} PRIVATE
    -fno-exceptions
    -fno-rtti
    -Wno-unused-variable
    -Wno-unused-value
)

set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
