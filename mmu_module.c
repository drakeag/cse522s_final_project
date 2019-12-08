#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/vmalloc.h>
#include <asm/pgtable.h>
#include <asm/pgtable-2level.h>


/* Macros */

// Returns the translation base control register
#define get_ttbcr() \
	({ \
		unsigned int ttbcr; \
		asm("mrc p15, 0, %0, c2, c0, 2" : "=r" (ttbcr) :: ); \
		ttbcr; \
	})

// Returns the translation base register 0
#define get_ttbr0() \
	({ \
		unsigned int ttbr; \
		asm("mrc p15, 0, %0, c2, c0, 0" : "=r" (ttbr) :: );	\
		ttbr; \
	})

// Returns address pointed to by TTBR0
#define get_pgd() \
	({ \
		unsigned int pg = get_ttbr0(); \
		pg &= ~(PTRS_PER_PGD*sizeof(pgd_t)-1); \
		(pgd_t *)phys_to_virt(pg); \
	})

/* Globals */

// Char device variables
static struct cdev dev; 
static dev_t  dev_number;     
static struct class *dev_class;

// Fault variables
static unsigned int fault_counter = 0U;
static unsigned int save_fault = 0U; 

/**
 * @brief Called the first time a memory area is accessed 
 *        which is not in memory, it does the actual mapping between 
 *        kernel and user space memory
 */
vm_fault_t vma_fault(struct vm_fault *vmf)
{
	struct page *page = NULL;
	unsigned long offset;
	pgd_t *pgd;
	pte_t *pte;

	fault_counter++;
	printk(KERN_NOTICE "vma_fault %d\n", fault_counter);

	printk(KERN_NOTICE "faulting virtual address: %lx\n", vmf->address);
	offset = (unsigned long)(vmf->pgoff << PAGE_SHIFT);

	printk(KERN_NOTICE "mapping to physical address %lx\n", vmf->vma->vm_private_data + offset);
	
	// Normal 4KB page
	page = virt_to_page(vmf->vma->vm_private_data + offset);
	vmf->page = page;
	get_page(page);

	if (fault_counter == 1) {
		asm("mrc p15, 0, %0, c6, c0, 0" : "=r" (save_fault) ::);
		printk("saved fault 0%lx\n", save_fault);
	} else if (fault_counter >= 2) {

		// Get the first fault's values
		u32 l1_addr;
		asm("mrc p15, 0, %0, c2, c0, 0" : "=r" (l1_addr) :: );
		l1_addr &= ~(0x3FFFU);
		l1_addr |= (u32)(((save_fault >> 20) & 0xFFFU) << 2);
		l1_addr = phys_to_virt(l1_addr);
		printk("first fault l1_addr 0x%lx=0x%lx\n", l1_addr, *(u32*)l1_addr);

		u32 l1_val = *(u32*)l1_addr;
		u32 l2_addr = l1_val;
		l2_addr &= ~(0x3FFU);
		l2_addr |= (((save_fault >> 12) & 0xFFU) << 2);
		l2_addr = phys_to_virt(l2_addr);
		printk("first fault l2_addr 0x%lx=0x%lx\n", l2_addr, *(u32*)l2_addr);
		save_fault = *(u32*)l2_addr;

		asm("mrc p15, 0, %0, c2, c0, 0" : "=r" (l1_addr) :: );
		l1_addr &= ~(0x3FFFU);
		l1_addr |= (u32)((((u32)vmf->vma->vm_start >> 20) & 0xFFFU) << 2);
		l1_addr = phys_to_virt(l1_addr);
		printk("vma start l1_addr 0x%lx=0x%lx\n", l1_addr, *(u32*)l1_addr);

		l1_val = *(u32*)l1_addr;
		l2_addr = l1_val;
		l2_addr &= ~(0x3FFU);
		l2_addr |= ((((u32)vmf->vma->vm_start >> 12) & 0xFFU) << 2);
		l2_addr = phys_to_virt(l2_addr);
		printk("vma start l2_addr 0x%lx=0x%lx\n", l2_addr, *(u32*)l2_addr);

		// Need to move these for the large descriptor
		u32 large_pte_val = *(u32*)l2_addr;
		large_pte_val = save_fault;
		u32 tex = (large_pte_val >> 6) & 3U;               // mask out the tex bits
		u32 xn  = large_pte_val & 1U;                      // mask out the execute never bit
		large_pte_val &= ~(0xFFFFF000U);                   // clear lower address bits 
		large_pte_val &= ~(3U);                            // clear small pte bits
		large_pte_val |= virt_to_phys(vmf->vma->vm_private_data) | 1U | (tex << 12) | (xn << 15);    // make the large page descriptor
		
		u32* large_pte = (u32*)l2_addr;
		*large_pte = large_pte_val;

		u32 total_pages = (vmf->vma->vm_end - vmf->vma->vm_start) / 0x1000;
		u32 current_page = 0U;
		printk("%d pages for vma range 0x%lx - 0x%lx\n", total_pages, vmf->vma->vm_end, vmf->vma->vm_start);

		while (current_page < total_pages) {
			*large_pte = large_pte_val + ((current_page/16U) << 16); // increase the base address to map to the correct pages
			asm("mcr p15, 0, %0, c7, c10, 1" :: "r"(large_pte) :);	 // flush VMA
			//printk("ARM pte value 0x%lx=0x%lx\n", large_pte, *large_pte);
			//printk("0x%lx\n", virt_to_phys(vmf->vma->vm_private_data));
			
			// Move to next pte
			large_pte++;
			current_page++;
		}

		printk("remapped to %d 64KB pages\n", current_page/16U + 1);

			// Code for 1MB pages, untested
			// fault_addr -= 0x1000U;
			// u32 l1_addr = get_ttbr0();
			// l1_addr &= ~(0x3FFFU);
			// l1_addr |= (u32)((((u32)fault_addr >> 20) & 0xFFFU) << 2);
			// l1_addr = phys_to_virt(l1_addr);
			// printk("l1_addr 0x%lx=0x%lx\n", l1_addr, *(u32*)l1_addr);

			// u32 l1_val = *(u32*)l1_addr;
			// u32 l2_addr = l1_val;
			// l2_addr &= ~(0x3FFU);
			// l2_addr |= ((((u32)fault_addr >> 12) & 0xFFU) << 2);
			// l2_addr = phys_to_virt(l2_addr);
			// printk("l2_addr 0x%lx=0x%lx\n", l2_addr, *(u32*)l2_addr);

			// u32 l2_val = *(u32*)l2_addr;
			// u32 section = 0U;
			// section |= l1_val & 0x3E0U;         // imp, domain bits
			// section |= (l1_val & 0x8U) << 16;   // ns bit
			// section |= l2_val & 0xFFF00000U;    // base address
			// section |= (l2_val & 0xFF0U) << 6;  // ng, s, ap[2], tex, ap[1:0] bits
			// section |= (l2_val & 0xEU);         // c, b, 1 bits
			// section |= (l2_val & 1U) << 4;      // xn bit
			// section &= ~((1UL << 18) | 1U);     // Clear bit 18 to make it normal 1MB section
			// *(u32*)l1_addr = section;
			
			// printk("section value %lx\n", *(u32*)l1_addr);
			// asm("mcr p15, 0, %0, c7, c10, 1" :: "r"(l1_addr) :); // flush VMA
	}

	return 0;
}

