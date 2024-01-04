#include <fcntl.h>

#include <sys/mman.h>

#include "stdinclude.hpp"
#include "hook.h"

#include "elf_util.h"

#include <jni.h>
#include <dlfcn.h>

void hook() {
    Game::currentGameRegion = Game::CheckPackageNameByDataPath();
    if (Game::currentGameRegion == Game::Region::UNKNOWN) {
        LOGW("Region UNKNOWN...");
        return;
    }
    int ret;
    pthread_t t;
    ret = pthread_create(&t, nullptr,
                            reinterpret_cast<void *(*)(void *)>(hack_thread), nullptr);
    if (ret != 0) {
        LOGE("can't create thread: %s\n", strerror(ret));
    }
}

typedef jint (*JNI_OnLoad_Fun)(JavaVM*);

extern "C" {

jint JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
{
    void* handle = dlopen("libmain_orig.so", RTLD_LAZY);
    JNI_OnLoad_Fun origOnLoad = reinterpret_cast<JNI_OnLoad_Fun>(dlsym(handle, "JNI_OnLoad"));
    hook();
    return origOnLoad(vm);
}

}
