#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>

static PyObject* pyda_transform_scale(PyObject* self, PyObject* args) {
    PyObject* surf_capsule;
    int w, h;
    if (!PyArg_ParseTuple(args, "O(ii)", &surf_capsule, &w, &h)) return NULL;

    SDL_Surface* src = (SDL_Surface*)PyCapsule_GetPointer(surf_capsule, "SDL_Surface");
    if (!src) return NULL;

    SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(0, w, h, src->format->BitsPerPixel, src->format->format);
    if (!dst) return NULL;

    SDL_Rect rect = {0, 0, w, h};
    SDL_BlitScaled(src, NULL, dst, &rect);

    return PyCapsule_New((void*)dst, "SDL_Surface", NULL);
}

static PyMethodDef TransformMethods[] = {
    {"scale", pyda_transform_scale, METH_VARARGS, "Scale surface"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef transform_module = {
    PyModuleDef_HEAD_INIT, "_transform", "C extension for transforms", -1, TransformMethods
};

PyMODINIT_FUNC PyInit__transform(void) {
    return PyModule_Create(&transform_module);
}
