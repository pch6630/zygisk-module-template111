#include <jni.h>
#include <string>
#include "zygisk.hpp"
#include "dobby.h"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

// 원래 함수를 저장할 포인터
float (*old_get_Atk)(void* instance);

// 우리가 만든 가짜 함수 (공격력 조작)
float new_get_Atk(void* instance) {
    // 원래 값을 무시하고 99999.0f를 반환 (데미지 폭증)
    return 99999.0f; 
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        // 타겟 게임 패키지명 확인 (예: com.linegames.bw)
        if (process && strstr(process, "com.linegames.bw")) { 
            api->setOption(zygisk::Option::DLCLOSE_PROTECT_HANDLE);
        }
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

    void postAppSpecialize(const AppSpecializeArgs *args) override {
        // libil2cpp.so가 로드될 때까지 기다렸다가 후킹 실행
        // 실제 구현 시에는 별도의 스레드나 헬퍼 함수를 통해 
        // il2cpp_resolve_icall("CommonAttribute::get_Atk") 주소를 찾아 DobbyHook을 적용합니다.
    }

private:
    Api *api;
    JNIEnv *env;
};

REGISTER_ZYGISK_MODULE(MyModule)
