# File: numbers.sh
# Written by: Angel Hernandez
# Description: Module 3 - Portfolio Milestone
# Requirement(s): Random generation of 1,000,000 numbers

clear

echo "Portfolio Milestone 3 - Start generation of random numbers - Bash implementation..."

# Remove old file if it exists
rm -f file1.txt 

# Record start time
SECONDS=0
start_time=$(date +"%Y-%m-%d %H:%M:%S")
echo "System time before process: $start_time"

# Generate 1,000,000 random numbers
for i in {1..1000000}
do
  echo $RANDOM >> file1.txt     # Append random number to file 
done

# Record end time
end_time=$(date +"%Y-%m-%d %H:%M:%S")
echo "System time after process: $end_time"

# Calculate elapsed time in seconds
elapsed=$SECONDS

# Convert elapsed seconds into hours/minutes/seconds
hours=$((elapsed / 3600))
minutes=$(((elapsed % 3600) / 60))
seconds=$((elapsed % 60))

echo "Random number generator is complete."
time_taken="Time taken: ${hours}h ${minutes}m ${seconds}s"
echo $time_taken

# Create results directory and write stats
mkdir -p results
{
  echo "Date: $(date +"%Y-%m-%d %H:%M:%S")"
  echo "Line count: $(wc -l < file1.txt)"
  echo "Word count: $(wc -w < file1.txt)"
  echo $time_taken  
  echo -e "\n"
  #echo -e "$(time_taken)\n"  
} >> results/wc_bash.txt


echo "Word/line count and execution time written to results/wc_bash.txt"

