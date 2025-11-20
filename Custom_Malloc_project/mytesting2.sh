#!/usr/bin/bash
#
# this script tests provided drivers.  
#
# The script runs in less than 10 seconds on an apollo machine
#
#  only the trial "./lab4 -e" takes more than 1 second.
#
# if this script is not executable then do
#     chmod +x longrun.sh
#
# MP4 place in student directory
echo "this script should run in less than 10 seconds"
seed=10172024
echo "unit tests"
./lab4 -u0 -s $seed
./lab4 -u0 -c -s $seed
./lab4 -u1 -s $seed
./lab4 -u1 -f best -s $seed
./lab4 -u1 -c -s $seed
./lab4 -u1 -c -f best -s $seed
./lab4 -u2 -s $seed
./lab4 -u2 -f best -s $seed
./lab4 -u2 -c -s $seed
./lab4 -u2 -c -f best -s $seed
