cmake_minimum_required(VERSION 3.22.1)

project(sample)

set(CMAKE_CXX_STANDARD 17)

# 컴파일 옵션 (줄바꿈 오류 해결 버전)
set(CMAKE_CXX_FLAGS
    ${CMAKE_CXX_FLAGS}
    -fno-exceptions
    -fno-rtti
    -fvisibility=hidden
    -fvisibility-inlines-hidden
)

# Dobby 가져오기
include(FetchContent)

FetchContent_Declare(
    dobby
    GIT_REPOSITORY github.com
)

# Dobby 옵션
set(DOBBY_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(DOBBY_GENERATE_SHARED OFF CACHE BOOL "" FORCE)
set(Plugin.SymbolResolver OFF CACHE BOOL "" FORCE)

# ARM64 ASM 비활성화
add_definitions(-DDOBBY_DISABLE_ARM64_ASM)

FetchContent_MakeAvailable(dobby)

# 모듈 생성
add_library(${MODULE_NAME} SHARED
    example.cpp
)

# include 경로
target_include_directories(${MODULE_NAME} PRIVATE
    ${dobby_SOURCE_DIR}/include
)

# 라이브러리 링크
target_link_libraries(${MODULE_NAME} PRIVATE
    dobby
    log
)

# strip 및 심볼 저장
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
