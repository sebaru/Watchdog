#!/bin/bash

# Build script for ABLS Habitat Agent
# This script builds the project in the 'build' directory using CMake

set -e  # Exit on error

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo "Building ABLS Habitat AGENT..."
echo "Project directory: $PROJECT_DIR"
echo "Build directory: $BUILD_DIR"
echo "Number of processors: $(nproc)"

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

# Navigate to build directory
cd "$BUILD_DIR"

# Run CMake to generate build files
echo "Running CMake..."
cmake ..

# Build the project
echo "Building project..."
cmake --build . -- -j$(nproc)

echo ""
echo "Build completed successfully!"
echo "Built artifacts are in: $BUILD_DIR"
echo "Install with ./install.sh"
