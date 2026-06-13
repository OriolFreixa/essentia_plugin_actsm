#!/bin/bash
# Script to build external dependencies on macOS/Linux
# Creates the build_external directory at the project root level

echo "===== Building External Dependencies ====="

# Navigate to project root
cd "$(dirname "$0")/.."
echo "Project directory: $(pwd)"

# Create and navigate to build directory
echo "Creating build directory..."
mkdir -p build_external
cd build_external
echo "Build directory: $(pwd)"

# Get number of CPU cores
if [[ "$(uname)" == "Darwin" ]]; then
  NUM_CORES=$(sysctl -n hw.ncpu)
else
  NUM_CORES=$(nproc)
fi
echo "Using $NUM_CORES CPU cores for parallel build"

# Configure with CMake
echo "Configuring with CMake..."
cmake ../external
echo "Configuration complete"

# Build with parallel jobs
echo "Building external dependencies..."
cmake --build . -- -j"$NUM_CORES"
echo "Build complete!"
