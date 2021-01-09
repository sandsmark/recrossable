#!/bin/bash
cd /app
qmake
make clean
rm recrossable
make
