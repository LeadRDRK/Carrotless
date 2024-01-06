#include <sstream>
#include <thread>
#include "stdinclude.hpp"
#include "hook.h"
#include "il2cpp_hook.h"
#include "localify/localify.h"
#include "logger/logger.h"
#include "native_bridge_itf.h"

using namespace std;
using namespace localify;
using namespace logger;

bool g_enable_logger = false;
int g_max_fps = -1;
float g_ui_animation_scale = 1.0f;
bool g_ui_use_system_resolution = false;
float g_resolution_3d_scale = 1.0f;
bool g_replace_to_builtin_font = false;
bool g_replace_to_custom_font = false;
string g_font_assetbundle_path;
string g_font_asset_name;
string g_tmpro_font_asset_name;
bool g_dump_entries = false;
bool g_dump_db_entries = false;
bool g_static_entries_use_hash = false;
bool g_static_entries_use_text_id_name = false;
int g_graphics_quality = -1;
int g_anti_aliasing = -1;
bool g_force_landscape = false;
float g_force_landscape_ui_scale = 0.5f;
bool g_ui_loading_show_orientation_guide = true;
bool g_restore_notification = false;
unordered_map<string, ReplaceAsset> g_replace_assets;
string g_replace_assetbundle_file_path;
bool g_character_system_text_caption = false;
int g_cyspring_update_mode = -1;
bool g_hide_now_loading = false;
// Dicts (for master.mdb)
string g_text_data_dict_path;
string g_character_system_text_dict_path;
string g_race_jikkyo_comment_dict_path;
string g_race_jikkyo_message_dict_path;

string text_id_dict;

bool isGame(const char *pkgNm) {
    if (!pkgNm)
        return false;
    if (Game::IsPackageNameEqualsByGameRegion(pkgNm, Game::Region::JAP) ||
        Game::IsPackageNameEqualsByGameRegion(pkgNm, Game::Region::KOR) ||
        Game::IsPackageNameEqualsByGameRegion(pkgNm, Game::Region::TWN)) {
        LOGI("detect package: %s", pkgNm);
        return true;
    }
    return false;
}

/*bool isSettings(const char *pkgNm) {
    if (!pkgNm)
        return false;
    if ("com.kimjio.umamusumelocalify.settings"s == pkgNm) {
        return true;
    }
    return false;

}*/

/*HOOK_DEF(jstring, getModuleVersion,
         JNIEnv *env,
         jclass) {
    return env->NewStringUTF(Module::moduleVersionName);
}*/

optional<vector<string>> read_config();

HOOK_DEF(bool, il2cpp_init, const char *domain_name) {
    const bool result = orig_il2cpp_init(domain_name);

    auto dict = read_config();

    logger::init_logger();
    il2cpp_hook_init(il2cpp_handle);
    if (dict.has_value()) {
        localify::load_textdb(get_application_version(), &dict.value());
    }
    if (!text_id_dict.empty()) {
        localify::load_textId_textdb(text_id_dict);
    }
    localify::load_dicts();
    
    il2cpp_hook();
    DobbyDestroy(addr_il2cpp_init);
    return result;
}

bool isCriWareInit = false;

enum class ProcessStatus {
    INIT,
    DONE,
    NONE,
};

ProcessStatus dlopen_process(const char *name, void *handle) {
    if (!il2cpp_handle) {
        if (name != nullptr && strstr(name, "libil2cpp.so")) {
            il2cpp_handle = handle;
            LOGI("Got il2cpp handle!");
            addr_il2cpp_init = dlsym(il2cpp_handle, "il2cpp_init");
            DobbyHook(addr_il2cpp_init,
                      reinterpret_cast<void *>(new_il2cpp_init),
                      reinterpret_cast<void **>(&orig_il2cpp_init));
            return ProcessStatus::INIT;
        }
    }
    if (name != nullptr && strstr(name, "libcri_ware_unity.so") && !isCriWareInit) {
        isCriWareInit = true;
        il2cpp_load_assetbundle();
        return ProcessStatus::DONE;
    }
    /*if (!il2cpp_handle && !app_handle) {
        if (name != nullptr && strstr(name, "libapp.so")) {
            app_handle = handle;
            return ProcessStatus::DONE;
        }
    }*/
    return ProcessStatus::NONE;
}

