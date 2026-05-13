cmake_minimum_required(VERSION 3.22.1)
project(sample)

# Dobby 라이브러리를 외부(GitHub)에서 직접 다운로드하여 포함시키는 설정
include(FetchContent)
FetchContent_Declare(
  dobby
  GIT_REPOSITORY https://github.com/jmpews/Dobby.git
  GIT_TAG master
)
FetchContent_MakeAvailable(dobby)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -fvisibility=hidden -fvisibility-inlines-hidden")

add_library(${MODULE_NAME} SHARED example.cpp)
# dobby를 링크 대상에 포함
target_link_libraries(${MODULE_NAME} PRIVATE log dobby)

macro(strip_and_move NAME)
    add_custom_command(TARGET ${NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DEBUG_SYMBOLS_PATH}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${NAME}> ${DEBUG_SYMBOLS_PATH}/$<TARGET_FILE_NAME:${NAME}>
        COMMAND ${CMAKE_STRIP} --strip-all $<TARGET_FILE:${NAME}>
    )
endmacro()
strip_and_move(${MODULE_NAME})
