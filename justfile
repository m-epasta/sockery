default: build

PROJECT_NAME := "sockery"

build:
    #!/usr/bin/env bash
    mkdir -p artifacts
    set -eux
    gcd {{PROJECT_NAME}}
    mkdir -p build
    cd build
    cmake ..
    cmake --build .
    cp {{PROJECT_NAME}} ../artifacts/

distclean:
    rm -rf build
    rm -rf {{PROJECT_NAME}}
    rm -rf release
    rm -rf artifacts/

release:
    mkdir -p artifacts
    v run scripts/release.vsh

actions-build:
    #!/usr/bin/env bash
    set -eux
    mkdir -p build
    cd build
    cmake ..
    cmake --build .
    cp {{PROJECT_NAME}} ../artifacts/

download-assets:
    v run scripts/download_assets.vsh

fill-assets:
    v run scripts/fill_csv.vsh
