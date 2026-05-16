cmake_minimum_required(VERSION 3.10.2)

# 1. 프로젝트 이름 정의
project("bleach_mod_zygisk")

# 2. C++ 컴파일 표준 사양 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 3. 템플릿 표준 빌드 타겟 명칭 설정
set(MODULE_NAME "arm64-v8a")

# 4. 📁 [정밀 수정] dobby 폴더 내부를 정확히 찾아내도록 하위 경로 추가
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dobby)
link_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dobby)

# 5. 라이브러리 타겟 생성 (example.cpp 컴파일)
add_library(${MODULE_NAME} SHARED
    example.cpp
)

# 6. Zygisk 전용 네이밍 규칙 적용 (lib 접두사 제거)
set_target_properties(${MODULE_NAME} PROPERTIES PREFIX "")

# 7. 가상환경 4Kg 페이지 크기 호환 및 정적 링크 플래그 주입
target_link_options(${MODULE_NAME} PRIVATE "-Wl,-z,max-page-size=4096" "-static-libstdc++")

# 8. 안드로이드 시스템 로그 및 Dobby 정적 객체 링크
target_link_libraries(${MODULE_NAME}
    log
    android
    dobby
)
