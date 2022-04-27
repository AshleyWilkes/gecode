#!/bin/bash
clear && g++ -g -std=c++2a src/main.cpp -lgecodesupport -lgecodekernel -lgecodesearch -lgecodeint -lgecodedriver -lgecodeminimodel -lgecodegist -o ../../bin/2022_1kolo_6uloha_krive_sudoku && time ../../bin/2022_1kolo_6uloha_krive_sudoku
