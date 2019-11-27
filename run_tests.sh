#!/bin/bash

RESULTS_FILE=results.txt
PERF_EVENTS=cache-references,cache-misses,page-faults,dTLB-load-misses,context-switches
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

TEST="Test ${TEST_NUM}"
echo $TEST
echo $TEST >> $RESULTS_FILE
CMD="sudo ./perf stat -e ${PERF_EVENTS} ./simple_sequential 4096 ${SYS_MMAP}"
# Write the command to the results file
echo $CMD >> $RESULTS_FILE
# Run the command and write the output to the results file
# 2> is the output from the program, 1> is the output from perf
$CMD 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

TEST="Test ${TEST_NUM}"
echo $TEST
echo $TEST >> $RESULTS_FILE
CMD="sudo ./perf stat -e ${PERF_EVENTS} ./simple_sequential 65536 ${SYS_MMAP}"
# Write the command to the results file
echo $CMD >> $RESULTS_FILE
# Run the command and write the output to the results file
# 2> is the output from the program, 1> is the output from perf
$CMD 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

TEST="Test ${TEST_NUM}"
echo $TEST
echo $TEST >> $RESULTS_FILE
CMD="sudo ./perf stat -e ${PERF_EVENTS} ./simple_sequential 1048576 ${SYS_MMAP}"
# Write the command to the results file
echo $CMD >> $RESULTS_FILE
# Run the command and write the output to the results file
# 2> is the output from the program, 1> is the output from perf
$CMD 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

TEST="Test ${TEST_NUM}"
echo $TEST
echo $TEST >> $RESULTS_FILE
CMD="sudo ./perf stat -e ${PERF_EVENTS} ./simple_sequential 16777216 ${SYS_MMAP}"
# Write the command to the results file
echo $CMD >> $RESULTS_FILE
# Run the command and write the output to the results file
# 2> is the output from the program, 1> is the output from perf
$CMD 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

TEST="Test ${TEST_NUM}"
echo $TEST
echo $TEST >> $RESULTS_FILE
CMD="sudo ./perf stat -e ${PERF_EVENTS} ./random_access 2 ${SYS_MMAP}"
# Write the command to the results file
echo $CMD >> $RESULTS_FILE
# Run the command and write the output to the results file
# 2> is the output from the program, 1> is the output from perf
$CMD 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

TEST="Test ${TEST_NUM}"
echo $TEST
echo $TEST >> $RESULTS_FILE
CMD="sudo ./perf stat -e ${PERF_EVENTS} ./random_access 4 ${SYS_MMAP}"
# Write the command to the results file
echo $CMD >> $RESULTS_FILE
# Run the command and write the output to the results file
# 2> is the output from the program, 1> is the output from perf
$CMD 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

TEST="Test ${TEST_NUM}"
echo $TEST
echo $TEST >> $RESULTS_FILE
CMD="sudo ./perf stat -e ${PERF_EVENTS} ./random_access 8 ${SYS_MMAP}"
# Write the command to the results file
echo $CMD >> $RESULTS_FILE
# Run the command and write the output to the results file
# 2> is the output from the program, 1> is the output from perf
$CMD 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

TEST="Test ${TEST_NUM}"
echo $TEST
echo $TEST >> $RESULTS_FILE
CMD="sudo ./perf stat -e ${PERF_EVENTS} ./random_access 16 ${SYS_MMAP}"
# Write the command to the results file
echo $CMD >> $RESULTS_FILE
# Run the command and write the output to the results file
# 2> is the output from the program, 1> is the output from perf
$CMD 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

TEST="Test ${TEST_NUM}"
echo $TEST
echo $TEST >> $RESULTS_FILE
CMD="sudo ./perf stat -e ${PERF_EVENTS} ./dense_mm 64 ${SYS_MMAP}"
# Write the command to the results file
echo $CMD >> $RESULTS_FILE
# Run the command and write the output to the results file
# 2> is the output from the program, 1> is the output from perf
$CMD 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

TEST="Test ${TEST_NUM}"
echo $TEST
echo $TEST >> $RESULTS_FILE
CMD="sudo ./perf stat -e ${PERF_EVENTS} ./dense_mm 512 ${SYS_MMAP}"
# Write the command to the results file
echo $CMD >> $RESULTS_FILE
# Run the command and write the output to the results file
# 2> is the output from the program, 1> is the output from perf
$CMD 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

TEST="Test ${TEST_NUM}"
echo $TEST
echo $TEST >> $RESULTS_FILE
CMD="sudo ./perf stat -e ${PERF_EVENTS} ./dense_mm 1024 ${SYS_MMAP}"
# Write the command to the results file
echo $CMD >> $RESULTS_FILE
# Run the command and write the output to the results file
# 2> is the output from the program, 1> is the output from perf
$CMD 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

TEST="Test ${TEST_NUM}"
echo $TEST
echo $TEST >> $RESULTS_FILE
CMD="sudo ./perf stat -e ${PERF_EVENTS} ./context_switching 1024 1 ${SYS_MMAP}"
# Write the command to the results file
echo $CMD >> $RESULTS_FILE
# Run the command and write the output to the results file
# 2> is the output from the program, 1> is the output from perf
$CMD 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

TEST="Test ${TEST_NUM}"
echo $TEST
echo $TEST >> $RESULTS_FILE
CMD="sudo ./perf stat -e ${PERF_EVENTS} ./context_switching 4096 1 ${SYS_MMAP}"
# Write the command to the results file
echo $CMD >> $RESULTS_FILE
# Run the command and write the output to the results file
# 2> is the output from the program, 1> is the output from perf
$CMD 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

TEST="Test ${TEST_NUM}"
echo $TEST
echo $TEST >> $RESULTS_FILE
CMD="sudo ./perf stat -e ${PERF_EVENTS} ./context_switching 32768 1024 ${SYS_MMAP}"
# Write the command to the results file
echo $CMD >> $RESULTS_FILE
# Run the command and write the output to the results file
# 2> is the output from the program, 1> is the output from perf
$CMD 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

TEST="Test ${TEST_NUM}"
echo $TEST
echo $TEST >> $RESULTS_FILE
CMD="sudo ./perf stat -e ${PERF_EVENTS} ./context_switching 16777216 32768 ${SYS_MMAP}"
# Write the command to the results file
echo $CMD >> $RESULTS_FILE
# Run the command and write the output to the results file
# 2> is the output from the program, 1> is the output from perf
$CMD 2>>$RESULTS_FILE 1>>$RESULTS_FILE
((++TEST_NUM))
echo >> $RESULTS_FILE

echo Tests complete


