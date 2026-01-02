#!/bin/bash
for i in {1..1000}
do
  cat file1.txt >> hugefile1.txt
  cat file2.txt >> hugefile2.txt
done
