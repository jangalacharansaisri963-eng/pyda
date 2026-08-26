#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>

// Global Engine State
static SDL_Window* pyda_window = NULL;
static SDL_Surface* pyda_surface = NULL;
static int pyda_is_initialized = 0;

// --- INITIALIZATION & SHUTDOWN ---
static PyObject* pyda_pygame_init(PyObject* self, PyObject* args) {
    if (pyda_is_initialized) {
        Py_RETURN_TRUE;
    }
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        PyErr_Format(PyExc_RuntimeError, "SDL_Init failed: %s", SDL_GetError());
        return NULL;
    }
    
    pyda_is_initialized = 1;
    Py_RETURN_TRUE;
}

static PyObject* pyda_pygame_quit(PyObject* self, PyObject* args) {
    if (pyda_window) {
        SDL_DestroyWindow(pyda_window);
        pyda_window = NULL;
        pyda_surface = NULL;
    }
    if (pyda_is_initialized) {
        SDL_Quit();
        pyda_is_initialized = 0;
    }
    Py_RETURN_NONE;
}

// --- DISPLAY & WINDOW MANAGEMENT ---
static PyObject* pyda_pygame_set_mode(PyObject* self, PyObject* args) {
    int width, height;
    int flags = SDL_WINDOW_SHOWN;

    // Accept width, height, and optional SDL flags
    if (!PyArg_ParseTuple(args, "(ii)|i", &width, &height, &flags)) {
        return NULL;
    }

    if (!pyda_is_initialized) {
        PyErr_SetString(PyExc_RuntimeError, "pyda.pygame not initialized. Call pyda.pygame.init() first.");
        return NULL;
    }

    if (pyda_window) {
        SDL_DestroyWindow(pyda_window);
    }

    pyda_window = SDL_CreateWindow(
        "pyda High-Performance Engine",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        flags
    );

    if (!pyda_window) {
        PyErr_Format(PyExc_RuntimeError, "Failed to create SDL Window: %s", SDL_GetError());
        return NULL;
    }

    pyda_surface = SDL_GetWindowSurface(pyda_window);
    if (!pyda_surface) {
        PyErr_Format(PyExc_RuntimeError, "Failed to get window surface: %s", SDL_GetError());
        return NULL;
    }

    // Return a capsule wrapping the surface pointer for zero-copy memory access
    return PyCapsule_New((void*)pyda_surface, "SDL_Surface", NULL);
}

static PyObject* pyda_pygame_flip(PyObject* self, PyObject* args) {
    if (pyda_window && pyda_surface) {
        if (SDL_UpdateWindowSurface(pyda_window) < 0) {
            PyErr_Format(PyExc_RuntimeError, "SDL_UpdateWindowSurface failed: %s", SDL_GetError());
            return NULL;
        }
    }
    Py_RETURN_NONE;
}

// --- BLITTING & PIXEL CRUNCHING (Blazing Fast C-Level Blit Stub) ---
static PyObject* pyda_pygame_fill(PyObject* self, PyObject* args) {
    unsigned int color;
    if (!PyArg_ParseTuple(args, "I", &color)) {
        return NULL;
    }

    if (!pyda_surface) {
        PyErr_SetString(PyExc_RuntimeError, "No active display surface. Call set_mode first.");
        return NULL;
    }

    // Direct memory fill using SDL's optimized routines
    Uint32 mapped_color = SDL_MapRGB(pyda_surface->format, 
                                     (color >> 16) & 0xFF, 
                                     (color >> 8) & 0xFF, 
                                     color & 0xFF);
    
    SDL_FillRect(pyda_surface, NULL, mapped_color);
    Py_RETURN_NONE;
}

// --- METHOD MAPPING TABLE ---
static PyMethodDef PydaPygameMethods[] = {
    {"init", pyda_pygame_init, METH_NOARGS, "Initialize high-performance SDL2 video/audio."},
    {"quit", pyda_pygame_quit, METH_NOARGS, "Clean up and shutdown SDL2 subsystems."},
    {"set_mode", pyda_pygame_set_mode, METH_VARARGS, "Create window and return raw surface capsule."},
    {"flip", pyda_pygame_flip, METH_NOARGS, "Blit the backbuffer to screen instantly."},
    {"fill", pyda_pygame_fill, METH_VARARGS, "Fast screen background color fill."},
    {NULL, NULL, 0, NULL}
};

// --- MODULE DEFINITION ---
static struct PyModuleDef pyda_pygame_core_module = {
    PyModuleDef_HEAD_INIT,
    "_core",
    "High-performance C backend for pyda.pygame",
    -1,
    PydaPygameMethods
};

PyMODINIT_FUNC PyInit__core(void) {
    return PyModule_Create(&pyda_pygame_core_module);
}
