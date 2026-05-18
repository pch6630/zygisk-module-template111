#include <jni.h>
#include <thread>
#include <chrono>
#include <random>
#include <dlfcn.h>
#include <cstring>
#include <unistd.h>
#include <stdint.h>
#include <string>

#include "zygisk.hpp"

// =================================================================
// 🔗 안드로이드용 Dobby 라이브러리 메모리 패치 외부 선언
// =================================================================
extern "C" int DobbyCodePatch(void *address, uint8_t *buffer, uint32_t buffer_size);

using zygisk::Api;
using zygisk::AppSpecializeArgs;

// 💡 새로운 64비트 메모리 패치 오프셋 주소
#define RVA_TargetOffset 0x1E0D650

// 메모리 맵 스캔 함수
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

// 패치 실행 스레드
void hack_thread() {
    uintptr_t base = 0;
    
    // libil2cpp.so가 메모리에 완전히 로드될 때까지 끝까지 대기
    while (!base) {
        base = get_lib_base("libil2cpp.so");
        if (!base) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    if (base) {
        uintptr_t target_addr = base + RVA_TargetOffset;
        
        // 💡 [10배율 설정 완료] ARM64 기계어 코드 패치 데이터: MOV X0, #10 / RET
        // 대미지 관련 수치 배율을 정확히 10배로 강제 고정하는 바이트 배열입니다.
        uint8_t patch_code[] = {
            0x40, 0x01, 0x80, 0xD2, // MOV X0, #10
            0xC0, 0x03, 0x5F, 0xD6  // RET
        };

        static bool patched = false;
        if (!patched) {
            DobbyCodePatch((void*)target_addr, patch_code, sizeof(patch_code));
            patched = true;
        }
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
        if (!args || !args->nice_name) return;

        const char* name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (!name) return;

        std::string current_package(name);
        env->ReleaseStringUTFChars(args->nice_name, name);

        // 크런치롤 글로벌 서버 정식 패키지명 필터링
        if (current_package.find("com.crunchyroll.bleachsoulres") != std::string::npos) {
            std::thread(hack_thread).detach();
        }
    }

private:
    Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(MyModule)
