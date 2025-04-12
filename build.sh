#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure with qmake
qmake ..

# Build the project
make -j4

echo "Build completed! Run ./build/protone-timeshift to launch the application."