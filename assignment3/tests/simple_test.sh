#!/bin/bash


make clean_all
make minizpar

cd ./tests

cp ../utility.hpp .
cp ./utility.hpp ./utility-copy.hpp
../minizpar -C 1 ./utility-copy.hpp
../minizpar -D 1 ./utility-copy.hpp.zip
if diff ./utility-copy.hpp ./utility.hpp > /dev/null; then
    echo "Test passed"
else
    echo "Error"
fi
rm ./utility.hpp ./utility-copy.hpp