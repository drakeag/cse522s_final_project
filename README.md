# Mmap Driver for Dynamic Page Size Allocation

Alex Drake and Nick Norden

[CSE 522S Class Website](https://www.cse.wustl.edu/~brian.kocoloski/courses/cse522s/)

## Usage
#### Build MMU kernel module (optional dmesg clear)
```
$ make
$ sudo dmesg --clear
```
#### Load MMU kernel module (optional dmesg to verify load)
```
$ sudo insmod mmu_module.ko
$ dmesg
````
#### Run tests
```
$ ./run_tests.sh
````
#### Unload MMU kernel module (optional dmesg to verify unload)
```
$ sudo rmmod mmu_module
$ dmesg
```


## Motivation
The memory management paradigm Linux uses is called demand paging. Demand paging means that a process will not be mapped completely into RAM when it is first executed. Over the course of process execution an operation on a virtual address that has not been mapped and cause a page fault. On the page fault, If the necessary conditions are met, the kernel will then map the physical memory into the process’ virtual address space. 

The mapping from physical to virtual memory is done is discrete chunks (pages), Linux uses the smallest available page size for mappings. Using the smallest page size (4KB) has benefits of not over allocating. This works well for the majority of applications; however, smaller pages increase the total entries in the TLB. Consider a TLB with 4K entries and a process that has a 4MB data structure, the kernel would use up 25% of the TLB for one structure using 4KB pages. Consider instead using 2MB pages to map the data structure. Using 2MB pages reduces the TLB usage from 25% to 0.025%, a desirable improvement. Furthermore, the 2MB page will use one less stage of translation to form the physical address. 

Therefore, the motivation to use large page sizes is realized in the more efficient use of TLB entries and the decrease in overall time needed to access memory. 

## Solution Overview
The proposed solution will be a mmap driver that will dynamically use 2MB and 4KB page sizes when appropriate. The high level overview is that the kernel module will create a file and register its own file operations on it. The file “open” operation will then register a custom mmap operation that will dynamically handle page sizes. Such that a user space application will be able to “allocate” memory by calling mmap on the file created by the module. This will allow the kernel module to manage the user space process’ mappings, importantly, the mapping page size. This would be an alternative to using other dynamic memory allocation functions from user space. The algorithm to remap pages will be a compile time constant based on the percentage of 2MB requesting to be mapped, and tuned based testing. Kernel functions for creating the 2MB pages and inserting them will be inspired by the HugePages library, but effort will be made to approach the page table modification as low level as possible and recreate kernel functions specifically for our needs.
