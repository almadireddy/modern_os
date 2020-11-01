// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).
	//
	//   variables: 
	//   FEC_WR - Page fault caused by a write
	//   uvpt - user read only virtual page table 
	//   page number field of address - PPN(pa) - (((uintptr_t) (pa)) >> PTXSHIFT)

	// LAB 4: Your code here.
	
	if(!(err &FEC_WR)&&(uvpt[PPN(addr)] &PTE_COW)){
		
		panic("pagefault found by PTE_COW, page isn't writable.");

	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	//   No need to explicitly delete the old page's mapping.

	// LAB 4: Your code here.

	void *page_addr;
	if((r = sys_page_alloc(0, (void*)PFTEMP, PTE_USER)) == 0){
		
		page_addr = ROUNDDOWN(addr, PGSIZE);
		memmove(PFTEMP, page_addr, PGSIZE);

		if((r = sys_page_map(0, PFTEMP, 0,page_addr, PTE_USER)) < 0){
			
			panic("sys_page_map has failed at PFTEMP: %e",r);
		
		}
		if((r = sys_page_unmap(0, PFTEMP)) < 0){

			panic("sys_page_unmap has failed at PFTEMP: %e",r);
		}

	}else {
		
		panic("sys_page_alloc failed during page_fault: %e",r);

	}
	
	//panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	
	int perm = (uvpt[pn])&0x1FF; //use page directory index
	void *addr = (void *)((uint64_t)pn *PGSIZE);

	if(perm &PTE_W || perm &PTE_COW){

		perm = ((perm | (PTE_P|PTE_U|PTE_COW)) &(~PTE_W));

		if((r = sys_page_map(0, addr, envid, addr, perm)) < 0){
			
			panic("sys_page_map has failed with PTE_COW: %e",r);
		
		}
		if((r = sys_page_map(0, addr, 0, addr, perm)) < 0){
		
			panic("sys_page_map has failed with PTE_COW: %e",r);
		
		}

	} else{

		if((r = sys_page_map(0, addr, envid, addr, perm)) < 0){
			
			panic("sys_page_map has failed with PTE_COW: %e",r);
		
		}
		
	}

	//panic("duppage not implemented");
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	
	envid_t envid;
	extern void _pgfault_upcall(void);
	uint8_t *addr;
	extern char end[];
	set_pgfault_handler(pgfault);

	envid = sys_exofork();
	if (envid < 0){

		panic("envid less than 0, process creation has failed.");
	}
	else if (envid == 0){

		thisenv = &envs[ENVX(sys_getenvid())];
		return envid;
	}
	else {

		for (addr = (uint8_t*)USTACKTOP - PGSIZE; addr >= (uint8_t*)UTEXT; addr -= PGSIZE){

			if(uvpml4e[VPML4E(addr) & PTE_P]){

				if(uvpde[VPDPE(addr) & PTE_P]){

					if(uvpd[VPD(addr) & PTE_P]){

						if(uvpt[VPN(addr) & PTE_P]){
							
							duppage(envid, VPN(addr));
						}
					} else {
						
						addr -= NPDENTRIES*PGSIZE;
					}
				} else{

					addr -= NPDENTRIES*NPDENTRIES*PGSIZE;
				}
			}

		}
	}
	
	int r;
	if ((r = sys_env_set_pgfault_upcall(envid, (void*)_pgfault_upcall)) < 0){
		
		panic("sys_env_set_pgfault has failed: %e",r);
	}
	if ((r = sys_page_alloc(envid, (void*)UXSTACKTOP - PGSIZE, PTE_USER)) < 0){
		
		panic("sys_page_alloc has failed: %e",r);
	}
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0){

		panic("sys_env_set_status has failed: %e",r);
	}

	return envid;


	//panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
