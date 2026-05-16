cmake_minimum_required(VERSION 3.10.2)

project("bleach_mod_zygisk")

# ====================================================================
# [핵심 수정] 가상 환경(Virtual Master) 및 아키텍처 호환성 강제 설정
# ====================================================================

# 1. 페이지 크기 정렬을 가상 환경 커널 표준인 4KB(4096)로 강제 고정 (호환되지 않는 기기 오류 해결)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,max-page-size=4096")

# 2. 최소 안드로이드 API 버전을 26(안드로이드 8.0)으로 낮춰 VM 구버전 OS 호환성 확보
set(ANDROID_PLATFORM android-26)

# 3. C++ 표준 및 런타임 공유 라이브러리 정적 링킹 (가상 환경 내 링킹 에러 방지)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ANDROID_STL libc++_static)

# ====================================================================
# 소스 파일 및 라이브러리 빌드 설정
# ====================================================================

# 모듈에 사용될 소스 파일 지정 (프로젝트 구조에 맞게 파일명 확인)
add_library(${PROJECT_NAME} SHARED
        main.cpp
        hook/hook.cpp
        utils/logging.cpp
)

# 안드로이드 로그 라이브러리 등 시스템 라이브러리 링크
target_link_libraries(${PROJECT_NAME}
        log
        android
)
