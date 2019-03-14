#!/bin/bash

g++ -o main gaussianFilter.cpp main.cpp -lOpenCL -lm `pkg-config --cflags --libs opencv`