HOOK_DEF(void*, do_dlopen, const char *name, int flags) {
    void *handle = orig_do_dlopen(name, flags);
    if (dlopen_process(name, handle) == ProcessStatus::DONE) {
        DobbyDestroy(addr_do_dlopen);
    }
    return handle;
}

// android/platform/bionic/linker/dlfcn.cpp
HOOK_DEF(void*, __loader_dlopen, const char *filename, int flags, const void *caller_addr) {
    void *handle = orig___loader_dlopen(filename, flags, caller_addr);
    if (dlopen_process(filename, handle) == ProcessStatus::DONE) {
        DobbyDestroy(addr___loader_dlopen);
    }
    return handle;
}

optional<vector<string>> read_config() {
    ifstream config_stream{
            string(SDCARD_DATA_PATH "/").append(Game::GetCurrentPackageName()).append(
                    "/config.json")};
    vector<string> dicts{};

    if (!config_stream.is_open()) {
        LOGW("config.json not loaded.");
        return nullopt;
    }

    LOGI("config.json loaded.");

    rapidjson::IStreamWrapper wrapper{config_stream};
    rapidjson::Document document;

    document.ParseStream(wrapper);

    if (!document.HasParseError()) {
        if (document.HasMember("enableLogger")) {
            g_enable_logger = document["enableLogger"].GetBool();
        }
        if (document.HasMember("dumpStaticEntries")) {
            g_dump_entries = document["dumpStaticEntries"].GetBool();
        }
        if (document.HasMember("dumpDbEntries")) {
            g_dump_db_entries = document["dumpDbEntries"].GetBool();
        }
        if (document.HasMember("staticEntriesUseHash")) {
            g_static_entries_use_hash = document["staticEntriesUseHash"].GetBool();
        }
        if (document.HasMember("staticEntriesUseTextIdName")) {
            g_static_entries_use_text_id_name = document["staticEntriesUseTextIdName"].GetBool();
        }
        if (document.HasMember("maxFps")) {
            g_max_fps = document["maxFps"].GetInt();
        }
        if (document.HasMember("uiAnimationScale")) {
            g_ui_animation_scale = document["uiAnimationScale"].GetFloat();
        }
        if (document.HasMember("uiUseSystemResolution")) {
            g_ui_use_system_resolution = document["uiUseSystemResolution"].GetBool();
        }
        if (document.HasMember("resolution3dScale")) {
            g_resolution_3d_scale = document["resolution3dScale"].GetFloat();
        }
        if (document.HasMember("replaceFont")) {
            g_replace_to_builtin_font = document["replaceFont"].GetBool();
        }
        if (!document.HasMember("replaceFont") && document.HasMember("replaceToBuiltinFont")) {
            g_replace_to_builtin_font = document["replaceToBuiltinFont"].GetBool();
        }
        if (document.HasMember("replaceToCustomFont")) {
            g_replace_to_custom_font = document["replaceToCustomFont"].GetBool();
        }
        if (document.HasMember("fontAssetBundlePath")) {
            g_font_assetbundle_path = string(document["fontAssetBundlePath"].GetString());
        }
        if (document.HasMember("fontAssetName")) {
            g_font_asset_name = string(document["fontAssetName"].GetString());
        }
        if (document.HasMember("tmproFontAssetName")) {
            g_tmpro_font_asset_name = string(document["tmproFontAssetName"].GetString());
        }
        if (document.HasMember("graphicsQuality")) {
            g_graphics_quality = document["graphicsQuality"].GetInt();
            if (g_graphics_quality < -1) {
                g_graphics_quality = -1;
            }
            if (g_graphics_quality > 4) {
                g_graphics_quality = 3;
            }
        }
        if (document.HasMember("antiAliasing")) {
            g_anti_aliasing = document["antiAliasing"].GetInt();
            vector<int> options = {0, 2, 4, 8, -1};
            g_anti_aliasing =
                    find(options.begin(), options.end(), g_anti_aliasing) - options.begin();
        }
        if (document.HasMember("forceLandscape")) {
            g_force_landscape = document["forceLandscape"].GetBool();
        }
        if (document.HasMember("forceLandscapeUiScale")) {
            g_force_landscape_ui_scale = document["forceLandscapeUiScale"].GetFloat();
            if (g_force_landscape_ui_scale <= 0) {
                g_force_landscape_ui_scale = 1;
            }
        }
        if (document.HasMember("uiLoadingShowOrientationGuide")) {
            g_ui_loading_show_orientation_guide = document["uiLoadingShowOrientationGuide"].GetBool();
        }
        /*if (document.HasMember("restoreNotification")) {
            g_restore_notification = document["restoreNotification"].GetBool();
        }*/
        if (document.HasMember("replaceAssetsPath")) {
            auto replaceAssetsPath = localify::u8_u16(document["replaceAssetsPath"].GetString());
            if (!replaceAssetsPath.starts_with(u"/")) {
                replaceAssetsPath.insert(0, u16string(U_SDCARD_DATA_PATH "/").append(
                        localify::u8_u16(Game::GetCurrentPackageName())).append(u"/"));
            }
            if (filesystem::exists(replaceAssetsPath) &&
                filesystem::is_directory(replaceAssetsPath)) {
                for (auto &file: filesystem::directory_iterator(replaceAssetsPath)) {
                    if (file.is_regular_file()) {
                        g_replace_assets.emplace(file.path().filename().string(),
                                                 ReplaceAsset{file.path().string(), nullptr});
                    }
                }
            }
        }

        if (document.HasMember("replaceAssetBundleFilePath")) {
            auto replaceAssetBundleFilePath = localify::u8_u16(
                    document["replaceAssetBundleFilePath"].GetString());
            if (!replaceAssetBundleFilePath.starts_with(u"/")) {
                replaceAssetBundleFilePath.insert(0, u16string(U_SDCARD_DATA_PATH "/").append(
                        localify::u8_u16(Game::GetCurrentPackageName())).append(u"/"));
            }
            if (filesystem::exists(replaceAssetBundleFilePath) &&
                filesystem::is_regular_file(replaceAssetBundleFilePath)) {
                g_replace_assetbundle_file_path = localify::u16_u8(replaceAssetBundleFilePath);
            }
        }

        // master.mdb dicts
        if (document.HasMember("text_data_dict")) {
            g_text_data_dict_path = document["text_data_dict"].GetString();
        }

        if (document.HasMember("character_system_text_dict")) {
            g_character_system_text_dict_path = document["character_system_text_dict"].GetString();
        }

        if (document.HasMember("race_jikkyo_comment_dict")) {
            g_race_jikkyo_comment_dict_path = document["race_jikkyo_comment_dict"].GetString();
        }

        if (document.HasMember("race_jikkyo_message_dict")) {
            g_race_jikkyo_message_dict_path = document["race_jikkyo_message_dict"].GetString();
        }

        //////

        if (document.HasMember("characterSystemTextCaption")) {
            g_character_system_text_caption = document["characterSystemTextCaption"].GetBool();
        }

        if (document.HasMember("cySpringUpdateMode")) {
            g_cyspring_update_mode = document["cySpringUpdateMode"].GetInt();
            vector<int> options = {0, 1, 2, 3, -1};
            g_cyspring_update_mode =
                    find(options.begin(), options.end(), g_cyspring_update_mode) - options.begin();
        } else if (g_max_fps > 30) {
            g_cyspring_update_mode = 1;
        }

        if (document.HasMember("hideNowLoading")) {
            g_hide_now_loading = document["hideNowLoading"].GetBool();
        }

        if (document.HasMember("textIdDict")) {
            text_id_dict = document["textIdDict"].GetString();
        }

        if (document.HasMember("dicts")) {
            auto &dicts_arr = document["dicts"];
            auto len = dicts_arr.Size();

            for (size_t i = 0; i < len; ++i) {
                auto dict = dicts_arr[i].GetString();

                dicts.emplace_back(dict);
            }
        }
    }

    config_stream.close();
    return dicts;
}

