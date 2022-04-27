#!/bin/bash
clear && g++ -g -std=c++2a src/main.cpp src/tykadlaHelpers.cpp src/propagatorRoot.cpp src/propagatorField.cpp -lgecodesupport -lgecodekernel -lgecodesearch -lgecodeint -lgecodedriver -lgecodeminimodel -lgecodegist -o ../../bin/2022_1kolo_8uloha_tykadla && ../../bin/2022_1kolo_8uloha_tykadla
