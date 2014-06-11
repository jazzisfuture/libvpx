#!/bin/sh
##
##  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
##
##  Use of this source code is governed by a BSD-style license
##  that can be found in the LICENSE file in the root of the source
##  tree. An additional intellectual property rights grant can be found
##  in the file PATENTS.  All contributing project authors may
##  be found in the AUTHORS file in the root of the source tree.
##
##
## This script generates 'VPX.framework'. An iOS app can encode and decode VPx
## video by including 'VPX.framework'.
##
## Run ./iosbuild.sh to generate 'VPX.framework' in the root of the libvpx
## source directory (previous build will be erased if it exists).
##
set -e

if [ ! -x "configure" ]; then
  echo "${0%.*} must be run from the root of the libvpx tree."
  exit 1
fi

BUILD_ROOT="_iosbuild"
DIST_DIR="_dist"
FRAMEWORK_DIR="VPX.framework"
HEADER_DIR="${FRAMEWORK_DIR}/Headers/vpx"
LIPO=$(xcrun -sdk iphoneos${SDK} -find lipo)
ORIG_PWD="$(pwd)"
TARGETS="armv6-darwin-gcc
         armv7-darwin-gcc
         armv7s-darwin-gcc
         x86-iphonesimulator-gcc
         x86_64-iphonesimulator-gcc"

# This variable is set to the last dist dir used with make dist, and reused when
# populating the framework directory to get the path to the most recent
# includes.
TARGET_DIST_DIR=""

# List of library files passed to lipo.
LIBS=""

rm -rf "${BUILD_ROOT}" "${FRAMEWORK_DIR}"

mkdir -p "${BUILD_ROOT}"
mkdir -p "${HEADER_DIR}"

cd "${BUILD_ROOT}"

build_target() {
  local target="$1"
  local old_pwd="$(pwd)"
  mkdir "${target}"
  cd "${target}"
  ../../configure --target="${target}" --disable-docs
  export DIST_DIR
  # TODO(tomfinegan): support -j; single job make sucks.
  make dist
  cd "${old_pwd}"
}

build_targets() {
  local targets="$1"
  local target
  for target in ${targets}; do
    build_target "${target}"
    TARGET_DIST_DIR="${BUILD_ROOT}/${target}/${DIST_DIR}"
    LIBS="${LIBS} ${TARGET_DIST_DIR}/lib/libvpx.a"
  done
}

build_targets "${TARGETS}"

# TODO(tomfinegan): This is purely debugging, and should be behind a verbose
# flag.
cd "${ORIG_PWD}"
for lib in ${LIBS}; do
  echo ${lib}
done

# Includes are identical for all platforms, and according to dist target
# behavior vpx_config.h and vpx_version.h aren't actually necessary for user
# apps built with libvpx. So, just copy the includes from the last target built.
# TODO(tomfinegan): The above is a lame excuse. Build common config/version
# includes that use the preprocessor to include the correct file.
cp -p "${TARGET_DIST_DIR}"/include/vpx/* "${HEADER_DIR}"
${LIPO} -create ${LIBS} -output ${FRAMEWORK_DIR}/VPX