void *GetNativeBridgeLoadLibrary(void *fallbackAddress) {
    void *handle = dlopen(GetNativeBridgeLibrary().data(), RTLD_NOW);
    // clear error
    dlerror();
    if (handle) {
        auto itf = reinterpret_cast<NativeBridgeCallbacks *>(dlsym(handle, "NativeBridgeItf"));
        LOGI("NativeBridgeItf version: %d", itf->version);
        if (GetAndroidApiLevel() >= 26) {
            return reinterpret_cast<void *>(itf->loadLibraryExt);
        }
        return reinterpret_cast<void *>(itf->loadLibrary);
    }
    return fallbackAddress;
}

void hack_thread(void *arg [[maybe_unused]]) {
    LOGI("%s hack thread: %d", ABI, gettid());
    const int api_level = GetAndroidApiLevel();
    LOGI("%s api level: %d", ABI, api_level);

    if (IsABIRequiredNativeBridge()) {
        LOGE("Unsupported ABI " ABI);
        return;
    }

#if defined(__aarch64__)
#   define LINKER_IMAGE "linker64"
#else
#   define LINKER_IMAGE "linker"
#endif

    // A6, A7, A7.1      (api >= 23): __dl_open
    // A8, A8.1          (api >= 26): __dl__Z8__dlopenPKciPKv
    // A9, A10, A12, A13 (api >= 28): __dl___loader_dlopen
    if (api_level >= 28) {
        addr___loader_dlopen = DobbySymbolResolver(LINKER_IMAGE, "__dl___loader_dlopen");
        LOGI("__loader_dlopen at: %p", addr___loader_dlopen);
    }
    else if (api_level >= 26) {
        addr___loader_dlopen = DobbySymbolResolver(LINKER_IMAGE, "__dl__Z8__dlopenPKciPKv");
        LOGI("__dl__Z8__dlopenPKciPKv at: %p", addr___loader_dlopen);
    }
    else {
        addr___loader_dlopen = DobbySymbolResolver(LINKER_IMAGE, "__dl_open");
        LOGI("__dl_open at: %p", addr___loader_dlopen);
    }

    DobbyHook(
        addr___loader_dlopen,
        reinterpret_cast<void *>(new___loader_dlopen),
        reinterpret_cast<void **>(&orig___loader_dlopen)
    );
}

