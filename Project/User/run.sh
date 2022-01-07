#!/bin/bash

make clean
make
./User -p 25555 -n localhost
