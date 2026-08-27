#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Capsule destructor for safe memory cleanup of TTF_Font
static void free_ttf_font(PyObject* capsule) {
    TTF_Font* font = (TTF_Font*)PyCapsule_GetPointer(capsule, "TTF_Font");
    if (font) {
        TTF_CloseFont(font);
    }
}

// Capsule destructor for safe memory cleanup of rendered SDL_Surface
static void free_sdl_surface(PyObject* capsule) {
    SDL_Surface* surf = (SDL_Surface*)PyCapsule_GetPointer(capsule, "SDL_Surface");
    if (surf) {
        SDL_FreeSurface(surf);
    }
}

// Helper to parse color argument (supports tuples, lists, or packed hex integers)
static int parse_color(PyObject* obj, SDL_Color* color) {
    if (PyTuple_Check(obj) || PyList_Check(obj)) {
        Py_ssize_t size = PySequence_Size(obj);
        if (size < 3) {
            PyErr_SetString(PyExc_ValueError, "Color sequence must have at least 3 elements (R, G, B)");
            return 0;
        }
        {
            PyObject *tmp;
            tmp = PySequence_GetItem(obj, 0); color->r = (Uint8)PyLong_AsLong(tmp); Py_XDECREF(tmp);
            tmp = PySequence_GetItem(obj, 1); color->g = (Uint8)PyLong_AsLong(tmp); Py_XDECREF(tmp);
            tmp = PySequence_GetItem(obj, 2); color->b = (Uint8)PyLong_AsLong(tmp); Py_XDECREF(tmp);
            if (size >= 4) {
                tmp = PySequence_GetItem(obj, 3); color->a = (Uint8)PyLong_AsLong(tmp); Py_XDECREF(tmp);
            } else {
                color->a = 255;
            }
        }
        return 1;
    } else if (PyLong_Check(obj)) {
        unsigned long val = PyLong_AsUnsignedLong(obj);
        color->r = (Uint8)((val >> 16) & 0xFF);
        color->g = (Uint8)((val >> 8) & 0xFF);
        color->b = (Uint8)(val & 0xFF);
        color->a = 255;
        return 1;
    }
    PyErr_SetString(PyExc_TypeError, "Invalid color format");
    return 0;
}

// --- 1. INITIALIZE TTF SUBSYSTEM ---
static PyObject* pyda_font_init(PyObject* self, PyObject* args) {
    if (TTF_WasInit() == 0) {
        if (TTF_Init() < 0) {
            PyErr_Format(PyExc_RuntimeError, "TTF_Init failed: %s", TTF_GetError());
            return NULL;
        }
    }
    Py_RETURN_TRUE;
}

static PyObject* pyda_font_quit(PyObject* self, PyObject* args) {
    if (TTF_WasInit()) {
        TTF_Quit();
    }
    Py_RETURN_NONE;
}

// --- 2. LOAD FONT FILE ---
static PyObject* pyda_font_load(PyObject* self, PyObject* args) {
    char* filepath;
    int ptsize;
    if (!PyArg_ParseTuple(args, "si", &filepath, &ptsize)) return NULL;

    if (TTF_WasInit() == 0 && TTF_Init() < 0) {
        PyErr_Format(PyExc_RuntimeError, "TTF not initialized: %s", TTF_GetError());
        return NULL;
    }

    TTF_Font* font = TTF_OpenFont(filepath, ptsize);
    if (!font) {
        PyErr_Format(PyExc_RuntimeError, "Failed to load font from '%s': %s", filepath, TTF_GetError());
        return NULL;
    }
    return PyCapsule_New((void*)font, "TTF_Font", free_ttf_font);
}

// --- 3. RENDER TEXT (Supports Blended, Solid, and Shaded modes) ---
static PyObject* pyda_font_render(PyObject* self, PyObject* args) {
    PyObject* font_capsule;
    char* text;
    PyObject* color_obj;
    char* mode = "blended"; // Default to high-quality anti-aliased blended mode
    PyObject* bg_color_obj = NULL;

    if (!PyArg_ParseTuple(args, "OsO|sz", &font_capsule, &text, &color_obj, &mode, &bg_color_obj)) {
        return NULL;
    }

    TTF_Font* font = (TTF_Font*)PyCapsule_GetPointer(font_capsule, "TTF_Font");
    if (!font) return NULL;

    SDL_Color fg_color;
    if (!parse_color(color_obj, &fg_color)) return NULL;

    SDL_Surface* surf = NULL;

    if (mode && strcmp(mode, "solid") == 0) {
        surf = TTF_RenderText_Solid(font, text, fg_color);
    } else if (mode && strcmp(mode, "shaded") == 0) {
        SDL_Color bg_color = {0, 0, 0, 255};
        if (bg_color_obj && !parse_color(bg_color_obj, &bg_color)) return NULL;
        surf = TTF_RenderText_Shaded(font, text, fg_color, bg_color);
    } else {
        // Default blended
        surf = TTF_RenderText_Blended(font, text, fg_color);
    }

    if (!surf) {
        PyErr_Format(PyExc_RuntimeError, "Failed to render text: %s", TTF_GetError());
        return NULL;
    }

    return PyCapsule_New((void*)surf, "SDL_Surface", free_sdl_surface);
}

// --- 4. METRICS & UTILITIES ---
static PyObject* pyda_font_size(PyObject* self, PyObject* args) {
    PyObject* font_capsule;
    char* text;
    if (!PyArg_ParseTuple(args, "Os", &font_capsule, &text)) return NULL;

    TTF_Font* font = (TTF_Font*)PyCapsule_GetPointer(font_capsule, "TTF_Font");
    if (!font) return NULL;

    int w = 0, h = 0;
    if (TTF_SizeText(font, text, &w, &h) < 0) {
        PyErr_Format(PyExc_RuntimeError, "Failed to get text size: %s", TTF_GetError());
        return NULL;
    }
    return Py_BuildValue("(ii)", w, h);
}

static PyMethodDef FontMethods[] = {
    {"init", pyda_font_init, METH_NOARGS, "Initialize TTF subsystem"},
    {"quit", pyda_font_quit, METH_NOARGS, "Quit TTF subsystem"},
    {"load", pyda_font_load, METH_VARARGS, "Load font file with point size"},
    {"render", pyda_font_render, METH_VARARGS, "Render text to surface (supports blended/solid/shaded)"},
    {"size", pyda_font_size, METH_VARARGS, "Get dimensions of text string without rendering"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef font_module = {
    PyModuleDef_HEAD_INIT, 
    "_font", 
    "Zero-dependency native font extension", 
    -1, 
    FontMethods
};

PyMODINIT_FUNC PyInit__font(void) {
    return PyModule_Create(&font_module);
}
