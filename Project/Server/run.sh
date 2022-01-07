#!/bin/bash

make clean
make
./Server -p 25555 -v
