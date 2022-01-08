#!/bin/bash

make clean
make
./user -p 25555 -n localhost
