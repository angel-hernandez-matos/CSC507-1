clear

echo "Starting generating random numbers..."
rm -f file1.txt 

start=$(date +%s%N)

for i in {1..1000}
do
  # echo $RANDOM > file1.txt      # Write random number
  echo $RANDOM >> file1.txt     # Append another random number 
done


end=$(date +%s%N)
elapsed=$(((end - start)/1000000))

echo "Random number generator is complete. Time taken: $elapsed ms"

mkdir -p results
{
  echo "Date: $(date +"%Y-%m-%d %H:%M:%S")"
  echo "Line count: $(wc -l < file1.txt)"
  echo -e "Word count: $(wc -w < file1.txt)\n"
} >> results/wc_bash.txt

echo "Word/line count written to results/wc_bash.txt"
