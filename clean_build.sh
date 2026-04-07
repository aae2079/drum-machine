#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
BUILD_DIR="$SCRIPT_DIR/build"

cd "$BUILD_DIR" || { echo "Build directory not found!"; exit 1; }
rm -rf ./*


if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Running on macOS"
    export PKG_CONFIG_PATH="$(brew --prefix portaudio)/lib/pkgconfig"
    CXX_COMPILER=$(xcrun -find c++)
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "Running on Linux"
    export PKG_CONFIG_PATH="/usr/lib/x86_64-linux-gnu/pkgconfig"
    CXX_COMPILER=/usr/bin/g++
fi

cmake .. \
  -DCMAKE_VERBOSE_MAKEFILE=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER="$CXX_COMPILER"\
  >> build.log 2>&1
cmake --build . >> build.log 2>&1
if [ $? -ne 0 ]; then
    echo "Build failed! Check build/build.log for details."
    exit 1
fi
echo "Build completed successfully. Logs are available in build/build.log"

echo "Creating links to OpenGL shaders ... "
ln -sf "$SCRIPT_DIR"/src/frontend/default* "$BUILD_DIR"/bin/
