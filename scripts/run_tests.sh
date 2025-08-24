#!/bin/bash
# scripts/run_tests.sh

BUILD_DIR="build/"
CONFIG="${1:-Release}"
MODULES="${2:-all}"

echo "Building tests with config: $CONFIG, modules: $MODULES"
cmake -B $BUILD_DIR -DCMAKE_BUILD_TYPE=$CONFIG -DBUILD_TESTS=ON

if [ "$MODULES" != "all" ]; then
    cmake -B $BUILD_DIR -DBUILD_CORE_MODULE=OFF -DBUILD_NETWORK_MODULE=OFF -DBUILD_LOGGING_MODULE=OFF
    
    IFS=',' read -ra MODULE_ARRAY <<< "$MODULES"
    for module in "${MODULE_ARRAY[@]}"; do
        cmake -B $BUILD_DIR -DBUILD_${module^^}_MODULE=ON
    done
fi

cmake --build $BUILD_DIR --config $CONFIG --target test_runner

echo "Running tests..."
$BUILD_DIR/bin/${CONFIG}/test_runner --gtest_output="xml:test_results.xml"

echo "Tests completed. Results saved to test_results.xml"