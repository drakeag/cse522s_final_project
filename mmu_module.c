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

#define get_ttbcr() \
	({ \
		unsigned int ttbcr; \
		asm("mrc p15, 0, %0, c2, c0, 2" : "=r" (ttbcr) :: ); \
		ttbcr; \
	})

#define get_ttbr0() \
	({ \
		unsigned int ttbr; \
		asm("mrc p15, 0, %0, c2, c0, 0" : "=r" (ttbr) :: );	\
		ttbr; \
	})

#define get_pgd() \
	({ \
		unsigned int pg = get_ttbr0(); \
		pg &= ~(PTRS_PER_PGD*sizeof(pgd_t)-1); \
		(pgd_t *)phys_to_virt(pg); \
	})

static dev_t first; // Global variable for the first device number 
static struct cdev c_dev; // Global variable for the character device structure
static struct class *cl; // Global variable for the device class


#define L1_PTE_IDX(x) ((x & 0xFFFFFC00U) >> 10)
#define get_ptep(tt_base, vaddr) (unsigned int*)(tt_base + L1_PTE_IDX(vaddr))

static unsigned int fault_counter = 0;
static unsigned long save_fault = 0; 
unsigned int addr;

static unsigned int vaddr2paddr(unsigned int vaddr)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	unsigned int paddr = 0;
	unsigned int page_addr = 0;
	unsigned int page_offset = 0;

	pgd = pgd_offset(current->mm, vaddr);
	if (pgd_none(*pgd)) {
		printk("not mapped in pgd\n");
		return -1;
	}

	pud = pud_offset(pgd, vaddr);
	if (pud_none(*pud)) {
		printk("not mapped in pud\n");
		return -1;
	}

	pmd = pmd_offset(pud, vaddr);
	if (pmd_none(*pmd)) {
		printk("not mapped in pmd\n");
		return -1;
	}

	pte = pte_offset_kernel(pmd, vaddr);
	printk("pte val = 0x%lx\n", pte_val(*pte));
	printk("pte address = %p\n", pte);
	if (pte_none(*pte)) {
		printk("not mapped in pte\n");
		return -1;
	}


	/* Page frame physical address mechanism | offset */
	page_addr = pte_val(*pte) & PAGE_MASK;
	page_offset = vaddr & ~PAGE_MASK;
	paddr = page_addr | page_offset;
	printk("page_addr = %lx, page_offset = %lx\n", page_addr, page_offset);
		printk("vaddr = %lx, paddr = %lx\n", vaddr, paddr);

	return paddr;
}

static void mk_largepage(unsigned int address)
{
	// // Get the base table address in hardware
	// unsigned int ttbr0;
	// asm volatile ("mrc p15, 0, %0, c2, c0, 1" : "=r" (ttbr0));

	// printk(KERN_INFO "ttrb0 base address %lx\n", ttbr0);

	// // Get the virtual address ptep
	// unsigned int* ptep = get_ptep(ttbr0, address);

	// printk(KERN_INFO "ptep value %lx, address @ %p\n", *ptep, ptep);

	// // Update the large page bit
	// *ptep &= ~(1U << 1);
	// *ptep |= 1U;

	// printk(KERN_INFO "ptep new value %lx", *ptep, ptep);

	// // Flush the mmu
	// asm volatile("mcr	p15, 0, %0, c7, c10, 1" :: "r" (ptep));

	// Get the base table address in hardware
	long unsigned int ttbr0;
	long unsigned int fault_addr;
	long unsigned int fault;
	long unsigned int pdg_idx;
	long unsigned int pgt_idx;
	long unsigned int* pde;
	
	asm volatile ("mrc p15, 0, %0, c2, c0, 0" : "=r" (ttbr0) ::);
	printk("ttrb0 phys base address 0x%lx\n", ttbr0);
	
	asm volatile ("mrc p15, 0, %0, c5, c0, 0" : "=r" (fault) ::);
	
	if (fault & (1UL << 11U)) {
		
		asm volatile ("mrc p15, 0, %0, c6, c0, 0" : "=r" (fault_addr) ::);
		printk("faulting address 0x%ld\n", fault_addr);
	
		pdg_idx = (address & 0xFFF00000) >> 20U;
		printk("pdg idx %ld\n", pdg_idx);
		pde = (long unsigned int*)((long unsigned int)ttbr0 + (pdg_idx << 2U));
		printk("pde address 0x%lx\n", pde);

		if ((*pde & 3UL) == 0U) {
			printk("Address not mapped 0x%lx\n", *pde);
			return;
		}

		printk("pde val 0x%lx\n", *pde);
		*pde |= 1UL;
		*pde &= ~(1UL << 1U);
		printk("pde val 0x%lx\n", *pde);
	}
}

/**
 * @brief Called the first time a memory area is accessed 
 *        which is not in memory, it does the actual mapping between 
 *        kernel and user space memory
 */
