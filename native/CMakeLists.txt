cmake_minimum_required(VERSION 3.22.1)

project(zygisk-test LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 26)

add_compile_options(
    -fPIC
    -fvisibility=hidden
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

# 컴파일 단계에서 외부 libdobby.a 파일과 내 Zygisk 코드를 완전히 합칩니다.
target_link_libraries(${MODULE_NAME}
    dobby
    log
)

set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
