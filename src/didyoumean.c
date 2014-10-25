#include <Python.h>

#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include "didyoumean-safe.h"

#if !(__x86_64__)
	#error "This only works on x86_64"
#endif

extern PyObject* PyErr_Occurred(void);
extern PyObject* PyObject_GetAttr(PyObject *v, PyObject *name);

static int distance(char* a, char* b) {
	size_t maxi = strlen(b);
	size_t maxj = strlen(a);
	int i, j;

	unsigned int compare[maxi+1][maxj+1];

	compare[0][0] = 0;
	for (i = 1; i <= maxi; i++) compare[i][0] = i;
	for (j = 1; j <= maxj; j++) compare[0][j] = j;

	for (i = 1; i <= maxi; i++) {
		for (j = 1; j <= maxj; j++) {
			int left   = compare[i-1][j] + 1;
			int right  = compare[i][j-1] + 1;
			int middle = compare[i-1][j-1] + (a[j-1] == b[i-1] ? 0 : 1);

			if( left < right && left < middle )       compare[i][j] = left;
			else if( right < left && right < middle ) compare[i][j] = right;
			else                                      compare[i][j] = middle;
		}
	}
 
	return compare[maxi][maxj];
}

PyObject* trampoline(PyObject *v, PyObject *name)
{
	__asm__("nop");
	PyObject* rv = NULL;
	PyTypeObject *tp = Py_TYPE(v);

	if (!PyString_Check(name)) {
#ifdef Py_USING_UNICODE
		/* The Unicode to string conversion is done here because the
		   existing tp_getattro slots expect a string object as name
		   and we wouldn't want to break those. */
		if (PyUnicode_Check(name)) {
			name = _PyUnicode_AsDefaultEncodedString(name, NULL);
			if (name == NULL)
				return NULL;
		}
		else
#endif
		{
			PyErr_Format(PyExc_TypeError,
			             "attribute name must be string, not '%.200s'",
			             Py_TYPE(name)->tp_name);
			return NULL;
		}
	}
	if (tp->tp_getattro != NULL) {
		rv = (*tp->tp_getattro)(v, name);
	}
	else if (tp->tp_getattr != NULL) {
		rv = (*tp->tp_getattr)(v, PyString_AS_STRING(name));
	}
	else {
		PyErr_Format(PyExc_AttributeError,
		             "'%.50s' object has no attribute '%.400s'",
		             tp->tp_name, PyString_AS_STRING(name));
	}

	if(!rv && PyErr_ExceptionMatches(PyExc_AttributeError)) {
		PyThreadState *tstate = PyThreadState_GET();

		PyObject *oldtype, *oldvalue, *oldtraceback;
		oldtype = tstate->curexc_type;
		oldvalue = tstate->curexc_value;
		oldtraceback = tstate->curexc_traceback;

		/* clear exception temporarily */
		tstate->curexc_type = NULL;
		tstate->curexc_value = NULL;
		tstate->curexc_traceback = NULL;

		PyObject* dir = safe_PyObject_Dir(v);
		Py_LeaveRecursiveCall();

		PyObject* candidate = NULL;
		PyObject* newvalue = oldvalue;
		if(dir) {
			int candidate_dist = PyString_Size(name);
			int i;
			for(i = 0; i < PyList_Size(dir); ++i) {
				PyObject *item = PyList_GetItem(dir, i);
				int dist = distance(PyString_AS_STRING(name), PyString_AS_STRING(item));
				if(!candidate || dist < candidate_dist ) {
					candidate = item;
					candidate_dist = dist;
				}	
			}

			if( candidate ) {
				newvalue = PyString_FromFormat("%s\n\nMaybe you meant: .%s\n",
				                                PyString_AS_STRING(oldvalue),
				                                PyString_AS_STRING(candidate));
		
				Py_DECREF(oldvalue);
			}
		}
		PyErr_Clear(); /* clear exception if something else has set it */
		PyErr_Restore(oldtype, newvalue, oldtraceback);
	}

    return rv;
}

/* TODO: make less ugly! 
 *       there's got to be a nicer way to do this! */
#pragma pack(push, 1)
struct { 
	char push_rax; 
	char mov_rax[2];
	char addr[8];
	char jmp_rax[2]; } 
jump_asm = {
	.push_rax = 0x50,
	.mov_rax  = {0x48, 0xb8},
	.jmp_rax  = {0xff, 0xe0} };
#pragma pack(pop)

static PyMethodDef module_methods[] = {
	{NULL} /* Sentinel */
};

PyDoc_STRVAR(module_doc,
"This module implements a \"did you mean?\" functionality on getattr/LOAD_ATTR.\n"
"(It's not so much what it does but how it does it.)");

static int unprotect_page(void* addr) {
	int pagesize = sysconf(_SC_PAGE_SIZE);
	int pagemask = ~(pagesize -1);
	char* page = (char *)((size_t)addr & pagemask);
	return mprotect(page, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC);
}

static int hook_function(void* target, void* replace) {
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

PyDoc_STRVAR(safe_getattr_doc,
"getattr(object, name[, default]) -> value\n\
\n\
Get a named attribute from an object; getattr(x, 'y') is equivalent to x.y.\n\
When a default argument is given, it is returned when the attribute doesn't\n\
exist; without it, an exception is raised in that case.");

PyMethodDef builtin_methods[1] = {{"getattr", safe_builtin_getattr, METH_VARARGS, safe_getattr_doc}};

PyMODINIT_FUNC
initdidyoumean(void) {
	__asm__("");

	Py_InitModule3("didyoumean", module_methods, module_doc);

	if(hook_function(PyObject_GetAttr, &trampoline))
		fprintf(stderr, "Function hooking failed.\n");

	/* sometimes builtin_getattr calls to PyObject_Getattr get optimised out, 
 	 * so let's replace it with a safe version */
	PyObject* builtin_str = PyString_FromString("__builtin__");
	PyObject* builtin_mod = PyImport_Import(builtin_str);
    PyObject* builtin_dict = PyModule_GetDict(builtin_mod);
	PyObject* builtin_getattr = PyCFunction_NewEx(builtin_methods, NULL, builtin_str);

	if(builtin_getattr)
    	PyDict_SetItemString(builtin_dict, builtin_methods->ml_name, builtin_getattr);

	Py_XDECREF(builtin_getattr);
	Py_DECREF(builtin_str);
	Py_DECREF(builtin_mod);
}
