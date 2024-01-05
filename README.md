# Carrotless
Carrotless is an out-of-tree fork of [Kimjio/umamusume-localify-android](https://github.com/Kimjio/umamusume-localify-android) which aims to make it work without Zygisk (and without root). It achieves this by acting as a proxy library to Unity's JNI entry point (libmain.so), hooking localify into the game in the process.

# Building
Run `build.sh` to build the libraries. `ANDROID_NDK_ROOT` must be set before running the script.

# Installation
### UmaPatcher
The easiest way to install is by using [UmaPatcher](https://github.com/LeadRDRK/UmaPatcher) which will modify the APK, install and configure everything for you.

### Manually
1. Build or download the prebuilt libraries from the [Releases page](https://github.com/LeadRDRK/Carrotless/releases).
2. Extract the APK file of the game. You might want to use [apktool](https://apktool.org/) for this.
3. Rename the `libmain.so` file in each of the folders inside `lib` to `libmain_orig.so`.
4. Copy the proxy libraries to their corresponding folders (e.g. `libmain-arm64-v8a.so` goes to `lib/arm64-v8a`). Rename them to `libmain.so`.
5. Build the APK file and install it.

# Configuration
Carrotless is partially compatible with umamusume-localify-android but uses `/sdcard/Android/media/jp.co.cygames.umamusume` as the working directory. Simply put `config.json` and the files referenced in the `dicts` value of the config to the directory and you're all set.

Starting from v0.2.0, the original project's settings app is no longer compatible with Carrotless. If needed, you must edit the config.json file manually.

More info is available in the original project's [README](https://github.com/Kimjio/umamusume-localify-android/blob/main/README.md).

# License
[MIT License](LICENSE)
