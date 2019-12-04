/*
  Simple program that allocates a block of memory and writes
  data to it in sequential order.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "commonlib.h"

#define NUM_EXPECTED_ARGS 4 

static void usage(void)
{
    printf("Usage: ./simple_sequential <size> <offset> <mmap function>\n");
    printf("\t<size>         : map size in bytes, must be even\n");
    printf("\t<offset>       : must be positive\n");
    printf("\t<mmap function>: 0 for system, 1 for custom driver\n");
}

int main(int argc, char ** argv)
{
    int i = 0;
    int ret;
	unsigned int step = 0;
    unsigned long int map_size;
    int use_mmap_driver = 0;
    int *address = NULL;
    int *orig_addr = NULL;
    unsigned long int offset;
    struct timespec start_time;
    struct timespec end_time;

    if (argc != NUM_EXPECTED_ARGS)
    {
      printf("Incorrect number of arguments\n");
      usage();
      exit(EXIT_FAILURE);
    }

    sscanf(argv[1], "%lu", &map_size);
    sscanf(argv[2], "%lu", &offset);
    use_mmap_driver = atoi(argv[3]);
    
    if (map_size % 2 != 0)
    {
        printf("Size must be even\n");
        usage();
        exit(EXIT_FAILURE);
    }
    
    if (offset < 0)
    {
        printf("Offset must be greater than 0\n");
        usage();
        exit(EXIT_FAILURE);
    }

    get_time(&start_time);
    
    orig_addr = get_mmap_addr(map_size, use_mmap_driver); // Keep the original address so we can unmap later
    address = orig_addr;
    
    
    if (address == MAP_FAILED)
    {
        exit(EXIT_FAILURE);
    }    

    while((offset * step) < map_size) 
    {
		*address = step;
		//printf("%p=%d\n", address, *address);
		address += offset/sizeof(int);
		step++;
	}
    
    munmap(orig_addr, map_size);
    get_time(&end_time);
    printf("Execution time: %.9f secs\n", timediff(start_time, end_time));
    
    return ret;   
}
