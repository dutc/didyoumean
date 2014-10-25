#include <Python.h>
#include "safe_PyObject_Dir.h"

static int safe_merge_list_attr(PyObject* dict, PyObject* obj, const char *attrname);
static int safe_merge_class_dict(PyObject* dict, PyObject* aclass);
static PyObject * safe_PyObject_GetAttr(PyObject *v, PyObject *name);
static PyObject * safe_PyObject_GetAttrString(PyObject *v, const char *name);
static PyObject * safe__generic_dir(PyObject *obj);
static PyObject * safe__specialized_dir_type(PyObject *obj);
static PyObject * safe__specialized_dir_module(PyObject *obj);
static PyObject * safe__dir_object(PyObject *obj);

static int
safe_merge_list_attr(PyObject* dict, PyObject* obj, const char *attrname)
{
    PyObject *list;
    int result = 0;

    assert(PyDict_Check(dict));
    assert(obj);
    assert(attrname);

    list = safe_PyObject_GetAttrString(obj, attrname);
    if (list == NULL)
        PyErr_Clear();

    else if (PyList_Check(list)) {
        int i;
        for (i = 0; i < PyList_GET_SIZE(list); ++i) {
            PyObject *item = PyList_GET_ITEM(list, i);
            if (PyString_Check(item)) {
                result = PyDict_SetItem(dict, item, Py_None);
                if (result < 0)
                    break;
            }
        }
        if (Py_Py3kWarningFlag &&
            (strcmp(attrname, "__members__") == 0 ||
             strcmp(attrname, "__methods__") == 0)) {
            if (PyErr_WarnEx(PyExc_DeprecationWarning,
                           "__members__ and __methods__ not "
                           "supported in 3.x", 1) < 0) {
                Py_XDECREF(list);
                return -1;
            }
        }
    }

    Py_XDECREF(list);
    return result;
}

static int
safe_merge_class_dict(PyObject* dict, PyObject* aclass)
{
    PyObject *classdict;
    PyObject *bases;

    assert(PyDict_Check(dict));
    assert(aclass);

    /* Merge in the type's dict (if any). */
    classdict = safe_PyObject_GetAttrString(aclass, "__dict__");
    if (classdict == NULL)
        PyErr_Clear();
    else {
        int status = PyDict_Update(dict, classdict);
        Py_DECREF(classdict);
        if (status < 0)
            return -1;
    }

    /* Recursively merge in the base types' (if any) dicts. */
    bases = safe_PyObject_GetAttrString(aclass, "__bases__");
    if (bases == NULL)
        PyErr_Clear();
    else {
        /* We have no guarantee that bases is a real tuple */
        Py_ssize_t i, n;
        n = PySequence_Size(bases); /* This better be right */
        if (n < 0)
            PyErr_Clear();
        else {
            for (i = 0; i < n; i++) {
                int status;
                PyObject *base = PySequence_GetItem(bases, i);
                if (base == NULL) {
                    Py_DECREF(bases);
                    return -1;
                }
                status = safe_merge_class_dict(dict, base);
                Py_DECREF(base);
                if (status < 0) {
                    Py_DECREF(bases);
                    return -1;
                }
            }
        }
        Py_DECREF(bases);
    }
    return 0;
}

static PyObject *
safe_PyObject_GetAttr(PyObject *v, PyObject *name)
{
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
    if (tp->tp_getattro != NULL)
        return (*tp->tp_getattro)(v, name);
    if (tp->tp_getattr != NULL)
        return (*tp->tp_getattr)(v, PyString_AS_STRING(name));
    PyErr_Format(PyExc_AttributeError,
                 "'%.50s' object has no attribute '%.400s'",
                 tp->tp_name, PyString_AS_STRING(name));
    return NULL;
}

static PyObject *
safe_PyObject_GetAttrString(PyObject *v, const char *name)
{
    PyObject *w, *res;

    if (Py_TYPE(v)->tp_getattr != NULL)
        return (*Py_TYPE(v)->tp_getattr)(v, (char*)name);
    w = PyString_InternFromString(name);
    if (w == NULL)
        return NULL;
    res = safe_PyObject_GetAttr(v, w);
    Py_XDECREF(w);
    return res;
}

