#!/bin/bash

# Check if the required arguments are provided
if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <MATTER_REPO_PATH> <QCC74X_SDK_TOOLCHAIN> <ZAP_INSTALL_PATH> <QCC74X_SDK_SOURCE_ROOT>"
    exit 1
fi

MATTER_REPO_PATH=$1
QCC74X_SDK_TOOLCHAIN=$2
ZAP_INSTALL_PATH=$3
QCC74X_SDK_SOURCE_ROOT=$4

export MATTER_EXAMPLE_DIR=$(dirname "$(readlink -f "$0")")
MATTER_APP=$(pwd)

MATTER_BUILD_OPTION=qcc74x-qcc743dk-light-wifi
CONFIG_MATTER_MFD=$(grep -E "CONFIG_MATTER_MFD.*1" proj.conf)
if [[ "$CONFIG_MATTER_MFD" != "" ]]; then
    MATTER_BUILD_OPTION=${MATTER_BUILD_OPTION}-mfd
fi

cd "$MATTER_REPO_PATH" || { echo "no such path $MATTER_REPO_PATH"; exit 1; }
rm -rf out/${MATTER_BUILD_OPTION}

export QCC74X_SDK_TOOLCHAIN
export ZAP_INSTALL_PATH
export QCC74X_SDK_SOURCE_ROOT

source ./scripts/activate.sh -p qcc74x
./scripts/build/build_examples.py --target ${MATTER_BUILD_OPTION} build
cd "$MATTER_APP" || { echo "no such path $MATTER_APP"; exit 1; }

check_and_create_dest() {
    if [ ! -d "$1" ]; then
        echo "no such dir, create: $1"
        mkdir -p "$1"
        if [ $? -ne 0 ]; then
            echo "create lib fail: $1"
            exit 1
        fi
    fi
}

copy_files() {
    local src_path=$1
    local dest_path=$2
    local file_type=$3
    find "$src_path" -type f -name "$file_type" -exec cp {} "$dest_path" \;
    if [ $? -eq 0 ]; then
        echo "copy $file_type success."
    else
        echo "copy $file_type fail."
    fi
}

check_and_create_dest "$MATTER_APP/lib"

copy_files "$MATTER_REPO_PATH/out/${MATTER_BUILD_OPTION}/obj/third_party/connectedhomeip/src/app/clusters" "$MATTER_APP/lib" "*.o"
copy_files "$MATTER_REPO_PATH/out/${MATTER_BUILD_OPTION}/obj/third_party/connectedhomeip/src/app/util" "$MATTER_APP/lib" "qcc74x-lighting*.o"
cp "$MATTER_REPO_PATH/out/${MATTER_BUILD_OPTION}/chip-qcc743-lighting-example.out.a" "$MATTER_APP/lib"
if [ $? -eq 0 ]; then
    echo "copy chip-qcc743-lighting-example.out.a success"
else
    echo "copy chip-qcc743-lighting-example.out.a fail"
fi
copy_files "$MATTER_REPO_PATH/out/${MATTER_BUILD_OPTION}/lib" "$MATTER_APP/lib" "*.a"
copy_files "$MATTER_REPO_PATH/out/${MATTER_BUILD_OPTION}/obj/third_party/connectedhomeip/src/app/server/lib" "$MATTER_APP/lib" "*.a"
copy_files "$MATTER_REPO_PATH/out/${MATTER_BUILD_OPTION}/obj/third_party/connectedhomeip/zzz_generated/app-common/app-common/zap-generated/attributes" "$MATTER_APP/lib" "*.o"
copy_files "$MATTER_REPO_PATH/out/${MATTER_BUILD_OPTION}/obj/gen/app" "$MATTER_APP/lib" "*.o"
copy_files "$MATTER_REPO_PATH/out/${MATTER_BUILD_OPTION}/obj/gen/zapgen/zap-generated" "$MATTER_APP/lib" "*.o"
copy_files "$MATTER_REPO_PATH/out/${MATTER_BUILD_OPTION}/obj/third_party/connectedhomeip/src/app/reporting" "$MATTER_APP/lib" "qcc74x-lighting*.o"
