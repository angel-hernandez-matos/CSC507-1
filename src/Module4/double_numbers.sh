# File: double_numbers.sh
# Written by: Angel Hernandez
# Description: Module 4 - Portfolio Milestone
# Requirement(s): Random generation of 1,000,000 numbers plus the following:
#                 Read each line of file1.txt, storing the number from each line into a variable, then write a value that is double the original number, 
#                 into another file called newfile1.txt. Once this is done for the entire contents of file1.txt, display the time it took to run.

clear

echo "Portfolio Milestone 4 - Start generation of random numbers (double numbers)- Bash implementation..."

# Remove old output file if it exists
rm -f newfile1.txt

# Record start time
SECONDS=0
start_time=$(date +"%Y-%m-%d %H:%M:%S")
echo "System time before process: $start_time"

# Read each line from file1.txt, double the number, and write to newfile1.txt
while read -r number
do
  doubled=$((number * 2))
  echo $doubled >> newfile1.txt
done < file1.txt

# Record end time
end_time=$(date +"%Y-%m-%d %H:%M:%S")
echo "System time after process: $end_time"

# Calculate elapsed time in seconds
elapsed=$SECONDS

# Convert elapsed seconds into hours/minutes/seconds
hours=$((elapsed / 3600))
minutes=$(((elapsed % 3600) / 60))
seconds=$((elapsed % 60))

echo "Doubling process is complete."
time_taken="Time taken: ${hours}h ${minutes}m ${seconds}s"
echo $time_taken

# Create results directory and write stats
mkdir -p results
{
  echo "Date: $(date +"%Y-%m-%d %H:%M:%S")"
  echo "Line count: $(wc -l < newfile1.txt)"
  echo "Word count: $(wc -w < newfile1.txt)"
  echo $time_taken
  echo -e "\n"
} >> results/wc_double_bash.txt

echo "Word/line count and execution time written to results/wc_double_bash.txt"
