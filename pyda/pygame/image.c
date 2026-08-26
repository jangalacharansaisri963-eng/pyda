#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

static PyObject* pyda_image_load(PyObject* self, PyObject* args) {
    char* filepath;
    if (!PyArg_ParseTuple(args, "s", &filepath)) return NULL;

    SDL_Surface* surface = IMG_Load(filepath);
    if (!surface) {
        PyErr_Format(PyExc_RuntimeError, "Failed to load image '%s': %s", filepath, IMG_GetError());
        return NULL;
    }
    return PyCapsule_New((void*)surface, "SDL_Surface", NULL);
}

static PyMethodDef ImageMethods[] = {
    {"load", pyda_image_load, METH_VARARGS, "Load image surface"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef image_module = {
    PyModuleDef_HEAD_INIT, "_image", "C extension for images", -1, ImageMethods
};

PyMODINIT_FUNC PyInit__image(void) {
    return PyModule_Create(&image_module);
}
