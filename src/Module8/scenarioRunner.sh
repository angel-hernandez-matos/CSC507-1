#!/bin/bash

clear
declare -A scenarios
echo "Portfolio Project - Module 8 | Scenario Runner"
PROGRAM="./sumfiles"
RESULTS="scenarioRunnnerResults"

scenarios["SingleProgram"]="hugefile1.txt hugefile2.txt totalfile.txt full"
scenarios["TwoPrograms_FirstHalf"]="hugefile1.txt hugefile2.txt total_part1.txt range 1 500000000" 
scenarios["TwoPrograms_SecondHalf"]="hugefile1.txt hugefile2.txt total_part2.txt range 500000001 500000000"

mkdir -p "$RESULTS"


for key in "${!scenarios[@]}"; do
    args="${scenarios[$key]}"

    echo
    echo "Running scenario: $key"
    echo "Args: $args"

    (
        /usr/bin/time -f "%e" \
            $PROGRAM $args \
            > /dev/null \
            2> "$RESULTS/time_${key}.txt"
    ) &
done

wait

echo

for key in "${!scenarios[@]}"; do
    echo "Runtime of ${key}: $(cat "$RESULTS/time_${key}.txt") secs."
done


echo
echo "Running scenario: Break into 10 files each and run all 10 sets in parallel"

split --numeric-suffixes=1 -l 100000000 hugefile1.txt "$RESULTS/huge1_" &
split --numeric-suffixes=1 -l 100000000 hugefile2.txt "$RESULTS/huge2_" &

wait

count=$(ls "$RESULTS"/huge1_* | wc -l) #Need to know how many files were created 

id=""
chunks=()
for i in $(seq 1 "$count"); do
    chunks+=("$i")
    if [[ $i -lt 10 ]]; then
     id="0$i" 
    else 
     id="$i" 
    fi
    
    (
        /usr/bin/time -f "%e" \
            ./sumfiles "$RESULTS/huge1_$id" "$RESULTS/huge2_$id" "$RESULTS/total_$id.txt" chunk \
            > /dev/null \
            2> "$RESULTS/chunk_time_${i}.txt"
    ) &
done

wait


for key in "${chunks[@]}"; do
    echo "Runtime of chunk (${key}): $(cat "$RESULTS/chunk_time_${key}.txt") secs."
done

echo "Combining text files into one..."
cat "$RESULTS"/total_*.txt > "$RESULTS/totalfile_10procs.txt"

echo
echo "Running scenario: Using multiple threads"

 (
    /usr/bin/time -f "%e" \
    ./sumfiles hugefile1.txt hugefile2.txt "$RESULTS"/threaded_total.txt threaded 10 \
    > /dev/null \
    2> "$RESULTS/threaded_time.txt"
) &

wait

echo "Runtime of multiple threads: $(cat "$RESULTS/threaded_time.txt") secs."


echo "All scenarios finished."
