
export files_to_process=( $(grep "^End" 097827/* | awk -F ":" '{printf $1"\n"}') )
for file in "${files_to_process[@]}";do
  fileNameTimes=$(printf "097827/Times.txt\n" )
  T=$(awk '/^T =/ {printf $3}' $file)
  P=$(awk '/^P =/ {printf $3}' $file)
  Profit=0 # replace ignonred with 0
  Time=$(awk '/Solution/ {printf $4}' $file)
  printf "%d %d %s\n" $T $P $Time >> "$fileNameTimes" || echo "FAlHou Cause you suck"
  fileNameTasks=$(printf "097827/Tasks%02d_%02d.txt\n" $T $P )
  cat $file | head -n $T > "$fileNameTasks"
done;
echo "check for yourself fatso"
cat "097827/Times.txt"
