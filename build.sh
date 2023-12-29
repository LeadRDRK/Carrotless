#!/usr/bin/env bash
set -e

if [[ -z "$ANDROID_NDK_ROOT" ]]; then
    echo "ANDROID_NDK_ROOT must be set"
    exit 1
fi

if [[ -z "$ANDROID_PLATFORM" ]]; then
    ANDROID_PLATFORM=24
fi


build() {
    echo "---- Building for $1"
    mkdir -p "build/$1"
    pushd "build/$1"
    cmake ../.. -GNinja -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" -DANDROID_PLATFORM="$ANDROID_PLATFORM" -DANDROID_ABI="$1" -DCMAKE_BUILD_TYPE=Release
    ninja
    popd
}

build arm64-v8a
build armeabi-v7a
