#include <Python.h>
#include "levenshtein.h"
#include "hook.h"
#include "safe_PyObject_Dir.h"

extern PyObject* PyObject_GetAttr(PyObject *v, PyObject *name);

PyObject* hooked_PyObject_GetAttr(PyObject *v, PyObject *name)
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

PyObject *
hooked_builtin_getattr(PyObject *self, PyObject *args)
{
	PyObject *v, *result, *dflt = NULL;
	PyObject *name;

	if (!PyArg_UnpackTuple(args, "getattr", 2, 3, &v, &name, &dflt))
		return NULL;
#ifdef Py_USING_UNICODE
	if (PyUnicode_Check(name)) {
		name = _PyUnicode_AsDefaultEncodedString(name, NULL);
		if (name == NULL)
		return NULL;
	}
#endif

	if (!PyString_Check(name)) {
		PyErr_SetString(PyExc_TypeError,
		                "getattr(): attribute name must be string");
		return NULL;
	}
	result = PyObject_GetAttr(v, name);
	if (result == NULL && dflt != NULL &&
	PyErr_ExceptionMatches(PyExc_AttributeError))
	{
		PyErr_Clear();
		Py_INCREF(dflt);
		result = dflt;
	}
	return result;
}

PyDoc_STRVAR(hooked_getattr_doc,
"getattr(object, name[, default]) -> value\n\
\n\
Get a named attribute from an object; getattr(x, 'y') is equivalent to x.y.\n\
When a default argument is given, it is returned when the attribute doesn't\n\
exist; without it, an exception is raised in that case.");

PyMethodDef builtin_methods[1] = {
	{"getattr", hooked_builtin_getattr, METH_VARARGS, hooked_getattr_doc}
};

PyDoc_STRVAR(module_doc,
"This module implements a \"did you mean?\" functionality on getattr/LOAD_ATTR.\n"
"(It's not so much what it does but how it does it.)");

static PyMethodDef module_methods[] = {
	{NULL} /* Sentinel */
};

PyMODINIT_FUNC
initdidyoumean(void) {
	__asm__("");

	Py_InitModule3("didyoumean", module_methods, module_doc);

	if(hook_function(PyObject_GetAttr, &hooked_PyObject_GetAttr))
		fprintf(stderr, "Function hooking failed.\n");

	/* sometimes builtin_getattr calls to PyObject_Getattr get optimised out; 
	 *   let's replace it with a safer version! */
	PyObject* builtin_str = PyString_FromString("__builtin__");
	PyObject* builtin_mod = PyImport_Import(builtin_str);
	PyObject* builtin_dict = PyModule_GetDict(builtin_mod);

	/* we might be able to get a handle on __builtin__.getattr before
	 *   this code runs, so let's just patch that directly */
	PyObject* getattr_func = PyDict_GetItemString(builtin_dict, "getattr");
	if(getattr_func)
		((PyCFunctionObject*)getattr_func)->m_ml->ml_meth = hooked_builtin_getattr;

	Py_DECREF(builtin_str);
	Py_DECREF(builtin_mod);
}
