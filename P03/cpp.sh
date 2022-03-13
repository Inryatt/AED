#!/bin/bash
name=$1+"EX" 
c++ -Wall -O2 $1 -o $name -lm 
