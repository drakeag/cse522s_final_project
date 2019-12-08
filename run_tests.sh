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



# 23
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./random_access 4096 1 ${SYS_MMAP}"
((++TEST_NUM))

# 24
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./random_access 4096 1 ${DRIVER_MMAP}" $DRIVER_MMAP
((++TEST_NUM))

# 25
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./random_access 65536 16 ${SYS_MMAP}" 
((++TEST_NUM))

# 26
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./random_access 65536 16 ${DRIVER_MMAP}" $DRIVER_MMAP
((++TEST_NUM))

# 27
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./random_access 1048576 256 ${SYS_MMAP}"
((++TEST_NUM))

# 28
do_test $TEST_NUM "sudo ./perf stat -e ${PERF_EVENTS} ./random_access 1048576 256 ${DRIVER_MMAP}" $DRIVER_MMAP
((++TEST_NUM))

echo Tests complete


