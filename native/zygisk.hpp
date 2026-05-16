/* Copyright 2022-2023 John "topjohnwu" Wu ... (생략) */
#pragma once
#include <jni.h>
#include <stdint.h>

#define ZYGISK_API_VERSION 3

namespace zygisk {
// ... (클래스 정의: Api, AppSpecializeArgs, ServerSpecializeArgs 등 상동)
class ModuleBase { /* ... */ };
struct AppSpecializeArgs { /* ... */ };
struct ServerSpecializeArgs { /* ... */ };
enum Option : int { /* ... */ };
enum StateFlag : uint32_t { /* ... */ };
struct Api { /* ... */ };

namespace internal {
// 템플릿 빌드용 내부 구조 정의 및 ABI 매핑 기능
struct api_table {
    void *v1[4], *v2[1], *v3[1];
    void (*registerModule)(api_table *, void *);
};

struct module_abi {
    int api_version;
    ModuleBase *impl;
    void (*onLoad)(Api *, JNIEnv *);
    void (*preAppSpecialize)(AppSpecializeArgs *);
    void (*postAppSpecialize)(const AppSpecializeArgs *);
    void (*preServerSpecialize)(ServerSpecializeArgs *);
    void (*postServerSpecialize)(const ServerSpecializeArgs *);
};

template <class T>
void entry_impl(api_table *table, JNIEnv *env) {
    // ABI 바인딩 로직
    // ... (상동)
    table->registerModule(table, &abi);
}
} // namespace internal
} // namespace zygisk

// Zygisk 모듈 등록 매크로 (필수)
#define REGISTER_ZYGISK_MODULE(clazz) ...
#define REGISTER_ZYGISK_COMPANION(func) ...
