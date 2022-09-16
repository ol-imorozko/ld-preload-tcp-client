#!/bin/bash
gcc -Wall -o app app.c
gcc -shared -fPIC -o lib1.so lib1.c -ldl
gcc -shared -fPIC -o lib2.so lib2.c -ldl