vm_fault_t vma_fault(struct vm_fault *vmf)
{
	struct page *page = NULL;
	unsigned long offset;
	unsigned long fault_addr;
	unsigned long int* hw_pte;
	unsigned long int* new_pte;
	pgd_t *pgd;
	pte_t *pte;
	uint8_t tex = 0;
	uint8_t xn = 0;
	unsigned long int i = 0;

	fault_counter++;
	printk(KERN_NOTICE "vma_fault %d\n", fault_counter);

	printk(KERN_NOTICE "faulting virtual address: %lx\n", vmf->address);
	offset = (unsigned long)(vmf->pgoff << PAGE_SHIFT);

	printk(KERN_NOTICE "mapping to physical address %lx\n", vmf->vma->vm_private_data + offset);
	
	/* Normal 4KB page */
	page = virt_to_page(vmf->vma->vm_private_data + offset);
	vmf->page = page;
	get_page(page);

	/*********************************************************************************************/

	// if (fault_counter == 1U) {

		asm("mrc p15, 0, %0, c6, c0, 0" : "=r" (fault_addr) ::);
		printk("fault 0x%lx\n", fault_addr);
		// return 0;
	// } else if (fault_counter == 2U) {

	// 	fault_addr -= (unsigned long)0x1000;
		printk("Turning pte 0x%lx into large pte\n", fault_addr);
		
		pgd = get_pgd() + pgd_index(fault_addr);
		printk("pgd index 0x%x\n", pgd_index(fault_addr));
		printk("ttbr0 value       = %08llx\n", (long long)*(u32*)&pgd[0]);

		pte = (pte_t*)(pmd_page_vaddr(*(pmd_t*)pgd) + pte_index(fault_addr));
		printk("linux pte address = %lx\n", pte);
		printk("linux pte value   = %lx\n", pte_val(*pte));
		printk("ARM pte address   = %lx\n", (pte_t*)((u32)pte + (u32)2048));

		hw_pte = (unsigned long int*)((unsigned long int)pte + 2048UL);
		printk("ARM pte value     = %08llx\n", (long long)*hw_pte);

		// //Now 15 more times

		if (fault_counter == 2) {
			i = 0;
			hw_pte--; // Go back to the original fault

			// Need to move these for the large descriptor
			*hw_pte &= ~(0x0000F000U); // clear lower address bits
			*hw_pte &= ~(3U);          // clear small pte bits
			tex = (*hw_pte >> 6) & 3U; // mask out the tex bits
			xn  = *hw_pte & 1U;        // mask out the execute never bit
			*hw_pte |= 1UL | (tex << 12U) | (xn << 15U); // make the large page descriptor

			asm("mcr p15, 0, %0, c7, c10, 1" :: "r"(hw_pte) :);	// flush VMA
			printk("%lx ARM pte value = %08llx\n", (pte_t*)hw_pte, (long long)*(hw_pte));
			
			*new_pte = *hw_pte;

			while(i < 15) {
				hw_pte++;
				i++;

				*hw_pte = *new_pte;
				asm("mcr p15, 0, %0, c7, c10, 1" :: "r"(hw_pte) :);	// flush VMA
				printk("%lx ARM pte value = %08llx\n", (pte_t*)hw_pte, (long long)*(hw_pte));
			}
		}

	//vmf->page = NULL;
	return 0;//VM_FAULT_NOPAGE;

	/* Modified 64MB page
		Get lock
		vmf->pte = pte_offset_map_lock(vmf->vma->mm, vmf->pmd, vmf->address, &vmf->ptl);

		page = alloc_page_vma(vmf->gfp_mask, vmf->vma, vmf->address);
		make_huge_pte(vmf->vma, page, &entry);

		*vmf->pte = entry; 
		pmd_populate(vmf->vma->mm, vmf->pmd, pgtable_t ptep)
		update_mmu_cache(vmf->vma, vmf->address, vmf->pte);

		printk(KERN_NOTICE "new pte %lx\n", (unsigned long)*vmf->pte);

		pte_unmap_unlock(vmf->pte, vmf->ptl);
		return VM_FAULT_NOPAGE;
	*/
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

	// 128 KB
	filep->private_data = kmalloc(0x10000, GFP_KERNEL);

	// vaddr2paddr(filep->private_data);

	addr = filep->private_data;
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


	if (alloc_chrdev_region(&first, 0, 1, "cse522s") < 0) {
		return -1;
	}

	if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL) {
		unregister_chrdev_region(first, 1);
		return -1;
	}

	if (device_create(cl, NULL, first, NULL, "cproj") == NULL) {
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}
	cdev_init(&c_dev, &fops);

	if (cdev_add(&c_dev, first, 1) == -1) {
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}

	return 0;
}

static void __exit m_exit(void)
{
    printk(KERN_ALERT "Project module is being unloaded\n");
	cdev_del(&c_dev);
	device_destroy(cl, first);
	class_destroy(cl);
	unregister_chrdev_region(first, 1);
}

module_init(m_init);
module_exit(m_exit);

MODULE_LICENSE ("GPL");
MODULE_DESCRIPTION ("CSE 522s Project");


//