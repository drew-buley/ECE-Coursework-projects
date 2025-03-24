#!/usr/bin/sh
# Drew Buley
# C20407096
# MP6
#
# Purpose: Simple testing script to test edge cases in MP6 code 
#          place in directory to run, use diff or meld on expected output with
#          myoutput (the result of this script) to check if pass
#
# Notes: Test 7 involves a custom unit driver I wrote to test a few additional test cases directly

# Test 1: test to make sure standard construct, insertion, retrieve, and destruct work
#         Expected output:
echo "BEGINNING TEST 1"
./lab6 -p 1 -v -h linear > myoutput
echo "TEST 1 logged to myoutput"

# Test 2: test additional probing styles for basic functionality 
#         Expected output:
echo "BEGNNING TEST 2"
./lab6 -r -t10 -v -m 6 -h linear >> myoutput
./lab6 -r -t10 -v -m 7 -h double >> myoutput
./lab6 -r -t10 -v -m 8 -h quad >> myoutput
echo "TEST 2 logged to myoutput"

# Test 3: testing deletions in several specific edge cases including:
#         insert dupe after initial probe was a deleted cell
#         insert dupe after repeadedly encountering deleted cells then an empty cell
#         insert into table that has only deleted and valid keys, no empty cells
#         also tests inserting dupe into full table with no empty cells
#         Expected output: 
echo "BEGINNING TEST 3"
./lab6 -d >> myoutput
./lab6 -d -h double >> myoutput
echo "TEST 3 logged to myoutput"

# Test 4: testing invalid table sizes in two sum to ensure proper table size selections being made
#         Expected output:
echo "BEGINNING TEST 4"
./lab6 -p4 -m 50 -t 20 -h double >> myoutput
./lab6 -p4 -m 40 -t 30 -h quad >> myoutput
echo "TEST 4 logged to myoutput"

# Test 5: testing rehash functionality in addition to more combinations of
#         dupes, empty availability, and deleted cell frequency
#         checks to see that rehashing works with all probing types
#         Expected output:
echo "BEGINNING TEST 5"
./lab6 -b -v > myoutput
./lab6 -b -v -m 13 -h linear >> myoutput
./lab6 -b -v -m 11 -h double >> myoutput
./lab6 -b -v -m 16 -h quad >> myoutput
echo "TEST 5 logged to myoutput"

# Test 6: testing memory lossage for insertions, deletions, and reshashing
#         Expected output: Valgrind states no memory lost for each execution
echo "BEGINNING TEST 6"
valgrind --leak-check=full ./lab6 -r -t20 -m60 -h linear >> myoutput
valgrind --leak-check=full ./lab6 -d -h double >> myoutput
valgrind --leak-check=full ./lab6 -b -m 64 -h quad >> myoutput
echo "TEST 6 logged to myoutput"

# Test 7: This tests for deleting a key not in table, deleting from empty table
#         and inserting into a full table (with both last cell as empty and as deleted)
#         Expected Output:
echo "BEGINNING TEST 7"
./lab6 -q >> myoutput
echo "TEST 7 logged to myoutput"
