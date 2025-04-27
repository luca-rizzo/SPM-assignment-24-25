#!/bin/bash

TARGET_DIR="${1:-test_file}"  # Usa argomento oppure default

set -e  # Esce subito in caso di errore

mkdir -p "$TARGET_DIR/large" "$TARGET_DIR/small" "$TARGET_DIR/mixed"

echo "Creating large files..."
# 1. 3 file da 100MB
for i in {1..3}; do
    dd if=/dev/urandom of="$TARGET_DIR/large/file_$i.dat" bs=100M count=1 status=none
done

# Helper per creare directory ricorsive e salvare un file lì dentro
generate_recursive_path() {
    local base=$1
    local depth=$2
    local current="$base"
    for ((d=1; d<=depth; d++)); do
        current="$current/lvl$d"
    done
    mkdir -p "$current"
    echo "$current"
}

echo "Creating small mixed files..."
# 2. File piccoli (512KB, 1MB, 2B) fino a 100MB
small_total=0
small_target=$((100 * 1024 * 1024))  # 100 MB in byte
small_index=1
while [[ $small_total -lt $small_target ]]; do
    case $((RANDOM % 3)) in
        0) size=512K;;
        1) size=1M;;
        2) size=2M;;
    esac
    depth=$(( (RANDOM % 4) + 1 ))  # profondità 1-4
    path=$(generate_recursive_path "$TARGET_DIR/small" $depth)
    file_path="$path/file_$small_index.dat"
    dd if=/dev/urandom of="$file_path" bs=$size count=1 status=none
    small_total=$((small_total + $(stat -c%s "$file_path")))
    ((small_index++))
done

echo "Creating mixed size files..."
# 3. File di dimensioni varie da 256KB a 80MB fino a 100MB
mixed_total=0
mixed_target=$((100 * 1024 * 1024))
mixed_index=1
while [[ $mixed_total -lt $mixed_target ]]; do
    # Dimensione random da 256K a 80M
    size_kb=$((256 + RANDOM % (80*1024 - 256)))
    depth=$(( (RANDOM % 5) + 1 ))  # profondità 1-5
    path=$(generate_recursive_path "$TARGET_DIR/mixed" $depth)
    file_path="$path/file_$mixed_index.dat"
    dd if=/dev/urandom of="$file_path" bs=1K count=$size_kb status=none
    mixed_total=$((mixed_total + $(stat -c%s "$file_path")))
    ((mixed_index++))
done

echo "Done creating test files."
