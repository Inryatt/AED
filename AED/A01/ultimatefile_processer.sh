#!/bin/bash

##################
#
# file processor script
#
#    has to be in the same directory as the files to process
#   $1 for name of the directory only
#   other argumentes will be ignored
##################

if [[ ! -d $1 ]];then  echo "BadDirectory" ; exit -1 ;fi
[[ -d "files$1/" ]] && /usr/bin/rm "files$1/*" || mkdir files$1 # avoid rm aliases

files=( $(grep "End" $1/*.txt | awk -F "/|:" '{print $2}') )

echo ${#files[@]}
for f in ${files[@]};do
  number_of_tasks=$(awk -F "_" '{printf $1 "\n"}' <<< $f | sed "s/^0//g")
  number_of_programmers=$(awk -F "_" '{printf $2 "\n"}' <<< $f | sed "s/^0//g")
  profit_is_considered=$(awk -F "_" '{printf $3}' <<< $f )

  profit_filename=$(printf "files$1/profits_%02d_%02d_%s" $number_of_tasks $number_of_programmers "$profit_is_considered")
  filename_for_profits=$(printf "files$1/profits%s" "$profit_is_considered")

  task_filename=$(printf "files$1/tasks_%02d_%02d_%s" $number_of_tasks $number_of_programmers "$profit_is_considered")

  execution_filename=$(printf "files$1/Execution_Times%s" "$profit_is_considered")

  count=0
  for p in $(seq -1 $(( $number_of_programmers - 1)) );do
    [[ $p != -1 ]] && printf "P$p " >> $task_filename
    head -n $number_of_tasks $1/$f | awk -v p="P"$p -F "\t" '$0~p{if(p == "P-1") {printf "P-1 " $2 "\n" } else {printf $2 " "} }' >> $task_filename
    [[ $p != -1 ]] && printf "\n" >> $task_filename
  done;
  count=$(grep "P-1" $task_filename | wc -l)
  head -$count $task_filename | sed "s/^P-1 //g;s/\\t/ /g;s/  / /g" >> $task_filename
  sed -i 1,"$count"d $task_filename
  grep "^[[:digit:][:digit:]]" $1/$f | sort -nk 1 | awk '{ printf $4"\n"}'  > $profit_filename
  profit=$( printf "%d %d %d" $number_of_tasks $number_of_programmers "$(grep "Profit = " $1/$f | awk '{printf $3"\n"}')")
  echo "$profit" >> $filename_for_profits
  time_exec=$( printf "%d %d %s" $number_of_tasks $number_of_programmers "$(grep "Solution" $1/$f | awk '{printf $4"\n"}')")
  echo $time_exec >> $execution_filename
done
