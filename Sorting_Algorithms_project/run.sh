#!/usr/bin/bash
#
# if this script is not executable then do
#     chmod +x run.sh
#
# this script assumes that geninput.c and lab3 are in current directory
# and there is a tests directory with additional input files
#
# MP3 place in student directory
if [ ! -f ./geninput ]; then
    if [ -f ./geninput.c ]; then
        gcc geninput.c -o geninput
    else
        echo "geninput.c file not found! Check why not in repository!"
        exit 1
    fi
fi
seed=09202024
./geninput 20 1 1 $seed | ./lab3 > gradingout_t11_insertion
./geninput 20 2 1 $seed | ./lab3 > gradingout_t12_insertion
./geninput 20 3 1 $seed | ./lab3 > gradingout_t13_insertion
./geninput 20 1 2 $seed | ./lab3 > gradingout_t21_recsel
./geninput 20 2 2 $seed | ./lab3 > gradingout_t22_recsel
./geninput 20 3 2 $seed | ./lab3 > gradingout_t23_recsel
./geninput 20 1 3 $seed | ./lab3 > gradingout_t31_itersel
./geninput 20 2 3 $seed | ./lab3 > gradingout_t32_itersel
./geninput 20 3 3 $seed | ./lab3 > gradingout_t33_itersel
./geninput 20 1 4 $seed | ./lab3 > gradingout_t41_merge
./geninput 20 2 4 $seed | ./lab3 > gradingout_t42_merge
./geninput 20 3 4 $seed | ./lab3 > gradingout_t43_merge
./geninput 20 1 5 $seed | ./lab3 > gradingout_t51_qsort
./geninput 20 2 5 $seed | ./lab3 > gradingout_t52_qsort
./geninput 20 3 5 $seed | ./lab3 > gradingout_t53_qsort
./lab3 < ./tests/t01insert  > gradingout_t01insert
./lab3 < ./tests/t02recsel > gradingout_t02recsel
./lab3 < ./tests/t03itersel > gradingout_t03itersel
./lab3 < ./tests/t04merge > gradingout_t04merge
./lab3 < ./tests/t05qsort > gradingout_t05qsort

