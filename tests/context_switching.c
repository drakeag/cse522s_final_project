/*
  Simple program that allocates a block of memory and writes
  data to it in sequential order while yielding the processor
  at specified iteration points to simulate different context
  switching patterns.
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
#define SLEEP_USEC 1

static void usec_sleep(long usec)
{
    struct timespec ts;
    
    if (usec< 0)
    {
        printf("Sleep time cannot be less than 0\n");
        exit(EXIT_FAILURE);
    }
    
    ts.tv_sec = usec / 1000000;
    ts.tv_nsec = usec * 1000;
    nanosleep(&ts, NULL);
}

static void usage(void)
{
    printf("Usage: ./context_switching <map size (bytes)> (must be even) <iterations per context switch> <mmap function> (0 for system, 1 for custom driver)\n");
}

int main(int argc, char ** argv)
{
    int ret;
    unsigned long map_size;
    int switch_iteration;
    int use_mmap_driver = 0;
    int num_entries;
    int *map; // mmaped array of ints
    struct timespec start_time;
    struct timespec end_time;
    int configfd;

    if (argc != NUM_EXPECTED_ARGS)
    {
      usage();
      exit(EXIT_FAILURE);
    }

    map_size = atof(argv[1]);
    switch_iteration = atoi(argv[2]);
    use_mmap_driver = atoi(argv[3]);
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
        if (i % switch_iteration == 0)
        {
            // Yield the processor
            usec_sleep(SLEEP_USEC);
        }
    }

    munmap(map, map_size);
    get_time(&end_time);
    printf("Exeuction time: %.9f secs\n", timediff(start_time, end_time));
    
    return ret;    
}
