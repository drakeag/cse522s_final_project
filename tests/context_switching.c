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

#define NUM_EXPECTED_ARGS 5
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
    printf("Usage: ./context_switching <size> <offset> <switch percent> <mmap function>\n");
    printf("\t<size>          : map size in bytes, must be even\n");
    printf("\t<offset>        : must be positive\n");
    printf("\t<switch percent>: context switch percentage between 0 and 1\n");
    printf("\t<mmap function> : 0 for system, 1 for custom driver\n");
}

int main(int argc, char ** argv)
{
    int i = 0;
    int ret;
    unsigned int step = 0;
    unsigned long map_size;
    float switch_percent;
    int switch_iteration;
    int use_mmap_driver = 0;
    int *address = NULL;
    int *orig_addr = NULL;
    unsigned long int offset;
    struct timespec start_time;
    struct timespec end_time;
    int configfd;

    if (argc != NUM_EXPECTED_ARGS)
    {
      usage();
      exit(EXIT_FAILURE);
    }

    sscanf(argv[1], "%lu", &map_size);
    sscanf(argv[2], "%lu", &offset);
    switch_percent = atof(argv[3]);
    use_mmap_driver = atoi(argv[4]);
    
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
    
    if (switch_percent < 0 || switch_percent > 1)
    {
        printf("Switch percentage must be between 0 and 1\n");
        usage();
        exit(EXIT_FAILURE);
    }
    
    switch_iteration = (map_size / offset) / ((map_size / offset) * switch_percent);
    
    get_time(&start_time);
    
    orig_addr = get_mmap_addr(map_size, use_mmap_driver);
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
        if (i % switch_iteration == 0)
        {
            // Yield the processor
            usec_sleep(SLEEP_USEC);
        }
	}

    munmap(orig_addr, map_size);
    get_time(&end_time);
    printf("Exeuction time: %.9f secs\n", timediff(start_time, end_time));
    
    return ret;    
}
