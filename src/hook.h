#ifndef HOOK_H
#define HOOK_H

#if !(__x86_64__)
	#error "This only works on x86_64"
#endif

int hook_function(void* target, void* replace);

#endif
