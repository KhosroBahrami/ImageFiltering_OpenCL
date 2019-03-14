#!/bin/bash

g++ -o main boxFilter.cpp main.cpp -lOpenCL -lm `pkg-config --cflags --libs opencv`






