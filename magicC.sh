#!/bin/bash


cc -Wall -O2 $1 -o ${1%%.*} -lm 
