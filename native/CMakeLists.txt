cmake_minimum_required(VERSION 3.22.1)

project(zygisk-test)

enable_language(C CXX ASM) # ASM 언어 명시적 활성화

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

# Dobby 소스 빌드 시 어셈블리 에러를 막기 위한 전역 매크로 설정
add_definitions(-DDOBBY_DISABLE_ARM64_ASM)
add_compile_definitions(DOBBY_DISABLE_ARM64_ASM)

include(FetchContent)

# =========================
# Dobby
# =========================
FetchContent_Declare(
    dobby
    GIT_REPOSITORY github.com
    GIT_TAG master
    GIT_SHALLOW TRUE
)

# Dobby 내부 옵션 강제 오버라이드
set(DOBBY_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(DOBBY_GENERATE_SHARED OFF CACHE BOOL "" FORCE)
set(Plugin.SymbolResolver OFF CACHE BOOL "" FORCE)

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
