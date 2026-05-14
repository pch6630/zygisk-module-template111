cmake_minimum_required(VERSION 3.22.1)

project(zygisk-test LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(
    -fPIC
    -fvisibility=hidden
)

add_compile_definitions(
    MODULE_NAME="${MODULE_NAME}"
)

include(FetchContent)

# =========================
# Dobby (FIXED)
# =========================
FetchContent_Declare(
    dobby
    GIT_REPOSITORY https://github.com/jmpews/Dobby.git
    GIT_TAG master
    GIT_SHALLOW FALSE
)

FetchContent_MakeAvailable(dobby)

set(DOBBY ${dobby_SOURCE_DIR})

# =========================
# Dobby sources (ARM64 only)
# =========================
set(DOBBY_SRC
    ${DOBBY}/source/dobby.cpp
    ${DOBBY}/source/InterceptRouting/InterceptRouting.cpp
    ${DOBBY}/source/InterceptRouting/InterceptRoutingEntry.cpp
    ${DOBBY}/source/InstructionRelocation/arm64/InstructionRelocationARM64.cc
    ${DOBBY}/source/TrampolineBridge/Trampoline/trampoline_arm64.cc
    ${DOBBY}/source/TrampolineBridge/ClosureBridge/arm64/closure_bridge_arm64.cc
    ${DOBBY}/source/MemoryAllocator/MemoryAllocator.cc
    ${DOBBY}/source/MemoryAllocator/NearMemoryArena.cc
    ${DOBBY}/source/Backend/UserMode/ExecMemory/code-patch-tool-posix.cc
    ${DOBBY}/source/SymbolResolver/elf/dobby_symbol_resolver.cc
)

file(GLOB SRC_FILES CONFIGURE_DEPENDS
    *.cpp
)

add_library(${MODULE_NAME} SHARED
    ${SRC_FILES}
    ${DOBBY_SRC}
)

target_include_directories(${MODULE_NAME} PRIVATE
    ${DOBBY}/include
    ${DOBBY}/source
    ${DOBBY}/external
)

target_link_libraries(${MODULE_NAME}
    log
)

target_compile_options(${MODULE_NAME} PRIVATE
    -O3
    -fno-exceptions
    -fno-rtti
    -Wno-unused-variable
    -Wno-unused-value
)

set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
