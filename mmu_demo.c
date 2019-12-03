#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>


#define LARGE_PAGE 0x100000

int main(int argc, char** argv)
{
	unsigned long int offset = 0x1000;
	unsigned int step = 0;
	int configfd;
	configfd = open("/dev/cproj", O_RDWR);
	if(configfd < 0) {
		perror("open");
		return -1;
	}
	int* address = NULL;
	address = (int*)mmap((void*)0x76E00000U, LARGE_PAGE, PROT_READ|PROT_WRITE, MAP_SHARED, configfd, 0);
	if (address == MAP_FAILED) {
		perror("mmap");
		return -1;
	}

	//*(int*)(address + 0x4000) = 1;
    
	while((offset * step) < LARGE_PAGE) {
		*address = step;
		printf("0x%lx=%d\n", address, *address);
		address += 0x1000/sizeof(int);
		step++;
	}

    printf("pid %d\n", getpid());
    pause();

    return 0;
}
