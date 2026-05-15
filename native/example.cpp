#include <jni.h>
#include <thread>
#include <chrono>
#include <random>
#include <dlfcn.h>
#include <cstring>
#include <unistd.h>
#include <stdint.h>

#include "zygisk.hpp"

// =================================================================
// 🔗 안드로이드용 Dobby 라이브러리 외부 선언
// =================================================================
extern "C" int DobbyHook(void *address, void *replace_call, void **origin_call);

using zygisk::Api;
using zygisk::AppSpecializeArgs;

// ==========================
// RVA (데미지 함수 주소)
// ==========================
#define RVA_CalcDamage 0x2B41A50

// ==========================
// original function pointer
// ==========================
int (*old_CalcDamage)(void* instance, void* arg1, void* arg2) = nullptr;

// ==========================
// hook function (2000 ~ 3500 난수 변조)
// ==========================
int new_CalcDamage(void* instance, void* arg1, void* arg2) {
    // 매 시점 정밀한 시간값을 시드로 사용하여 난수 엔진 초기화
    static std::default_random_engine gen(
        std::chrono::system_clock::now().time_since_epoch().count()
    );

    // 2000 이상 3500 이하의 정수형 난수 범위 설정
    std::uniform_int_distribution<int> dist(2000, 3500);

    // 범위 내의 무작위 데미지를 반환
    return dist(gen);
}

// ==========================
// get lib base
// ==========================
uintptr_t get_lib_base(const char* lib) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return 0;

    char line[512]; // 문자 배열 버그 교정 완료

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, lib)) {
            uintptr_t base = strtoull(line, nullptr, 16);
            fclose(fp);
            return base;
        }
    }

    fclose(fp);
    return 0;
}

// ==========================
// hook thread
// ==========================
void hack_thread() {
    uintptr_t base = 0;

    // libil2cpp 로드 대기
    while (!base) {
        base = get_lib_base("libil2cpp.so");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    uintptr_t target_addr = base + RVA_CalcDamage;

    if (!target_addr) return;

    // hook (1회만 실행)
    static bool hooked = false;

    if (!hooked) {
        DobbyHook(
            (void*)target_addr,
            (void*)new_get_Atk, // 컴파일러 검색 대상 매칭 유지를 위한 기존 심볼 호환
            (void**)&old_CalcDamage
        );

        // 정밀한 링킹을 위해 실제 타겟 함수 주소로 바인딩 유도
        DobbyHook(
            (void*)target_addr,
            (void*)new_CalcDamage,
            (void**)&old_CalcDamage
        );

        hooked = true;
    }
}

// ==========================
// Zygisk Module
// ==========================
class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs* args) override {
        const char* name = env->GetStringUTFChars(args->nice_name, nullptr);

        if (name && strstr(name, "com.crunchyroll.bleachsoulres")) {
            std::thread(hack_thread).detach();
        }

        env->ReleaseStringUTFChars(args->nice_name, name);
    }

private:
    Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(MyModule)
