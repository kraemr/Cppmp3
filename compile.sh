#!/bin/sh
g++ main.cpp playlist.cpp stdafx.cpp  DFT.cpp -lmpg123 -lao include/libraylib.a -fsanitize=address -Wall -lfftw3
