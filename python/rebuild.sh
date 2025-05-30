#!/bin/bash

set -e

echo "Build and CMake ..."

cd ../build
cmake .. -DSPM_ENABLE_SHARED=OFF -DCMAKE_INSTALL_PREFIX=./root
make install

echo "Remove old Python build and dist ..."

cd ../python
rm -rf build/ dist/

echo "Build Python package ..."

python setup.py build && pip install -e .