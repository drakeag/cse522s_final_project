#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>

/**
 * @brief Find the difference in time between timeinitial and timefinal
 */ 
double timediff(struct timespec ti, struct timespec tf)
{
    return (double)((tf.tv_sec - ti.tv_sec)
      + (tf.tv_nsec - ti.tv_nsec)
      / 1E9);
}

void get_time(struct timespec *time)
{
    clock_gettime(CLOCK_MONOTONIC_RAW, time);
}

int *get_mmap_addr(int size, int mmap_function)
{
    int configfd;
    int *address;
  
    if (mmap_function == 0)
    {
        address = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    }
    else
    {
        configfd = open("/dev/cproj", O_RDWR);
        if(configfd < 0)
        {
            perror("open");
            exit(EXIT_FAILURE);
        }
        //address = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, configfd, 0);
        address = mmap((void*)0x76E00000U, size, PROT_READ | PROT_WRITE, MAP_SHARED, configfd, 0);
    }
  
    return address;
}

/**
 * @brief Returns a random number in the range [min, max]. 
 */
int get_rand(int min, int max)
{ 
    return (rand() % (max - min + 1)) + min; 
} 
