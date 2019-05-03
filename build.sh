## =========================================== ##
## File: build.sh                              ##
## Project: ZPO - Timelapse correction         ##
## Author: Martin Ivanco (xivanc03)            ##
## =========================================== ##

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
