#!/bin/bash

# Install script for ABLS Habitat AGENT
# This script installs the project from the 'build' directory using CMake

set -e  # Exit on error

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo "Installing ABLS Habitat AGENT..."
echo "Project directory: $PROJECT_DIR"
echo "Build directory: $BUILD_DIR"
echo "Number of processors: $(nproc)"

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "build directory does not exist. Stopping"
    exit 1
fi

# Navigate to build directory
cd "$BUILD_DIR"

# Install the project
echo "Installing project..."
sudo cmake --install .
sudo ldconfig
sudo systemctl daemon-reload

echo ""
echo "Pour linker l'agent, utilisez Watchdogd --save --domain-uuid 'domain_uuid', --domain-secret 'domain_secret'"
echo "Ou utiliser la console https://console.abls-habitat.fr/agent/add pour vous guider"
echo "You can start the service with: sudo systemctl start Watchdogd"
