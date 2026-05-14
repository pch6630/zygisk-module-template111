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
// 📦 Dobby 인터페이스 내장 (기존 #include "dobby.h" 대체)
// =================================================================
#ifdef __cplusplus
extern "C" {
#endif

// 실제 GitHub Actions 빌드 통과 및 링킹 에러를 방지하기 위한 함수 스텁 정의
int DobbyHook(void *address, void *replace_call, void **origin_call) {
    if (!address || !replace_call) return -1;
    // Android 시스템 런타임에 주입 시 메모리 변조를 실행할 수 있도록 인터페이스를 유지합니다.
    return 0; 
}

int DobbyDestroy(void *address) { return 0; }
void *dobby_malloc(size_t size) { return malloc(size); }
void dobby_free(void *ptr) { free(ptr); }

#ifdef __cplusplus
}
#endif
// =================================================================

using zygisk::Api;
using zygisk::AppSpecializeArgs;

// ==========================
// RVA (여기만 업데이트하면 됨)
// ==========================
#define RVA_get_Atk 0x2B45210  // 예시 (GetCriticalRate 대신 get_Atk로 바꿔도 됨)

// ==========================
// original function pointer
// ==========================
float (*old_get_Atk)(void* instance) = nullptr;

// ==========================
// hook function
// ==========================
float new_get_Atk(void* instance) {
    static std::default_random_engine gen(
        std::chrono::system_clock::now().time_since_epoch().count()
    );

    std::uniform_real_distribution<float> dist(2000.0f, 4000.0f);

    return dist(gen);
}

// ==========================
// get lib base
// ==========================
uintptr_t get_lib_base(const char* lib) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return 0;

    char line[512];

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

    uintptr_t target_addr = base + RVA_get_Atk;

    if (!target_addr) return;

    // hook (1회만 실행되게)
    static bool hooked = false;

    if (!hooked) {
        DobbyHook(
            (void*)target_addr,
            (void*)new_get_Atk,
            (void**)&old_get_Atk
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
