#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>

static PyObject* draw_rect(PyObject* self, PyObject* args) {
    PyObject* surf_capsule;
    unsigned int color_val;
    int x, y, w, h;

    if (!PyArg_ParseTuple(args, "OI(iiii)", &surf_capsule, &color_val, &x, &y, &w, &h)) {
        return NULL;
    }

    SDL_Surface* surface = (SDL_Surface*)PyCapsule_GetPointer(surf_capsule, "SDL_Surface");
    if (!surface) return NULL;

    SDL_Rect rect = {x, y, w, h};
    Uint32 mapped = SDL_MapRGB(surface->format, (color_val >> 16) & 0xFF, (color_val >> 8) & 0xFF, color_val & 0xFF);
    SDL_FillRect(surface, &rect, mapped);
    
    Py_RETURN_NONE;
}

static PyMethodDef DrawMethods[] = {
    {"rect", draw_rect, METH_VARARGS, "Draw filled rectangle directly on surface"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef draw_module = {
    PyModuleDef_HEAD_INIT, "_draw", "C extension for drawing", -1, DrawMethods
};

PyMODINIT_FUNC PyInit__draw(void) {
    return PyModule_Create(&draw_module);
}
