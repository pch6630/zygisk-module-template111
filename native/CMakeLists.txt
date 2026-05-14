cmake_minimum_required(VERSION 3.22.1)

project(zygisk-test LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 26)

add_compile_options(
    -fPIC
    -fvisibility=hidden
)

# -----------------------------------------------------------------
# 🌐 Dobby 공식 소스 코드 원격 자동 다운로드 및 빌드 설정 (코드로 해결)
# -----------------------------------------------------------------
include(FetchContent)

# GitHub에서 jmpews/Dobby 마스터 브랜치의 최신 소스코드를 자동으로 긁어옵니다.
FetchContent_Declare(
    dobby_remote
    GIT_REPOSITORY https://github.com
    GIT_TAG        master
)

# Dobby 내부 빌드 옵션 사전 제어 (불필요한 테스트/공유라이브러리 빌드 차단)
set(DOBBY_DEBUG OFF CACHE INTERNAL "" FORCE)
set(DOBBY_GENERATE_SHARED OFF CACHE INTERNAL "" FORCE)

# 원격 소스를 가져와서 내 프로젝트 서브디렉토리로 강제 주입
FetchContent_MakeAvailable(dobby_remote)

# Dobby 원본 저장소의 헤더 위치 자동 탐색 및 내보내기
get_property(DOBBY_INCLUDES TARGET dobby PROPERTY INCLUDE_DIRECTORIES)

# -----------------------------------------------------------------
# 🛠️ Zygisk 메인 모듈 컴파일
# -----------------------------------------------------------------
file(GLOB SRC_FILES CONFIGURE_DEPENDS *.cpp)

# 내 메인 소스 파일들로 zygisk 모듈 생성
add_library(${MODULE_NAME} SHARED
    ${SRC_FILES}
)

# 원격에서 다운로드되어 자동 빌드된 정적 dobby 라이브러리와 내 zygisk 모듈 연동
target_link_libraries(${MODULE_NAME}
    dobby
    log
)

# 헤더파일 디렉토리 강제 연동
target_include_directories(${MODULE_NAME} PRIVATE
    ${DOBBY_INCLUDES}
)

set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
