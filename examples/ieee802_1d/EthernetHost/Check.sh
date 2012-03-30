#!/bin/bash

if [ $# -eq 0 ]; then
ls --hide="[^Test]*" | ./CompareLauncher.sh
elif [ $1 = "clean" ]; then
echo "Cleaning results"
rm -f results/*
ls --hide="[^Test]*" | ./Cleaner.sh
fi
