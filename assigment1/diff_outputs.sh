#!/bin/bash

K_VALUES=(32 1025 10000 32768)


if [ "$#" -eq 0 ]; then
    echo "Error: No target specified"
    exit 1
fi

for K in "${K_VALUES[@]}"; do
    echo "==========================================="
    echo " Compare results with K=$K"
    echo "==========================================="

    for target in "$@"; do
        if [ ! -x "./$target" ]; then
            echo "Error: Executable ./$target not found or not executable!"
            continue
        fi
        ./$target $K 1 2>./out/$target.txt 1>/dev/null;
    done
    for out1 in "$@"; do
        for out2 in "$@"; do
                  if [[ "$out1" > "$out2" ]]; then
                    echo "Diff between "./out/$out1.txt" "./out/$out2.txt":";
                    diff "./out/$out1.txt" "./out/$out2.txt";
                    echo
                  fi
        done
    done
done