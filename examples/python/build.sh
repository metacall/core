#!/bin/bash

gcc `python3.4-config --cflags` main.c -o main.exe `python3.4-config --ldflags`

exit 0
