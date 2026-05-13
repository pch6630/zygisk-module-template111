#include <jni.h>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include "zygisk.hpp"
#include "dobby.h"

using zygisk::Api;
using zygisk::AppSpecializeArgs;

// 전역 변수: 원래의 공격력 함수 주소를 저장
float (*old_get_Atk)(void* instance);

// [데미지 조작 함수] - 호출될 때마다 2000.0 ~ 4000.0 사이의 랜덤값 반환
float new_get_Atk(void* instance) {
    // 실행 시마다 시드를 다르게 설정하여 완벽한 난수 생성
    static std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> distribution(2000.0f, 4000.0f);
    
    return distribution(generator); 
}

// 후킹을 실행하는 별도 스레드
void hack_thread() {
    void* handle = nullptr;
    
    // 게임 엔진인 libil2cpp.so가 메모리에 로드될 때까지 대기
    while (!handle) {
        handle = dobby_dlopen("libil2cpp.so", RTLD_NOW);
        if (!handle) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    // il2cpp 엔진에서 함수 이름으로 주소를 찾아주는 핵심 함수 확보
    typedef void* (*t_il2cpp_resolve_icall)(const char* name);
    auto il2cpp_resolve_icall = (t_il2cpp_resolve_icall)dobby_dlsym(handle, "il2cpp_resolve_icall");

    if (il2cpp_resolve_icall) {
        // [자동 업데이트 핵심] 오프셋 대신 함수 이름을 사용함
        // dump.cs에서 확인된 CommonAttribute의 get_Atk 함수를 추적
        void* target_addr = il2cpp_resolve_icall("CommonAttribute::get_Atk");
        
        if (target_addr) {
            // Dobby 라이브러리를 사용하여 함수 낚아채기 실행
            DobbyHook(target_addr, (void*)new_get_Atk, (void**)&old_get_Atk);
        }
    }
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        
        // 사용자가 제공한 패키지명: com.crunchyroll.bleachsoulres
        if (process && strstr(process, "com.crunchyroll.bleachsoulres")) {
            api->setOption(zygisk::Option::DLCLOSE_PROTECT_HANDLE);
            // 게임 프로세스 내에서만 후킹 스레드 시작
            std::thread(hack_thread).detach();
        }
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

private:
    Api *api;
    JNIEnv *env;
};

REGISTER_ZYGISK_MODULE(MyModule)
