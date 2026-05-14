cmake_minimum_required(VERSION 3.22.1)

project(zygisk-test LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(-fPIC -fvisibility=hidden)

include(FetchContent)

# ✅ Dobby 안정 버전 고정 (중요)
FetchContent_Declare(
    dobby
    GIT_REPOSITORY https://github.com/jmpews/Dobby.git
    GIT_TAG v1.2.31
    GIT_SHALLOW FALSE
)

FetchContent_MakeAvailable(dobby)

file(GLOB SRC_FILES CONFIGURE_DEPENDS *.cpp)

add_library(${MODULE_NAME} SHARED
    ${SRC_FILES}
)

target_include_directories(${MODULE_NAME} PRIVATE
    ${dobby_SOURCE_DIR}/include
)

target_link_libraries(${MODULE_NAME}
    dobby
    log
)

set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
