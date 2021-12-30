#!/bin/bash

rm -rf USERS
rm -rf GROUPS
make clean
make
./Server -p 25550