void vma_open(struct vm_area_struct* vma)
{
	printk(KERN_INFO "vma open at virtual address %lx\n", vma->vm_start);
}

void vma_close(struct vm_area_struct* vma)
{
	printk(KERN_INFO "vma close\n");
}

static struct vm_operations_struct mmap_vm_ops = {
	.fault = vma_fault,
	.open  = vma_open,
	.close = vma_close
};

/**
 * @brief Overloads the mmap nopage operation for the file, defers mapping till
 *        the memory is accessed
 * 
 */
static int driver_mmap(struct file* filp, struct vm_area_struct *vma)
{
	printk(KERN_INFO "Driver mmap\n");
	vma->vm_private_data = filp->private_data;
	vma->vm_ops = &mmap_vm_ops;
	vma_open(vma);
	return 0;
}

static int driver_release(struct inode* inodep, struct file* filep)
{
	printk(KERN_INFO "Driver released\n");
	kfree(filep->private_data);
	return 0;
}

static int driver_open(struct inode* inodep, struct file* filep)
{
	printk(KERN_INFO "Driver opened\n");
	filep->private_data = kmalloc(0x100000, GFP_KERNEL);
	if (!filep->private_data)
		return -1;
	return 0;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.mmap  = driver_mmap,
	.open  = driver_open,
	.release = driver_release,
};
 
static int __init m_init(void)
{
    printk(KERN_ALERT "Project module initialized\n");

	if (alloc_chrdev_region(&dev_number, 0, 1, "cse522s") < 0) {
		return -1;
	}
	if ((dev_class = class_create(THIS_MODULE, "chardrv")) == NULL) {
		unregister_chrdev_region(dev_number, 1);
		return -1;
	}
	if (device_create(dev_class, NULL, dev_number, NULL, "cproj") == NULL) {
		class_destroy(dev_class);
		unregister_chrdev_region(dev_number, 1);
		return -1;
	}
	cdev_init(&dev, &fops);
	if (cdev_add(&dev, dev_number, 1) == -1) {
		device_destroy(dev_class, dev_number);
		class_destroy(dev_class);
		unregister_chrdev_region(dev_number, 1);
		return -1;
	}

	return 0;
}

static void __exit m_exit(void)
{
    printk(KERN_ALERT "Project module is being unloaded\n");
	cdev_del(&dev);
	device_destroy(dev_class, dev_number);
	class_destroy(dev_class);
	unregister_chrdev_region(dev_number, 1);
}

module_init(m_init);
module_exit(m_exit);

MODULE_LICENSE ("GPL");
MODULE_DESCRIPTION ("CSE 522s Project");

