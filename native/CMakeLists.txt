cmake_minimum_required(VERSION 3.22.1)
project(sample)

# Dobby 소스 가져오기
include(FetchContent)
FetchContent_Declare(
  dobby
  GIT_REPOSITORY https://github.com/jmpews/Dobby.git
  GIT_TAG master
)

# Dobby 빌드 시 어셈블리 에러 방지 옵션
set(DOBBY_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(DOBBY_GENERATE_SHARED OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(dobby)

# 컴파일 플래그 설정
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -fvisibility=hidden -fvisibility-inlines-hidden")

# 라이브러리 빌드 (example.cpp 사용)
add_library(${MODULE_NAME} SHARED example.cpp)

# Dobby 및 로그 라이브러리 연결
target_link_libraries(${MODULE_NAME} PRIVATE log dobby)

# 빌드 후 파일 정리 스크립트
macro(strip_and_move NAME)
    add_custom_command(TARGET ${NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DEBUG_SYMBOLS_PATH}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${NAME}> ${DEBUG_SYMBOLS_PATH}/$<TARGET_FILE_NAME:${NAME}>
        COMMAND ${CMAKE_STRIP} --strip-all $<TARGET_FILE:${NAME}>
    )
endmacro()
strip_and_move(${MODULE_NAME})
