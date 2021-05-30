#!/bin/bash

make
printf "" > testoutputs.txt

total=0
passed=0

launch(){
    echo "File: $1" | tee -a testoutputs.txt
    let total++
    error=`./bin/tpcc -n < $1 2>&1`
    if [ $? = 0 ]; then
        echo "--> Valid syntax" | tee -a testoutputs.txt
        if [ $2 = 0 ]; then
            let passed++
        fi
    elif [ $? = 1 ]; then
        printf "$error\n" | tee -a testoutputs.txt
        echo "--> Invalid syntax" | tee -a testoutputs.txt
        if [ $2 = 1 ]; then
            let passed++
        fi
    else
        echo "--> Other error" | tee -a testoutputs.txt
        if [ $2 != 0 && $2 != 1 ]; then
            let passed++
        fi
    fi
}

echo
echo "##########Beginning tests##########" | tee -a testoutputs.txt
echo "======Valid programs======" | tee -a testoutputs.txt
for file in test/good/*.tpc; do
    launch $file 0
done
echo | tee -a testoutputs.txt
echo "======Syntax error programs======" | tee -a testoutputs.txt
for file in test/syn-err/*.tpc; do
    launch $file 1
done
echo "##########Ending tests##########" | tee -a testoutputs.txt
echo | tee -a testoutputs.txt

if [ $total = $passed ]; then
    printf "\033[0;32m"
else
    printf "\033[0;31m"
fi
echo -e "$passed out of $total test(s) passed!" | tee -a testoutputs.txt
printf "\033[0m"

make clean
echo -e "\033[0;32mDone! Complete test reports in testoutputs.txt\033[0m"