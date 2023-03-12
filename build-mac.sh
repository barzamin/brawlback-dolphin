# bin/sh

CMAKE_FLAGS=''
CMAKE_FLAGS+=' -DCMAKE_CXX_FLAGS="-std=c++17" -DMACOS_CODE_SIGNING=OFF -DCMAKE_BUILD_TYPE=Debug'

# Move into the build directory, run CMake, and compile the project

# Specify the build directory
BUILD_DIR="build"

# Check if the build directory already exists
if [ -d "$BUILD_DIR" ]; then
  echo "Build directory already exists."
else
  echo "Creating build directory..."
  mkdir -p "$BUILD_DIR"
fi

pushd $BUILD_DIR

# Check if the project has already been configured
if [ -f "./CMakeCache.txt" ]; then
  echo "Project already configured with CMake."
else
  echo "Configuring project with CMake..."
  cmake ${CMAKE_FLAGS} ..
fi

export LIBRARY_PATH=$LIBRARY_PATH:/usr/local/lib
make -j $(sysctl -n hw.logicalcpu)
popd

#open ./build/Binaries/Dolphin.app
