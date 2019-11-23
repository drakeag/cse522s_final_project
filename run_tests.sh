#!/bin/bash

RESULTS_FILE=results.txt
PERF_EVENTS=cache-references,cache-misses,page-faults,dTLB-load-misses
TEST_NUM=1
SYS_MMAP=0
DRIVER_MMAP=1

echo Compiling test programs...
cd tests
make clean
make

echo Running tests...
echo Test Results > $RESULTS_FILE
echo >> $RESULTS_FILE

echo Test $TEST_NUM >> $RESULTS_FILE
echo sudo ./perf stat -e $PERF_EVENTS ./random_access 2 $SYS_MMAP >> $RESULTS_FILE
# 2> is the output from the program, 1> is the output from perf
sudo ./perf stat -e $PERF_EVENTS ./random_access 2 $SYS_MMAP 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

echo Test $TEST_NUM >> $RESULTS_FILE
echo sudo ./perf stat -e $PERF_EVENTS ./dense_mm 20 $SYS_MMAP >> $RESULTS_FILE
sudo ./perf stat -e $PERF_EVENTS ./dense_mm 20 $SYS_MMAP 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

echo Tests complete


