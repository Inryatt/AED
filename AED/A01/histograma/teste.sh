#!/bin/bash

######################################################
#
#   $1 -> nmec
#   $2 -> profts
#   $3 -> tempo que queres esperar
#
################################333333333333333333333

mkdir -p $(printf "%06d" $1)
for t in {1..64};do
  for p in {1..8};do
    if [[ !(-n  $( cat $(printf "%06d/%02d_%02d_%d.txt" $1 $t $p $2)) && -n $(grep "End" $(printf "%06d/%02d_%02d_%d.txt" $1 $t $p $2)) ) ]] && [[ $p -le $t ]];then
       ./histograma $1 $t $p $2 > /dev/null
      if [[ $? == 124 ]] ;then echo "Cant Take it anymore";  exit -1;fi
    fi
  done
done
