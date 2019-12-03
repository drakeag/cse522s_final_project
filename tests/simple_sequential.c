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
    
    map = get_mmap_addr(map_size, use_mmap_driver);
    
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
