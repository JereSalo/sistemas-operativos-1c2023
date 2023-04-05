#!/bin/bash
FILE=fileSystem
make $FILE
if test -f "./$FILE"; then
    valgrind --tool=helgrind ./$FILE
fi