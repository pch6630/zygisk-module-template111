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

#define RVA_CalcDamage 0x1C8A420
int (*old_CalcDamage)(void* instance, void* arg1, void* arg2) = nullptr;

int new_CalcDamage(void* instance, void* arg1, void* arg2) {
    static std::default_random_engine gen(
        std::chrono::system_clock::now().time_since_epoch().count()
    );
    std::uniform_int_distribution<int> dist(2000, 3500);
    return dist(gen);
}

uintptr_t get_lib_base(const char* lib) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return 0;

    char line[512];
    uintptr_t base = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, lib)) {
            base = strtoull(line, nullptr, 16);
            break;
        }
    }
    fclose(fp);
    return base;
}

void hack_thread() {
    uintptr_t base = 0;
    while (!base) {
        base = get_lib_base("libil2cpp.so");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    uintptr_t target_addr = base + RVA_CalcDamage;
    if (!target_addr) return;

    static bool hooked = false;
    if (!hooked) {
        DobbyHook(
            (void*)target_addr,
            (void*)new_CalcDamage,
            (void**)&old_CalcDamage
        );
        hooked = true;
    }
}

// ==========================
// Zygisk Module Class
// ==========================
class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs* args) override {}

    void postAppSpecialize(const AppSpecializeArgs* args) override {
        if (!args->nice_name) return;

        const char* name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (name) {
            if (strstr(name, "com.crunchyroll.bleachsoulres")) {
                std::thread(hack_thread).detach();
            }
            env->ReleaseStringUTFChars(args->nice_name, name);
        }
    }

private:
    Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(MyModule)
