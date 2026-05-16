cmake_minimum_required(VERSION 3.10.2)

# 1. 프로젝트 이름 정의
project("bleach_mod_zygisk")

# 2. C++ 컴파일 표준 사양 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 3. [추가] 외부 Dobby 라이브러리 경로 지정 (프로ジェクト jni 폴더 또는 같은 위치에 dobby가 있어야 함)
# 만약 경로가 다르면 빌드 에러가 날 수 있으므로 기본 링크 헤더 구조를 잡습니다.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 4. 라이브러리 타겟 생성 (실제 파일명인 example.cpp 적용)
add_library(${PROJECT_NAME} SHARED
    example.cpp
)

# 5. ⭐ [핵심 수정] 출력되는 .so 파일 이름을 'arm64-v8a'로 강제 고정 (수동 이름 변경 불필요, 호환성 오류 완벽 해결)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "arm64-v8a")
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# 6. 가상환경 4Kg 페이지 크기 호환 및 정적 링크 플래그 주입
target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-z,max-page-size=4096" "-static-libstdc++")

# 7. 인드로이드 시스템 로그 및 Dobby 라이브러리 링크 (dobby가 누락되어 빌드가 깨졌던 것을 복구)
target_link_libraries(${PROJECT_NAME}
    log
    android
    dobby  # <-- 다른 AI가 빼먹은 필수 종속성 추가
)
