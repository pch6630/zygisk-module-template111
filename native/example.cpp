#include <jni.h>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include <dlfcn.h>
#include <cstring>

#include "zygisk.hpp"
#include "dobby.h"

using zygisk::Api;
using zygisk::AppSpecializeArgs;

// 원래 함수 포인터
float (*old_get_Atk)(void* instance);

// 랜덤 데미지
float new_get_Atk(void* instance) {
    static std::default_random_engine generator(
        std::chrono::system_clock::now().time_since_epoch().count()
    );

    std::uniform_real_distribution<float> distribution(2000.0f, 4000.0f);
    return distribution(generator);
}

// 후킹 스레드
void hack_thread() {
    void* handle = nullptr;

    // libil2cpp 로드 대기
    while (!handle) {
        handle = dlopen("libil2cpp.so", RTLD_NOW | RTLD_LOCAL);
        if (!handle) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    // il2cpp resolve icall 가져오기
    typedef void* (*t_il2cpp_resolve_icall)(const char*);
    auto il2cpp_resolve_icall =
        (t_il2cpp_resolve_icall)dlsym(handle, "il2cpp_resolve_icall");

    if (!il2cpp_resolve_icall) return;

    void* target_addr =
        il2cpp_resolve_icall("CommonAttribute::get_Atk");

    if (target_addr) {
        DobbyHook(
            target_addr,
            (void*)new_get_Atk,
            (void**)&old_get_Atk
        );
    }
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        const char *process =
            env->GetStringUTFChars(args->nice_name, nullptr);

        if (process && strstr(process, "com.crunchyroll.bleachsoulres")) {

            // ❌ 삭제됨: setOption(DLCLOSE_PROTECT_HANDLE)

            std::thread(hack_thread).detach();
        }

        env->ReleaseStringUTFChars(args->nice_name, process);
    }

private:
    Api *api;
    JNIEnv *env;
};

REGISTER_ZYGISK_MODULE(MyModule)
