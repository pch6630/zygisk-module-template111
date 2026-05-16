#pragma once

#include <jni.h>
#include <stdint.h>

#define ZYGISK_API_VERSION 3

namespace zygisk {

struct Api;
struct AppSpecializeArgs;
struct ServerSpecializeArgs;

class ModuleBase {
public:
    virtual void onLoad([[maybe_unused]] Api *api, [[maybe_unused]] JNIEnv *env) {}
    virtual void preAppSpecialize([[maybe_unused]] AppSpecializeArgs *args) {}
    virtual void postAppSpecialize([[maybe_unused]] const AppSpecializeArgs *args) {}
    virtual void preServerSpecialize([[maybe_unused]] ServerSpecializeArgs *args) {}
    virtual void postServerSpecialize([[maybe_unused]] const ServerSpecializeArgs *args) {}
};

struct AppSpecializeArgs {
    jint &uid;
    jint &gid;
    jintArray &gids;
    jint &runtime_flags;
    jobjectArray &rlimits;
    jint &mount_external;
    jstring &se_info;
    jstring &nice_name;
    jstring &instruction_set;
    jstring &app_data_dir;

    jintArray *const fds_to_ignore;
    jboolean *const is_child_zygote;
    jboolean *const is_top_app;
    jobjectArray *const pkg_data_info_list;
    jobjectArray *const whitelisted_data_info_list;
    jboolean *const mount_data_dirs;
    jboolean *const mount_storage_dirs;

    AppSpecializeArgs() = delete;
};

struct ServerSpecializeArgs {
    jint &uid;
    jint &gid;
    jintArray &gids;
    jint &runtime_flags;
    jlong &permitted_capabilities;
    jlong &effective_capabilities;

    ServerSpecializeArgs() = delete;
};

namespace internal {
struct api_table;
template <class T> void entry_impl(api_table *, JNIEnv *);
}

enum Option : int {
    FORCE_DENYLIST_UNMOUNT = 0,
    DLCLOSE_MODULE_LIBRARY = 1,
};

enum StateFlag : uint32_t {
    PROCESS_GRANTED_ROOT = (1u << 0),
    PROCESS_ON_DENYLIST = (1u << 1),
};

struct Api {
    int connectCompanion();
    uint32_t getFlags();
    void setOption(Option opt);
    void hookJniNativeMethods(JNIEnv *env, const char *className, const JNINativeMethod *methods, int numMethods);
    void *reserved;
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
    extern "C" [[gnu::visibility("default")]] void zygisk_module_entry(::zygisk::internal::api_table *table, JNIEnv *env) { \
        ::zygisk::internal::entry_impl<clazz>(table, env); \
    }

#define REGISTER_ZYGISK_COMPANION(func) \
    extern "C" [[gnu::visibility("default")]] void zygisk_companion_entry(int client_socket) { \
        func(client_socket); \
    }
