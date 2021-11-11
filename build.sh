#!/bin/bash

set -e

TOOLCHAIN_PATH=""
TOOLCHAIN_PREFIX="arm-linux-gnueabihf-"

ARCH="arm"
CONFIG="msm8212_defconfig"
OUT="./out"

if [ "${TOOLCHAIN_PATH}" != "" ]; then
    if [ ! -d "${TOOLCHAIN_PATH}" ]; then
        echo "Toolchain doesn't exists"
        exit 1
    fi

    export PATH="$(pwd)/${TOOLCHAIN_PATH}:${PATH}"
fi

mkdir -p "${OUT}"

MAKEFLAGS=(
    "O=${OUT}"
    "CROSS_COMPILE=ccache ${TOOLCHAIN_PREFIX}"
    "ARCH=${ARCH}"
    "-j$(nproc --all)"
)

#make "${MAKEFLAGS[@]}" clean
#make "${MAKEFLAGS[@]}" mrproper
make "${MAKEFLAGS[@]}" "${CONFIG}"
make "${MAKEFLAGS[@]}"
