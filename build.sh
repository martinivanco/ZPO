#!/bin/sh

run_type=$1

if [ "$run_type" = "rebuild" ]; then
  rm -r build
  cmake -H. -Bbuild
  make -C build
elif [ "$run_type" = "clean" ]; then
  rm -r build
else
  cmake -H. -Bbuild
  make -C build
fi
