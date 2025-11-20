#!/usr/bin/perl
use strict;
use warnings;
# put in students directory and run to check against correct output file
#
# if this script is not executable then do
#     chmod +x check.pl 
#

# loop through each file and directory at current location

    # backtick executes a command and collects stdout 
    # stderr is not collected but goes to output

# @ denotes an array
my @alltest = qw(
        t11_insertion
        t12_insertion
        t13_insertion
        t21_recsel
        t22_recsel
        t23_recsel
        t31_itersel
        t32_itersel
        t33_itersel
        t41_merge
        t42_merge
        t43_merge
        t51_qsort
        t52_qsort
        t53_qsort
        t01insert
        t02recsel
        t03itersel
        t04merge
        t05qsort
);

my $passcount = 0;
foreach my $test (@alltest) {
    my $gradefile = 'gradingout_'.$test;
    my $valgrindfile = 'gradingout_val'.$test;
    my $answerfile = 'out_'.$test;

    my $found = 0;
    if (-f $gradefile) {
        if (-z $gradefile) {
                print "$test did not complete\n";
        } else {
            # deletes first line with time information
            `sed -e "1d" < $gradefile > tempsed123`;
            my $info = `diff -w tempsed123 ./tests/$answerfile`;
            if ($? > 0) {
                print "$test failed\n";
                #print "the info is: $info\n";
                $found = 1;
            } else {
                $passcount++;
            }
            `rm tempsed123`;
        }
    }
    if (-f $valgrindfile) {
        my $info = `grep "no leaks" $valgrindfile`;
        if ($info eq "") {
            if ($found == 0) { 
                    print "$test failed valgrind\n";
            }
            $info = `grep "total heap usag" $valgrindfile`;
            print "\t$test found leaks: $info";
            $found = 1;
        }
        $info = `grep "0 errors" $valgrindfile`;
        if ($info eq "") {
            if ($found == 0) { 
                    print "$test failed valgrind\n";
            }
            $info = `grep "ERROR SUMMARY" $valgrindfile`;
            print "\t$test $info\n";
            $found = 1;
        }
    }
    if ($found == 1) {
        print "\n";
    }
}
print "Passed $passcount tests\n";
if ($passcount == 0) {
    print "\n\tWARNING no tests passed.  Make sure that testing is working\n\n";
}

    #$info = `cd $user; grep "winning amount is" gradingoutput; cd ..`;
    #print "the info is: $info\n";

    # for example, if we want to see only the errors and
    # warnings we can ignore the stdout.  To see stdout:
    # print "the info is: $info\n";
    # print "the status is $?\n";

    # system executes a command but only gets final return value
    # all output just goes to stdout
    #
    # $make = "cd $user; make clean; make; cd ..";
    # $rc = 0xffff & system $make;
    # if ($rc != 0) {
    #     die "system $make failed: $?";
    # }


#    a more elaborate way to test return value
#    $rc = 0xffff & system $make;
#    if ($rc == 0) {
#        print "ran with normal exit\n";
#    }
#    elsif ($rc == 0xff00) {
#        print "command failed: $!\n";
#    }
#    elsif (($rc & 0xff) == 0) {
#        $rc >>= 8;
#        print "ran with non-zero exit status $rc\n";
#    }
#    else {
#        print "ran with ";
#        if ($rc & 0x80) {
#            $rc &= ~0x80;
#            print "coredump from ";
#        }
#        print "signal $rc\n";
#    }

# reg expression  (upper case negates the following)
#   \d [0-9]
#   \w [a-zA-Z0-9_]
#   \s [ \r\t\n\f]
#   * zero or more
#   + one or more
#   ? zero or one

# File tests
#     -x file or directory is executable
#     -e file or directory exists
#     -z file exists and has zero size
#     -s non zero size
#     -f a file
#     -d a directory
#
