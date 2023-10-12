#!/bin/sh
g++ main.cpp DFT.cpp -O2 -lmpg123 -lao include/libraylib.a -fsanitize=address
