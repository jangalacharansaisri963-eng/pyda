#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// --- TTF SUBSYSTEM INIT & QUIT ---
static PyObject* pyda_ttf_init(PyObject* self, PyObject* args) {
    if (TTF_Init() < 0) {
        PyErr_Format(PyExc_RuntimeError, "TTF_Init failed: %s", TTF_GetError());
        return NULL;
    }
    Py_RETURN_TRUE;
}

static PyObject* pyda_ttf_quit(PyObject* self, PyObject* args) {
    TTF_Quit();
    Py_RETURN_NONE;
}

// --- FONT OBJECT MANAGEMENT ---
static PyObject* pyda_font_load(PyObject* self, PyObject* args) {
    char* filepath;
    int ptsize;
    if (!PyArg_ParseTuple(args, "si", &filepath, &ptsize)) return NULL;

    TTF_Font* font = TTF_OpenFont(filepath, ptsize);
    if (!font) {
        PyErr_Format(PyExc_RuntimeError, "Failed to load font %s: %s", filepath, TTF_GetError());
        return NULL;
    }
    return PyCapsule_New((void*)font, "TTF_Font", NULL);
}

// --- TEXT RENDERING (Blended Mode for Smooth Anti-aliased Edges) ---
static PyObject* pyda_font_render(PyObject* self, PyObject* args) {
    PyObject* font_capsule;
    char* text;
    int antialias;
    unsigned int color_val;

    if (!PyArg_ParseTuple(args, "OsII", &font_capsule, &text, &antialias, &color_val)) {
        return NULL;
    }

    TTF_Font* font = (TTF_Font*)PyCapsule_GetPointer(font_capsule, "TTF_Font");
    if (!font) return NULL;

    SDL_Color color = {
        (color_val >> 16) & 0xFF,
        (color_val >> 8) & 0xFF,
        color_val & 0xFF,
        255
    };

    SDL_Surface* text_surface = antialias ? TTF_RenderText_Blended(font, text, color) 
                                          : TTF_RenderText_Solid(font, text, color);
    if (!text_surface) {
        PyErr_Format(PyExc_RuntimeError, "Failed to render text: %s", TTF_GetError());
        return NULL;
    }

    // Return the rendered surface as a capsule so the Python Surface class can wrap it
    return PyCapsule_New((void*)text_surface, "SDL_Surface", NULL);
}

// --- METHOD TABLE ---
static PyMethodDef FontMethods[] = {
    {"init", pyda_ttf_init, METH_NOARGS, "Initialize SDL_ttf"},
    {"quit", pyda_ttf_quit, METH_NOARGS, "Quit SDL_ttf"},
    {"load_font", pyda_font_load, METH_VARARGS, "Load a TrueType font file"},
    {"render_text", pyda_font_render, METH_VARARGS, "Render text to a new surface"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef font_module = {
    PyModuleDef_HEAD_INIT,
    "_font",
    "High-performance C backend for pyda.pygame.font",
    -1,
    FontMethods
};

PyMODINIT_FUNC PyInit__font(void) {
    return PyModule_Create(&font_module);
}
