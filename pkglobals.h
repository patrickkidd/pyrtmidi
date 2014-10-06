#include <Python.h>

#if PY_MAJOR_VERSION >= 3
#define PK_PYTHON3 1
#else
#define PK_PYTHON3 0
#endif

#if PK_PYTHON3
#define PK_BOOL(x) PyBool_FromLong(x)
#define PK_INT(x) PyLong_FromLong(x)
#define PK_FLOAT(x) PyFloat_FromDouble(x)
#define PK_STRING(x) PyUnicode_FromString(x)
#else
#define PK_BOOL(x) PyBool_FromLong(x)
#define PK_INT(x) PyInt_FromLong(x)
#define PK_FLOAT(x) PyFloat_FromDouble(x)
#define PK_STRING(x) PyString_FromString(x)
#endif

#ifndef Py_RETURN_NONE
#define Py_RETURN_NONE Py_INCREF(Py_None); return Py_None;
#endif

#ifdef __APPLE__
#define PK_WINDOWS 0
#endif
#ifdef __LINUX__
#define PK_WINDOWS 0
#endif
#ifdef __WINDOWS__
#define PK_WINDOWS 1
#endif

#if PK_WINDOWS
#include <windows.h>
//typedef void *HANDLE;
#endif
