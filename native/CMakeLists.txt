cmake_minimum_required(VERSION 3.22.1)

project(zygisk-test LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

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
# Dobby Include
# =========================
set(DOBBY_INCLUDE
    ${dobby_SOURCE_DIR}/include
    ${dobby_SOURCE_DIR}/source
    ${dobby_SOURCE_DIR}
    ${dobby_SOURCE_DIR}/external
)

# =========================
# Dobby Android Source Only
# =========================
file(GLOB_RECURSE DOBBY_SRC
    ${dobby_SOURCE_DIR}/source/Backend/UserMode/*.cc
    ${dobby_SOURCE_DIR}/source/Backend/UserMode/*.cpp

    ${dobby_SOURCE_DIR}/source/InstructionRelocation/*.cc
    ${dobby_SOURCE_DIR}/source/InstructionRelocation/*.cpp

    ${dobby_SOURCE_DIR}/source/InterceptRouting/*.cc
    ${dobby_SOURCE_DIR}/source/InterceptRouting/*.cpp

    ${dobby_SOURCE_DIR}/source/TrampolineBridge/*.cc
    ${dobby_SOURCE_DIR}/source/TrampolineBridge/*.cpp

    ${dobby_SOURCE_DIR}/source/core/*.cc
    ${dobby_SOURCE_DIR}/source/core/*.cpp
)

# =========================
# asm 제거
# =========================
list(FILTER DOBBY_SRC EXCLUDE REGEX ".*\\.asm$")
list(FILTER DOBBY_SRC EXCLUDE REGEX ".*darwin.*")
list(FILTER DOBBY_SRC EXCLUDE REGEX ".*ios.*")
list(FILTER DOBBY_SRC EXCLUDE REGEX ".*kernel.*")
list(FILTER DOBBY_SRC EXCLUDE REGEX ".*dummy.*")

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
    ${DOBBY_INCLUDE}
)

# =========================
# Link
# =========================
target_link_libraries(${MODULE_NAME}
    log
)

# =========================
# Compile Option
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
