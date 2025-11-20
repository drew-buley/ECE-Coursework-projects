ECE 2230 MP3 test directory
Fall 2024

To run all the test input files in the tests directory do:
    ./run.sh
    ./check.pl

To repeat the tests but with valgrind do:
    ./valrun.sh
    ./check.pl

To check for run times do:
    ./longrun.sh

To collect data for performance evaluation do:
    ./mp3tests.sh > mydata.csv

To clean up between tests do:
   make clean

If any of the scripts have permission errors and do not run then do:
    chmod +x filename


