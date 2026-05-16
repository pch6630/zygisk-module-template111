cmake_minimum_required(VERSION 3.10.2)

# 1. 프로젝트 이름 정의
project("bleach_mod_zygisk")

# 2. C++ 컴파일 표준 사양 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 3. 템플릿 고유의 모듈 이름 정의 (자동 압축 및 호환성 네이밍 자동 매핑)
set(MODULE_NAME "arm64-v8a")

# 4. 템플릿 표준 디렉토리 경로 지정 및 Dobby 정적 라이브러리 위치 강제 추적
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
link_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 5. 라이브러리 타겟 생성 (실제 사용 중인 example.cpp 컴파일)
add_library(${MODULE_NAME} SHARED
    example.cpp
)

# 6. 접두사(lib)를 제거하여 시스템이 Zygisk 바이너리로 다이렉트 인식하게 설정
set_target_properties(${MODULE_NAME} PROPERTIES PREFIX "")

# 7. 가상환경 4Kg 페이지 크기 호환 및 정적 라이브러리 링커 플래그 주익
target_link_options(${MODULE_NAME} PRIVATE "-Wl,-z,max-page-size=4096" "-static-libstdc++")

# 8. 안드로이드 시스템 로그 및 누락되었던 Dobby 정적 객체 강제 바인딩
target_link_libraries(${MODULE_NAME}
    log
    android
    dobby
)
