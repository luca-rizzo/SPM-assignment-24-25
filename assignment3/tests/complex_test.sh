#!/bin/bash

set -e

make clean_all
make minizpar

TEST_DIR="./tests"
ORIG_DIR="orig-files"
COPY_DIR="copy-files"

cd "$TEST_DIR" || exit 1
mkdir -p "$ORIG_DIR"
mkdir -p "$COPY_DIR"

dd if=/dev/urandom of="${ORIG_DIR}/first.dat" bs=50M count=1 status=none
dd if=/dev/urandom of="${ORIG_DIR}/second.dat" bs=50M count=1 status=none

cp "${ORIG_DIR}/first.dat" "${COPY_DIR}/first-copy.dat"
cp "${ORIG_DIR}/second.dat" "${COPY_DIR}/second-copy.dat"

OMP_NUM_THREADS=40 ../minizpar -r 1 -C 1 "$COPY_DIR/"

rm -f "${COPY_DIR}"/*.dat

OMP_NUM_THREADS=40 ../minizpar -r 1 -D 1 "$COPY_DIR/"

pass=true

for name in first second; do
    if ! cmp -s "${ORIG_DIR}/${name}.dat" "${COPY_DIR}/${name}-copy.dat"; then
        echo "Error: ${name}.dat mismatch"
        pass=false
    fi
done

if [ "$pass" = true ]; then
    echo "Test passed"
else
    echo "Test failed"
fi

rm -rf "$ORIG_DIR" "$COPY_DIR"
