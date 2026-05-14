cmake_minimum_required(VERSION 3.22.1)

project(zygisk-test LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(
    -fPIC
    -fvisibility=hidden
)

# =========================
# Dobby (LOCAL ONLY)
# =========================
set(DOBBY_DIR ${CMAKE_SOURCE_DIR}/external/dobby)

add_library(dobby STATIC
    ${DOBBY_DIR}/source/dobby.cpp

    ${DOBBY_DIR}/source/InterceptRouting/InterceptRouting.cpp
    ${DOBBY_DIR}/source/InterceptRouting/InterceptRoutingEntry.cpp

    ${DOBBY_DIR}/source/InstructionRelocation/arm64/InstructionRelocationARM64.cc

    ${DOBBY_DIR}/source/TrampolineBridge/Trampoline/trampoline_arm64.cc

    ${DOBBY_DIR}/source/TrampolineBridge/ClosureBridge/arm64/closure_bridge_arm64.cc

    ${DOBBY_DIR}/source/MemoryAllocator/MemoryAllocator.cc
    ${DOBBY_DIR}/source/MemoryAllocator/NearMemoryArena.cc

    ${DOBBY_DIR}/source/Backend/UserMode/ExecMemory/code-patch-tool-posix.cc

    ${DOBBY_DIR}/source/SymbolResolver/elf/dobby_symbol_resolver.cc
)

target_include_directories(dobby PUBLIC
    ${DOBBY_DIR}/include
    ${DOBBY_DIR}/source
    ${DOBBY_DIR}/external
)

# =========================
# Module
# =========================
file(GLOB SRC_FILES CONFIGURE_DEPENDS *.cpp)

add_library(${MODULE_NAME} SHARED
    ${SRC_FILES}
)

target_link_libraries(${MODULE_NAME}
    dobby
    log
)

set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
