#!/bin/bash

g++ -o main laplacianFilter.cpp main.cpp -lOpenCL -lm `pkg-config --cflags --libs opencv`






