#!/bin/bash
echo "Hello"
sum=0
for file in log/*; do
	let sum=sum+$(fgrep -o ":search:" $file | wc -l)
done
echo "Total number of keywords searched=${sum}"
