#!/usr/bin/env bash
set -e

if [[ -z "$ANDROID_NDK_ROOT" ]]; then
    echo "ANDROID_NDK_ROOT must be set"
    exit 1
fi

if [[ -z "$ANDROID_PLATFORM" ]]; then
    ANDROID_PLATFORM=24
fi

if [[ -z "$BUILD_TYPE" ]]; then
    BUILD_TYPE=Release
fi


build() {
    echo "---- Building for $1"
    mkdir -p "build/$1"
    pushd "build/$1"
    cmake ../.. -GNinja -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" -DANDROID_PLATFORM="$ANDROID_PLATFORM" -DANDROID_ABI="$1" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    ninja
    popd
}

build arm64-v8a
build armeabi-v7a

ARM64_V8A_SHA1=($(sha1sum build/arm64-v8a/libmain.so))
ARMEABI_V7A_SHA1=($(sha1sum build/armeabi-v7a/libmain.so))

cat << EOF > build/sha1.json
{
    "libmain-arm64-v8a.so": "$ARM64_V8A_SHA1",
    "libmain-armeabi-v7a.so": "$ARMEABI_V7A_SHA1"
}
EOF
cp build/arm64-v8a/libmain.so build/libmain-arm64-v8a.so
cp build/armeabi-v7a/libmain.so build/libmain-armeabi-v7a.so
