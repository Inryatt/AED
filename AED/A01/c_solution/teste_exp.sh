#!/bin/bash

######################################################
#
#   $1 -> nmec
#   $2 -> profts
#   $3 -> tempo que queres esperar
#
################################333333333333333333333

mkdir -p $(printf "t%06d" $1)
for t in {1..64};do
  for p in {1..10};do
    if [[ !(-n  $( cat $(printf "t%06d_%02d_%02d_%d.txt" $1 $t $p $3)) && -n $(grep "End" $(printf "t%06d/%02d_%02d_%d.txt" $1 $t $p $3)) ) ]];then
      echo "timeout $3 ./export $1 $t $p $2" 
      timeout $3 ./export $1 $t $p $2  > /dev/null
      if [[ $? == 124 ]] ;then echo "Cant Take it anymore";  exit -1;fi
    fi
  done
done
