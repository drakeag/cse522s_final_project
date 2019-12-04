#!/bin/bash

if [ "$1" != "" ]; then
	RESULTS_FILE=$1
else
	RESULTS_FILE=results.txt
fi
PERF_EVENTS=cache-references,cache-misses,page-faults,dTLB-load-misses,context-switches
TEST_NUM=1
SYS_MMAP=0
DRIVER_MMAP=1

load_mmu_module() {
	sudo insmod ../mmu_module.ko
}

unload_mmu_module() {
	sudo rmmod mmu_module
}

do_test() {
	# $1 TEST_NUM
	# $2 CMD
	echo "Test $1"
	echo "Test $1" >> $RESULTS_FILE
	# Write the command to the results file
	echo $2 >> $RESULTS_FILE
	
	if [ "$3" == $DRIVER_MMAP ]; then
		# Load the module
		load_mmu_module
	fi
	
	# Run the command and write the output to the results file
	# 2> is the output from the program, 1> is the output from perf
	$2 2>>$RESULTS_FILE 1>>$RESULTS_FILE
	echo >> $RESULTS_FILE
	
	if [ "$3" == $DRIVER_MMAP ]; then
		# Unload the module
		unload_mmu_module
	fi
}

#echo Compiling module...
#make clean
#make

echo Compiling test programs...
cd tests
make clean
make

echo Running tests...
echo Test Results > $RESULTS_FILE
echo >> $RESULTS_FILE

# 1
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./simple_sequential 4096 4096 ${SYS_MMAP}"
((++TEST_NUM))

# 2
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./simple_sequential 4096 4096 ${DRIVER_MMAP}" $DRIVER_MMAP
((++TEST_NUM))

# 3
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./simple_sequential 65536 4096 ${SYS_MMAP}" 
((++TEST_NUM))

# 4
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./simple_sequential 65536 4096 ${DRIVER_MMAP}" $DRIVER_MMAP
((++TEST_NUM))

# 5
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./simple_sequential 1048576 4096 ${SYS_MMAP}"
((++TEST_NUM))

# 6
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./simple_sequential 1048576 4096 ${DRIVER_MMAP}" $DRIVER_MMAP
((++TEST_NUM))

# 7
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./simple_sequential 65536 65536 ${SYS_MMAP}"
((++TEST_NUM))

# 8
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./simple_sequential 65536 65536 ${DRIVER_MMAP}" $DRIVER_MMAP
((++TEST_NUM))

# 9
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./simple_sequential 1048576 65536 ${SYS_MMAP}"
((++TEST_NUM))

# 10
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./simple_sequential 1048576 65536 ${DRIVER_MMAP}" $DRIVER_MMAP
((++TEST_NUM))

SWITCH_PERCENT=0.05
# 11
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./context_switching 65536 4096 ${SWITCH_PERCENT} ${SYS_MMAP}" 
((++TEST_NUM))

# 12
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./context_switching 65536 4096 ${SWITCH_PERCENT} ${DRIVER_MMAP}" $DRIVER_MMAP
((++TEST_NUM))

# 13
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./context_switching 1048576 4096 ${SWITCH_PERCENT} ${SYS_MMAP}"
((++TEST_NUM))

# 14
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./context_switching 1048576 4096 ${SWITCH_PERCENT} ${DRIVER_MMAP}" $DRIVER_MMAP
((++TEST_NUM))

SWITCH_PERCENT=0.10
# 15
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./context_switching 65536 4096 ${SWITCH_PERCENT} ${SYS_MMAP}" 
((++TEST_NUM))

# 16
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./context_switching 65536 4096 ${SWITCH_PERCENT} ${DRIVER_MMAP}" $DRIVER_MMAP
((++TEST_NUM))

# 17
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./context_switching 1048576 4096 ${SWITCH_PERCENT} ${SYS_MMAP}"
((++TEST_NUM))

# 18
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./context_switching 1048576 4096 ${SWITCH_PERCENT} ${DRIVER_MMAP}" $DRIVER_MMAP
((++TEST_NUM))

SWITCH_PERCENT=0.20
# 19
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./context_switching 65536 4096 ${SWITCH_PERCENT} ${SYS_MMAP}" 
((++TEST_NUM))

# 20
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./context_switching 65536 4096 ${SWITCH_PERCENT} ${DRIVER_MMAP}" $DRIVER_MMAP
((++TEST_NUM))

# 21
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./context_switching 1048576 4096 ${SWITCH_PERCENT} ${SYS_MMAP}"
((++TEST_NUM))

# 22
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./context_switching 1048576 4096 ${SWITCH_PERCENT} ${DRIVER_MMAP}" $DRIVER_MMAP
((++TEST_NUM))

echo Tests complete


