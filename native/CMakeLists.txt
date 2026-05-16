cmake_minimum_required(VERSION 3.10.2)

# 1. 프로젝트 이름 정의
project("bleach_mod_zygisk")

# 2. C++ 컴파일 표준 사양 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 3. 라이브러리 타겟 생성 (실제 파일명인 example.cpp 적용)
add_library(${PROJECT_NAME} SHARED
        example.cpp
)

# 4. 가상환경 4KB 페이지 크기 호환 및 정적 링크 플래그 주입
target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-z,max-page-size=4096" "-static-libstdc++")

# 5. 안드로이드 시스템 로그 라이브러리 링크
target_link_libraries(${PROJECT_NAME}
        log
        android
)
