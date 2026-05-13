cmake_minimum_required(VERSION 3.22.1)

project(zygisk-test)

enable_language(C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

include(FetchContent)

# =========================
# Dobby
# =========================
FetchContent_Declare(
    dobby
    GIT_REPOSITORY https://github.com/jmpews/Dobby.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(dobby)

# =========================
# Source Files
# =========================
file(GLOB SRC_FILES
    *.cpp
)

# =========================
# Module
# =========================
add_library(${MODULE_NAME} SHARED ${SRC_FILES})

target_include_directories(${MODULE_NAME} PRIVATE
    ${dobby_SOURCE_DIR}/include
)

target_link_libraries(${MODULE_NAME}
    dobby
    log
)

target_compile_options(${MODULE_NAME} PRIVATE
    -Wno-unused-value
    -Wno-unused-variable
    -fno-exceptions
    -fno-rtti
)

set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
