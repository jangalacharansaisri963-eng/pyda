#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

static PyObject* pyda_font_init(PyObject* self, PyObject* args) {
    if (TTF_Init() < 0) {
        PyErr_Format(PyExc_RuntimeError, "TTF_Init failed: %s", TTF_GetError());
        return NULL;
    }
    Py_RETURN_TRUE;
}

static PyObject* pyda_font_load(PyObject* self, PyObject* args) {
    char* filepath;
    int ptsize;
    if (!PyArg_ParseTuple(args, "si", &filepath, &ptsize)) return NULL;
    TTF_Font* font = TTF_OpenFont(filepath, ptsize);
    if (!font) {
        PyErr_Format(PyExc_RuntimeError, "Failed to load font: %s", TTF_GetError());
        return NULL;
    }
    return PyCapsule_New((void*)font, "TTF_Font", NULL);
}

static PyObject* pyda_font_render(PyObject* self, PyObject* args) {
    PyObject* font_capsule;
    char* text;
    unsigned int color_val;
    if (!PyArg_ParseTuple(args, "OsI", &font_capsule, &text, &color_val)) return NULL;

    TTF_Font* font = (TTF_Font*)PyCapsule_GetPointer(font_capsule, "TTF_Font");
    if (!font) return NULL;

    SDL_Color color = {
        (color_val >> 16) & 0xFF,
        (color_val >> 8) & 0xFF,
        color_val & 0xFF,
        255
    };

    SDL_Surface* surf = TTF_RenderText_Blended(font, text, color);
    if (!surf) return NULL;
    return PyCapsule_New((void*)surf, "SDL_Surface", NULL);
}

static PyMethodDef FontMethods[] = {
    {"init", pyda_font_init, METH_NOARGS, "Init TTF"},
    {"load", pyda_font_load, METH_VARARGS, "Load font file"},
    {"render", pyda_font_render, METH_VARARGS, "Render text to surface"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef font_module = {
    PyModuleDef_HEAD_INIT, "_font", "C extension for fonts", -1, FontMethods
};

PyMODINIT_FUNC PyInit__font(void) {
    return PyModule_Create(&font_module);
}
