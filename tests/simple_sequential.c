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
#include "timelib.h"

#define NUM_EXPECTED_ARGS 3

static void usage(void)
{
    printf("Usage: ./simple_sequential <map size (bytes)> (must be even) <mmap function> (0 for system, 1 for custom driver)\n");
}

int main(int argc, char ** argv)
{
    int ret;
    unsigned long map_size;
    int use_mmap_driver = 0;
    int num_entries;
    int *map; // mmaped array of ints
    struct timespec start_time;
    struct timespec end_time;

    if (argc != NUM_EXPECTED_ARGS)
    {
      usage();
      exit(EXIT_FAILURE);
    }

    map_size = atof(argv[1]);
    use_mmap_driver = atoi(argv[2]);
    num_entries = map_size / sizeof(int);

    if (map_size % 2 != 0) {
        usage();
        exit(EXIT_FAILURE);
    }

    get_time(&start_time);
    
    if (use_mmap_driver == 0)
    {
        map = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); 
    }
    else
    {
        // Setup mmap device driver call
        printf("mmap device driver not implemented yet\n");
        exit(EXIT_FAILURE);
    }
    
    if (map == MAP_FAILED)
    {
        exit(EXIT_FAILURE);
    }    

    for (int i=0; i<num_entries; i++)
    {
        map[i] = i*2;

    }

    munmap(map, map_size);
    get_time(&end_time);
    printf("Exeuction time: %.9f secs\n", timediff(start_time, end_time));
    
    return ret;    
}
