#ifndef DIDYOUMEAN_SAFE_H
#define DIDYOUMEAN_SAFE_H
PyObject* safe_PyObject_Dir(PyObject *obj);
PyObject * safe_builtin_getattr(PyObject *self, PyObject *args);
#endif