static PyObject *
safe__generic_dir(PyObject *obj)
{
    PyObject *result = NULL;
    PyObject *dict = NULL;
    PyObject *itsclass = NULL;

    /* Get __dict__ (which may or may not be a real dict...) */
    dict = safe_PyObject_GetAttrString(obj, "__dict__");
    if (dict == NULL) {
        PyErr_Clear();
        dict = PyDict_New();
    }
    else if (!PyDict_Check(dict)) {
        Py_DECREF(dict);
        dict = PyDict_New();
    }
    else {
        /* Copy __dict__ to avoid mutating it. */
        PyObject *temp = PyDict_Copy(dict);
        Py_DECREF(dict);
        dict = temp;
    }

    if (dict == NULL)
        goto error;

    /* Merge in __members__ and __methods__ (if any).
     * This is removed in Python 3000. */
    if (safe_merge_list_attr(dict, obj, "__members__") < 0)
        goto error;
    if (safe_merge_list_attr(dict, obj, "__methods__") < 0)
        goto error;

    /* Merge in attrs reachable from its class. */
    itsclass = safe_PyObject_GetAttrString(obj, "__class__");
    if (itsclass == NULL)
        /* XXX(tomer): Perhaps fall back to obj->ob_type if no
                       __class__ exists? */
        PyErr_Clear();
    else {
        if (safe_merge_class_dict(dict, itsclass) != 0)
            goto error;
    }

    result = PyDict_Keys(dict);
    /* fall through */
error:
    Py_XDECREF(itsclass);
    Py_XDECREF(dict);
    return result;
}

static PyObject *
safe__specialized_dir_type(PyObject *obj)
{
    PyObject *result = NULL;
    PyObject *dict = PyDict_New();

    if (dict != NULL && safe_merge_class_dict(dict, obj) == 0)
        result = PyDict_Keys(dict);

    Py_XDECREF(dict);
    return result;
}

/* Helper for PyObject_Dir of module objects: returns the module's __dict__. */
static PyObject *
safe__specialized_dir_module(PyObject *obj)
{
    PyObject *result = NULL;
    PyObject *dict = safe_PyObject_GetAttrString(obj, "__dict__");

    if (dict != NULL) {
        if (PyDict_Check(dict))
            result = PyDict_Keys(dict);
        else {
            char *name = PyModule_GetName(obj);
            if (name)
                PyErr_Format(PyExc_TypeError,
                             "%.200s.__dict__ is not a dictionary",
                             name);
        }
    }

    Py_XDECREF(dict);
    return result;
}

static PyObject *
safe__dir_object(PyObject *obj)
{
    PyObject *result = NULL;
    static PyObject *dir_str = NULL;
    PyObject *dirfunc;

    assert(obj);
    if (PyInstance_Check(obj)) {
        dirfunc = safe_PyObject_GetAttrString(obj, "__dir__");
        if (dirfunc == NULL) {
            if (PyErr_ExceptionMatches(PyExc_AttributeError))
                PyErr_Clear();
            else
                return NULL;
        }
    }
    else {
        dirfunc = _PyObject_LookupSpecial(obj, "__dir__", &dir_str);
        if (PyErr_Occurred())
            return NULL;
    }
    if (dirfunc == NULL) {
        /* use default implementation */
        if (PyModule_Check(obj))
            result = safe__specialized_dir_module(obj);
        else if (PyType_Check(obj) || PyClass_Check(obj))
            result = safe__specialized_dir_type(obj);
        else
            result = safe__generic_dir(obj);
    }
    else {
        /* use __dir__ */
        result = PyObject_CallFunctionObjArgs(dirfunc, NULL);
        Py_DECREF(dirfunc);
        if (result == NULL)
            return NULL;

        /* result must be a list */
        /* XXX(gbrandl): could also check if all items are strings */
        if (!PyList_Check(result)) {
            PyErr_Format(PyExc_TypeError,
                         "__dir__() must return a list, not %.200s",
                         Py_TYPE(result)->tp_name);
            Py_DECREF(result);
            result = NULL;
        }
    }

    return result;
}

PyObject* safe_PyObject_Dir(PyObject *obj)
{
    PyObject * result;

#if 0 // don't need to support
    if (obj == NULL)
        /* no object -- introspect the locals */
        result = safe__dir_locals();
    else
#endif
        /* object -- introspect the object */
        result = safe__dir_object(obj);

    assert(result == NULL || PyList_Check(result));

#if 0 // don't need to sort them
    if (result != NULL && PyList_Sort(result) != 0) {
        /* sorting the list failed */
        Py_DECREF(result);
        result = NULL;
    }
#endif

    return result;
}
