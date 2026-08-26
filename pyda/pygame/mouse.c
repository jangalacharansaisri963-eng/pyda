#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>

static PyObject* mouse_get_pos(PyObject* self, PyObject* args) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    return Py_BuildValue("(ii)", x, y);
}

static PyObject* mouse_get_pressed(PyObject* self, PyObject* args) {
    int x, y;
    Uint32 buttons = SDL_GetMouseState(&x, &y);
    return Py_BuildValue("(iii)", 
        (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) ? 1 : 0,
        (buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) ? 1 : 0,
        (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) ? 1 : 0
    );
}

static PyMethodDef MouseMethods[] = {
    {"get_pos", mouse_get_pos, METH_NOARGS, "Get mouse pos"},
    {"get_pressed", mouse_get_pressed, METH_NOARGS, "Get mouse buttons"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef mouse_module = {
    PyModuleDef_HEAD_INIT, "_mouse", "C extension for mouse", -1, MouseMethods
};

PyMODINIT_FUNC PyInit__mouse(void) {
    return PyModule_Create(&mouse_module);
}
