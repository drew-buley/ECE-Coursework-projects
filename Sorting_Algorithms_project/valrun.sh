#!/usr/bin/sh
#
# if this script is not executable then do
#     chmod +x valrun.sh
#
# MP3 place in student directory
valgrind --leak-check=yes ./lab3 < ./tests/t01insert > gradingout_t01insert 2> gradingout_valt01insert
valgrind --leak-check=yes ./lab3 < ./tests/t02recsel > gradingout_t02recsel 2> gradingout_valt02recsel
valgrind --leak-check=yes ./lab3 < ./tests/t03itersel > gradingout_t03itersel 2> gradingout_valt03itersel
valgrind --leak-check=yes ./lab3 < ./tests/t04merge > gradingout_t04merge 2> gradingout_valt04merge
valgrind --leak-check=yes ./lab3 < ./tests/t05qsort > gradingout_t05qsort 2> gradingout_valt05qsort

