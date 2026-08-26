#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>

// --- SURFACE SCALING ---
static PyObject* pyda_transform_scale(PyObject* self, PyObject* args) {
    PyObject* surf_capsule;
    int new_w, new_h;

    if (!PyArg_ParseTuple(args, "O(ii)", &surf_capsule, &new_w, &new_h)) {
        return NULL;
    }

    SDL_Surface* src = (SDL_Surface*)PyCapsule_GetPointer(surf_capsule, "SDL_Surface");
    if (!src) return NULL;

    if (new_w <= 0 || new_h <= 0) {
        PyErr_SetString(PyExc_ValueError, "New width and height must be greater than 0");
        return NULL;
    }

    // Create a temporary surface matching the format for software scaling
    SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(0, new_w, new_h, src->format->BitsPerPixel, src->format->format);
    if (!dst) {
        PyErr_Format(PyExc_RuntimeError, "Failed to create destination surface: %s", SDL_GetError());
        return NULL;
    }

    // Perform smooth scaling or fast blit scaling
    SDL_Rect dst_rect = {0, 0, new_w, new_h};
    // Note: For advanced smooth scaling SDL_BlitScaled requires a software fallback or SDL_image/SDL2_gfx, 
    // but standard SDL_BlitScaled works wonderfully when surfaces share formats.
    if (SDL_BlitScaled(src, NULL, dst, &dst_rect) < 0) {
        SDL_FreeSurface(dst);
        PyErr_Format(PyExc_RuntimeError, "SDL_BlitScaled failed: %s", SDL_GetError());
        return NULL;
    }

    return PyCapsule_New((void*)dst, "SDL_Surface", NULL);
}

// --- METHOD TABLE ---
static PyMethodDef TransformMethods[] = {
    {"scale", pyda_transform_scale, METH_VARARGS, "Scale a surface to new dimensions (width, height)"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef transform_module = {
    PyModuleDef_HEAD_INIT,
    "_transform",
    "High-performance C backend for pyda.pygame.transform",
    -1,
    TransformMethods
};

PyMODINIT_FUNC PyInit__transform(void) {
    return PyModule_Create(&transform_module);
}
