#!/bin/bash

LD_PRELOAD=.libs/libfaultinject.so $1 || exit 1
