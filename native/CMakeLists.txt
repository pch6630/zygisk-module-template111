cmake_minimum_required(VERSION 3.22.1)
project(sample)

set(CMAKE_CXX_STANDARD 17)

# 컴파일 옵션
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS}
    -fno-exceptions
    -fno-rtti
    -fvisibility=hidden
    -fvisibility-inlines-hidden"
)

include(FetchContent)

FetchContent_Declare(
    dobby
    GIT_REPOSITORY https://github.com/jmpews/Dobby.git
    GIT_TAG 9ecb6c1
)

# 핵심
set(DOBBY_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(DOBBY_GENERATE_SHARED OFF CACHE BOOL "" FORCE)
set(Plugin.SymbolResolver OFF CACHE BOOL "" FORCE)

# ARM64 ASM 비활성화
add_definitions(-DDOBBY_DISABLE_ARM64_ASM)

FetchContent_MakeAvailable(dobby)

add_library(${MODULE_NAME} SHARED
    example.cpp
)

target_include_directories(${MODULE_NAME} PRIVATE
    ${dobby_SOURCE_DIR}/include
)

target_link_libraries(${MODULE_NAME} PRIVATE
    dobby
    log
)

macro(strip_and_move NAME)
    add_custom_command(TARGET ${NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DEBUG_SYMBOLS_PATH}
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:${NAME}>
            ${DEBUG_SYMBOLS_PATH}/$<TARGET_FILE_NAME:${NAME}>
        COMMAND ${CMAKE_STRIP} --strip-all
            $<TARGET_FILE:${NAME}>
    )
endmacro()

strip_and_move(${MODULE_NAME})
