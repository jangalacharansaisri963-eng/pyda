#define PY_SSIZE_T_CLEAN
#include <Python.h>

// Fast in-place integer array summing (simulating raw C arrays)
static PyObject* ds_fast_sum(PyObject* self, PyObject* args) {
    PyObject *list_obj;
    if (!PyArg_ParseTuple(args, "O", &list_obj)) return NULL;
    
    if (!PyList_Check(list_obj)) {
        PyErr_SetString(PyExc_TypeError, "Expected a Python list");
        return NULL;
    }
    
    Py_ssize_t size = PyList_Size(list_obj);
    double total = 0.0;
    
    for (Py_ssize_t i = 0; i < size; i++) {
        PyObject *item = PyList_GetItem(list_obj, i);
        total += PyFloat_AsDouble(item); // Handles ints and floats automatically
    }
    
    return PyFloat_FromDouble(total);
}

// Fast finding of min and max in a list in a single pass
static PyObject* ds_min_max(PyObject* self, PyObject* args) {
    PyObject *list_obj;
    if (!PyArg_ParseTuple(args, "O", &list_obj)) return NULL;
    if (!PyList_Check(list_obj)) return NULL;
    
    Py_ssize_t size = PyList_Size(list_obj);
    if (size == 0) return Py_BuildValue("(dd)", 0.0, 0.0);
    
    double min_val = PyFloat_AsDouble(PyList_GetItem(list_obj, 0));
    double max_val = min_val;
    
    for (Py_ssize_t i = 1; i < size; i++) {
        double val = PyFloat_AsDouble(PyList_GetItem(list_obj, i));
        if (val < min_val) min_val = val;
        if (val > max_val) max_val = val;
    }
    
    return Py_BuildValue("(dd)", min_val, max_val);
}

static PyMethodDef DSMethods[] = {
    {"fast_sum", ds_fast_sum, METH_VARARGS, "Lightning-fast C-level list sum"},
    {"min_max", ds_min_max, METH_VARARGS, "Find min and max in one pass"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef ds_module = {
    PyModuleDef_HEAD_INIT, "_datastruct", "Fast data structure helpers", -1, DSMethods
};

PyMODINIT_FUNC PyInit__datastruct(void) { return PyModule_Create(&ds_module); }
