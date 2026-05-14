cmake_minimum_required(VERSION 3.22.1)

project(zygisk-test LANGUAGES C CXX)

# =========================
# C / C++
# =========================
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

    # 안정 커밋 권장
    GIT_TAG 5637b3c

    GIT_SHALLOW TRUE
)

# ❗ MakeAvailable 쓰지 말 것
# Dobby 자체 CMake 빌드 방지
FetchContent_Populate(dobby)

# =========================
# Dobby Source
# =========================
file(GLOB_RECURSE DOBBY_SRC
    ${dobby_SOURCE_DIR}/source/*.cc
    ${dobby_SOURCE_DIR}/source/*.cpp
)

# =========================
# ARM64 ASM 제거
# =========================
list(FILTER DOBBY_SRC EXCLUDE REGEX ".*\\.asm$")
list(FILTER DOBBY_SRC EXCLUDE REGEX ".*arm64.*")

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
    -fno-stack-protector
    -fomit-frame-pointer
    -ffunction-sections
    -fdata-sections
    -Wno-unused-variable
    -Wno-unused-value
)

# =========================
# Linker Options
# =========================
target_link_options(${MODULE_NAME} PRIVATE
    -Wl,--gc-sections
    -Wl,--exclude-libs,ALL
)

# =========================
# Visibility
# =========================
set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)

# =========================
# Android
# =========================
if(ANDROID)

    target_compile_definitions(${MODULE_NAME} PRIVATE
        ANDROID
    )

endif()

message(STATUS "====================================")
message(STATUS "Module Name : ${MODULE_NAME}")
message(STATUS "Build Type  : ${CMAKE_BUILD_TYPE}")
message(STATUS "ABI         : ${ANDROID_ABI}")
message(STATUS "====================================")
