#!/bin/bash

g++ -o main sharpeningFilter.cpp main.cpp -lOpenCL -lm `pkg-config --cflags --libs opencv`






