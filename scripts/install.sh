#!/bin/bash

gcc -Os -s main.c -o rsc
mv rsc /usr/local/bin/ # or place inside your project to call as ./rsc, the binary is 16kb