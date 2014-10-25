#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include "hook.h"

/* TODO: make less ugly! 
 *       there's got to be a nicer way to do this! */
#pragma pack(push, 1)
static struct { 
	char push_rax; 
	char mov_rax[2];
	char addr[8];
	char jmp_rax[2]; } 
jump_asm = {
	.push_rax = 0x50,
	.mov_rax  = {0x48, 0xb8},
	.jmp_rax  = {0xff, 0xe0} };
#pragma pack(pop)

static int unprotect_page(void* addr) {
	int pagesize = sysconf(_SC_PAGE_SIZE);
	int pagemask = ~(pagesize -1);
	char* page = (char *)((size_t)addr & pagemask);
	return mprotect(page, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC);
}

int hook_function(void* target, void* replace) {
	int count;

	if(unprotect_page(replace)) {
		fprintf(stderr, "Could not unprotect replace mem: %p\n", replace);
		return 1;
	}

	if(unprotect_page(target)) {
		fprintf(stderr, "Could not unprotect target mem: %p\n", target);
		return 1;
	}

	/* find the NOP */
	for(count = 0; count < 255 && ((unsigned char*)replace)[count] != 0x90; ++count);

	if(count == 255) {
		fprintf(stderr, "Couldn't find the NOP.\n");
		return 1;
	}

	/* shift everything down one */
	memmove(replace+1, replace, count);

	/* add in `pop %rax` */
	*((unsigned char *)replace) = 0x58;

	/* set up the address */
	memcpy(jump_asm.addr, &replace, sizeof (void *));

	/* smash the target function */
	memcpy(target, &jump_asm, sizeof jump_asm);

	return 0;
}
