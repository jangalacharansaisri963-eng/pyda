#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>

static PyObject* key_get_pressed(PyObject* self, PyObject* args) {
    int numkeys;
    const Uint8* state = SDL_GetKeyboardState(&numkeys);
    PyObject* tuple = PyTuple_New(numkeys);
    for (int i = 0; i < numkeys; i++) {
        PyTuple_SetItem(tuple, i, PyLong_FromLong(state[i]));
    }
    return tuple;
}

static PyMethodDef KeyMethods[] = {
    {"get_pressed", key_get_pressed, METH_NOARGS, "Get keyboard state"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef key_module = {
    PyModuleDef_HEAD_INIT, "_key", "C extension for keys", -1, KeyMethods
};

PyMODINIT_FUNC PyInit__key(void) {
    return PyModule_Create(&key_module);
}
