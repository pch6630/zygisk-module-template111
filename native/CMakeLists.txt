cmake_minimum_required(VERSION 3.22.1)

project(zygisk-test LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 26)

# [교정] -fvisibility=hidden 옵션을 완전히 지워 Zygisk 진입점이 잘려나가는 문제를 방지합니다.
add_compile_options(
    -fPIC
)

# -----------------------------------------------------------------
# 📦 업로드한 안드로이드용 진짜 Dobby(libdobby.a) 연동
# -----------------------------------------------------------------
set(DOBBY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dobby")

add_library(dobby STATIC IMPORTED)
set_target_properties(dobby PROPERTIES
    IMPORTED_LOCATION "${DOBBY_DIR}/libdobby.a"
)

# -----------------------------------------------------------------
# 🛠️ Zygisk 메인 모듈 빌드
# -----------------------------------------------------------------
file(GLOB SRC_FILES CONFIGURE_DEPENDS *.cpp)

add_library(${MODULE_NAME} SHARED
    ${SRC_FILES}
)

target_link_libraries(${MODULE_NAME}
    dobby
    log
)

# [교정] 속성 제어부에서도 진입점을 숨기는 설정을 제거(YES -> NO)하여 호환성 에러를 완벽히 해결합니다.
set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN NO
)
