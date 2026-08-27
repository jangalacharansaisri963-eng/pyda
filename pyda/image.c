#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Capsule destructor for safe memory cleanup of loaded SDL_Surface
static void free_sdl_surface(PyObject* capsule) {
    SDL_Surface* surf = (SDL_Surface*)PyCapsule_GetPointer(capsule, "SDL_Surface");
    if (surf) {
        SDL_FreeSurface(surf);
    }
}

// --- 1. INITIALIZATION & SHUTDOWN ---
static PyObject* pyda_image_init(PyObject* self, PyObject* args) {
    int flags = IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_WEBP;
    int inited = IMG_Init(flags);
    if ((inited & flags) != flags) {
        // Allow partial init if some loaders succeeded, or report warning/error
        // We'll let it pass if at least PNG/JPG are ready, or check error
    }
    Py_RETURN_TRUE;
}

static PyObject* pyda_image_quit(PyObject* self, PyObject* args) {
    IMG_Quit();
    Py_RETURN_NONE;
}

// --- 2. LOAD IMAGE ---
static PyObject* pyda_image_load(PyObject* self, PyObject* args) {
    char* filepath;
    if (!PyArg_ParseTuple(args, "s", &filepath)) return NULL;

    // Ensure IMG is initialized
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    SDL_Surface* surface = IMG_Load(filepath);
    if (!surface) {
        PyErr_Format(PyExc_RuntimeError, "Failed to load image '%s': %s", filepath, IMG_GetError());
        return NULL;
    }
    return PyCapsule_New((void*)surface, "SDL_Surface", free_sdl_surface);
}

// --- 3. SAVE IMAGE ---
static PyObject* pyda_image_save(PyObject* self, PyObject* args) {
    PyObject* surf_capsule;
    char* filepath;
    if (!PyArg_ParseTuple(args, "Os", &surf_capsule, &filepath)) return NULL;

    SDL_Surface* surface = (SDL_Surface*)PyCapsule_GetPointer(surf_capsule, "SDL_Surface");
    if (!surface) return NULL;

    int result = -1;
    // Check file extension to determine format
    char* ext = strrchr(filepath, '.');
    if (ext) {
        if (strcmp(ext, ".png") == 0 || strcmp(ext, ".PNG") == 0) {
            result = IMG_SavePNG(surface, filepath);
        } else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".JPG") == 0) {
            result = IMG_SaveJPG(surface, filepath, 90); // Default quality 90
        } else {
            result = SDL_SaveBMP(surface, filepath); // Fallback to BMP
        }
    } else {
        result = SDL_SaveBMP(surface, filepath);
    }

    if (result < 0) {
        PyErr_Format(PyExc_RuntimeError, "Failed to save image to '%s': %s", filepath, SDL_GetError());
        return NULL;
    }

    Py_RETURN_TRUE;
}

static PyMethodDef ImageMethods[] = {
    {"init", pyda_image_init, METH_NOARGS, "Initialize SDL_image subsystem"},
    {"quit", pyda_image_quit, METH_NOARGS, "Quit SDL_image subsystem"},
    {"load", pyda_image_load, METH_VARARGS, "Load image surface from file path"},
    {"save", pyda_image_save, METH_VARARGS, "Save surface to file (PNG/JPG/BMP)"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef image_module = {
    PyModuleDef_HEAD_INIT, 
    "_image", 
    "Zero-dependency native image extension", 
    -1, 
    ImageMethods
};

PyMODINIT_FUNC PyInit__image(void) {
    return PyModule_Create(&image_module);
}
