/* Copyright 2022-2023 John "topjohnwu" Wu
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

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

namespace internal {

struct api_table {
    void *v1, *v2, *v3;
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
    static module_abi abi{
        .api_version = ZYGISK_API_VERSION,
        .impl = nullptr,
        .onLoad = [](Api *api, JNIEnv *env) {
            abi.impl = new T();
            abi.impl->onLoad(api, env);
        },
        .preAppSpecialize = [](AppSpecializeArgs *args) { abi.impl->preAppSpecialize(args); },
        .postAppSpecialize = [](const AppSpecializeArgs *args) { abi.impl->postAppSpecialize(args); },
        .preServerSpecialize = [](ServerSpecializeArgs *args) { abi.impl->preServerSpecialize(args); },
        .postServerSpecialize = [](const ServerSpecializeArgs *args) { abi.impl->postServerSpecialize(args); }
    };
    table->registerModule(table, &abi);
}

} // namespace internal
} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
    extern "C" [[gnu::visibility("default")]] void zygisk_module_entry(::zygisk::internal::api_table *table, JNIEnv *env) { \
        ::zygisk::internal::entry_impl<clazz>(table, env); \
    }

#define REGISTER_ZYGISK_COMPANION(func) \
    extern "C" [[gnu::visibility("default")]] void zygisk_companion_entry(int client_socket) { \
        func(client_socket); \
    }
