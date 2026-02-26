#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
BUILD_DIR="$SCRIPT_DIR/build"

cd "$BUILD_DIR" || { echo "Build directory not found!"; exit 1; }
rm -rf ./*
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ >> build.log 2>&1
cmake --build . >> build.log 2>&1
echo "Build completed successfully. Logs are available in build/build.log"