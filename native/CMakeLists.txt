cmake_minimum_required(VERSION 3.22.1)

project(zygisk-test LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

# ==========================================
# 기본 옵션
# ==========================================

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

add_compile_options(
    -fvisibility=hidden
)

add_compile_definitions(
    MODULE_NAME="${MODULE_NAME}"
)

# ==========================================
# FetchContent
# ==========================================

include(FetchContent)

FetchContent_Declare(
    dobby
    GIT_REPOSITORY https://github.com/jmpews/Dobby.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

FetchContent_Populate(dobby)

set(DOBBY ${dobby_SOURCE_DIR})

# ==========================================
# Dobby 최소 Android ARM64 소스만 사용
# ==========================================

set(DOBBY_SRC

    # core
    ${DOBBY}/source/dobby.cpp

    # assembler
    ${DOBBY}/source/core/assembler/assembler-arm64.cc

    # instruction relocator
    ${DOBBY}/source/InstructionRelocation/arm64/InstructionRelocationARM64.cc

    # interceptor
    ${DOBBY}/source/InterceptRouting/InterceptRouting.cpp
    ${DOBBY}/source/InterceptRouting/InterceptRoutingEntry.cpp

    # trampoline
    ${DOBBY}/source/TrampolineBridge/Trampoline/trampoline_arm64.cc

    # closure bridge
    ${DOBBY}/source/TrampolineBridge/ClosureBridge/arm64/closure_bridge_arm64.cc

    # memory
    ${DOBBY}/source/MemoryAllocator/MemoryAllocator.cc
    ${DOBBY}/source/MemoryAllocator/NearMemoryArena.cc

    # platform
    ${DOBBY}/source/Backend/UserMode/ExecMemory/code-patch-tool-posix.cc

    # symbol resolver
    ${DOBBY}/source/SymbolResolver/elf/dobby_symbol_resolver.cc
)

# ==========================================
# 사용자 소스
# ==========================================

file(GLOB SRC_FILES CONFIGURE_DEPENDS
    *.cpp
)

# ==========================================
# 라이브러리
# ==========================================

add_library(${MODULE_NAME} SHARED
    ${SRC_FILES}
    ${DOBBY_SRC}
)

# ==========================================
# include
# ==========================================

target_include_directories(${MODULE_NAME} PRIVATE

    ${DOBBY}/include
    ${DOBBY}/source
    ${DOBBY}/external
)

# ==========================================
# 링크
# ==========================================

target_link_libraries(${MODULE_NAME}
    log
)

# ==========================================
# 컴파일 옵션
# ==========================================

target_compile_options(${MODULE_NAME} PRIVATE
    -O3
    -fno-exceptions
    -fno-rtti
    -Wno-unused-variable
    -Wno-unused-value
)

# ==========================================
# visibility
# ==========================================

set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
