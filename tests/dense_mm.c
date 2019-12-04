/******************************************************************************
* 
* dense_mm.c
* 
* This program implements a dense matrix multiply and can be used as a
* hypothetical workload. 
*
* Usage: This program takes a single input describing the size of the matrices
*        to multiply. For an input of size N, it computes A*B = C where each
*        of A, B, and C are matrices of size N*N. Matrices A and B are filled
*        with random values. 
*
* Written Sept 6, 2015 by David Ferry
******************************************************************************/

#include <stdio.h>  //For printf()
#include <stdlib.h> //for exit() and atoi()
#include <sys/mman.h>
#include "commonlib.h"

const int num_expected_args = 3;
const unsigned sqrt_of_UINT32_MAX = 65536;

int main( int argc, char* argv[] ){

	int use_mmap_driver = 0;
	unsigned index, row, col; //loop indicies
	unsigned matrix_size, squared_size;
	int length;
	double *A, *B, *C;
	struct timespec start_time;
    struct timespec end_time;

	if( argc != num_expected_args ){
		printf("Usage: ./dense_mm <size of matrices> <mmap function> (0 for system, 1 for custom driver)\n");
		exit(-1);
	}

	matrix_size = atoi(argv[1]);
	use_mmap_driver = atoi(argv[2]);
	
	if( matrix_size > sqrt_of_UINT32_MAX ){
		printf("ERROR: Matrix size must be between zero and 65536!\n");
		exit(-1);
	}

	squared_size = matrix_size * matrix_size;
	length = sizeof(double) * squared_size;

	//printf("Generating matrices...\n");
	
	get_time(&start_time);
	printf("length: %d\n", length);
	A = (double*)get_mmap_addr(length, use_mmap_driver);
	printf("A: %p\n", A);
	B = (double*)get_mmap_addr(length, use_mmap_driver);
	C = (double*)get_mmap_addr(length, use_mmap_driver);

	for( index = 0; index < squared_size; index++ ){
		A[index] = (double) rand();
		B[index] = (double) rand();
		C[index] = 0.0;
	}

	//printf("Multiplying matrices...\n");

	for( row = 0; row < matrix_size; row++ ){
		for( col = 0; col < matrix_size; col++ ){
			for( index = 0; index < matrix_size; index++){
			C[row*matrix_size + col] += A[row*matrix_size + index] *B[index*matrix_size + col];
			}	
		}
	}
	printf("A: %p\n", A);
	//munmap(A, length);
	//munmap(B, length);
	//munmap(C, length);
	get_time(&end_time);


	printf("Exeuction time: %.9f secs\n", timediff(start_time, end_time));

	return 0;
}