/*void hack_settings_thread(void *arg [[maybe_unused]]) {
    const int api_level = GetAndroidApiLevel();
    LOGI("%s api level: %d", ABI, api_level);
    if (api_level >= 30) {
        void *addr = DobbySymbolResolver(nullptr,
                                         "__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv");
        if (addr) {
            LOGI("%s do_dlopen at: %p", ABI, addr);
            DobbyHook(addr, reinterpret_cast<void *>(new_do_dlopen_V24),
                      reinterpret_cast<void **>(&orig_do_dlopen_V24));

        }
    } else if (api_level >= 26) {
        void *libdl_handle = dlopen("libdl.so", RTLD_LAZY);
        void *addr = dlsym(libdl_handle, "__loader_dlopen");

        if (addr) {
            LOGI("__loader_dlopen at: %p", addr);
            DobbyHook(addr, reinterpret_cast<void *>(new___loader_dlopen),
                      reinterpret_cast<void **>(&orig___loader_dlopen));

        }
    } else if (api_level >= 24) {
        void *addr = DobbySymbolResolver(nullptr,
                                         "__dl__Z9do_dlopenPKciPK17android_dlextinfoPv");
        if (addr) {
            LOGI("do_dlopen at: %p", addr);
            DobbyHook(addr, reinterpret_cast<void *>(new_do_dlopen_V24),
                      reinterpret_cast<void **>(&orig_do_dlopen_V24));
        }
    } else {
        void *addr = DobbySymbolResolver(nullptr,
                                         "__dl__Z9do_dlopenPKciPK17android_dlextinfo");
        if (addr) {
            LOGI("do_dlopen at: %p", addr);
            DobbyHook(addr, reinterpret_cast<void *>(new_do_dlopen_V19),
                      reinterpret_cast<void **>(&orig_do_dlopen_V19));
        }
    }

    while (!app_handle) {
        sleep(1);
    }

    auto getModuleVersion = dlsym(app_handle,
                                  "Java_com_kimjio_umamusumelocalify_settings_ModuleUtils_getModuleVersion");
    DobbyHook(getModuleVersion, reinterpret_cast<void *>(new_getModuleVersion),
              reinterpret_cast<void **>(&orig_getModuleVersion));
}*/
