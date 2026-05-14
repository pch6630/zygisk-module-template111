cmake_minimum_required(VERSION 3.22.1)

project(zygisk-test LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 26)

add_compile_options(
    -fPIC
    -fvisibility=hidden
)

# -----------------------------------------------------------------
# 🛠️ 메인 모듈單독 빌드 (Dobby 종속성 완전 제거)
# -----------------------------------------------------------------
file(GLOB SRC_FILES CONFIGURE_DEPENDS *.cpp)

add_library(${MODULE_NAME} SHARED
    ${SRC_FILES}
)

target_link_libraries(${MODULE_NAME}
    log
)

set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
