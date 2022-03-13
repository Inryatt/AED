files_to_process=( $(ls 097827/Tasks*) )
rm -f "097827/*txtf*"
for file in ${files_to_process[@]};do
  NumberofTasks=$(echo $file | awk -F "_" '{printf $1 "\n"}' | cut -c 13-)
  NumberofProgrammers=$(echo $file | awk -F "_" '{printf $2 "\n"}' | cut -c 1-2)
  newFileName=$(printf "%sformated" $file)
  for p in $(seq 0 $NumberofProgrammers);do
    printf "P$p " >> $newFileName
    cat $file  | head -n $NumberofTasks | awk -v p="P"$p -F "\t" '$0~p{printf $2 " "}' >> $newFileName
   printf "\n" >> $newFileName
  done;
  cat $newFileName | head -$(( $(wc -l < $newFileName) - 1 )) > $newFileName
done;
