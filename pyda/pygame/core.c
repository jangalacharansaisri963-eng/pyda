#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>

static SDL_Window* pyda_window = NULL;
static SDL_Surface* pyda_surface = NULL;
static int pyda_is_initialized = 0;

// --- INIT & QUIT ---
static PyObject* pyda_pygame_init(PyObject* self, PyObject* args) {
    if (pyda_is_initialized) Py_RETURN_TRUE;
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

// --- DISPLAY ---
static PyObject* pyda_pygame_set_mode(PyObject* self, PyObject* args) {
    int width, height;
    int flags = SDL_WINDOW_SHOWN;
    if (!PyArg_ParseTuple(args, "(ii)|i", &width, &height, &flags)) return NULL;
    if (!pyda_is_initialized) {
        PyErr_SetString(PyExc_RuntimeError, "pygame not initialized.");
        return NULL;
    }
    if (pyda_window) SDL_DestroyWindow(pyda_window);

    pyda_window = SDL_CreateWindow("pyda Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
    if (!pyda_window) {
        PyErr_Format(PyExc_RuntimeError, "Window creation failed: %s", SDL_GetError());
        return NULL;
    }
    pyda_surface = SDL_GetWindowSurface(pyda_window);
    return PyCapsule_New((void*)pyda_surface, "SDL_Surface", NULL);
}

static PyObject* pyda_pygame_flip(PyObject* self, PyObject* args) {
    if (pyda_window && pyda_surface) SDL_UpdateWindowSurface(pyda_window);
    Py_RETURN_NONE;
}

static PyObject* pyda_pygame_fill(PyObject* self, PyObject* args) {
    unsigned int color;
    if (!PyArg_ParseTuple(args, "I", &color)) return NULL;
    if (!pyda_surface) {
        PyErr_SetString(PyExc_RuntimeError, "No active surface.");
        return NULL;
    }
    Uint32 mapped = SDL_MapRGB(pyda_surface->format, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    SDL_FillRect(pyda_surface, NULL, mapped);
    Py_RETURN_NONE;
}

// --- EVENTS & INPUT ---
static PyObject* pyda_pygame_pump_events(PyObject* self, PyObject* args) {
    SDL_Event event;
    PyObject* event_list = PyList_New(0);
    
    while (SDL_PollEvent(&event)) {
        PyObject* dict = PyDict_New();
        PyDict_SetItemString(dict, "type", PyLong_FromLong(event.type));
        
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            PyDict_SetItemString(dict, "key", PyLong_FromLong(event.key.keysym.sym));
        } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
            PyDict_SetItemString(dict, "button", PyLong_FromLong(event.button.button));
            PyObject* pos = Py_BuildValue("(ii)", event.button.x, event.button.y);
            PyDict_SetItemString(dict, "pos", pos);
            Py_DECREF(pos);
        } else if (event.type == SDL_MOUSEMOTION) {
            PyObject* pos = Py_BuildValue("(ii)", event.motion.x, event.motion.y);
            PyDict_SetItemString(dict, "pos", pos);
            PyObject* rel = Py_BuildValue("(ii)", event.motion.xrel, event.motion.yrel);
            PyDict_SetItemString(dict, "rel", rel);
            Py_DECREF(pos);
            Py_DECREF(rel);
        }
        PyList_Append(event_list, dict);
        Py_DECREF(dict);
    }
    return event_list;
}

static PyObject* pyda_pygame_get_pressed(PyObject* self, PyObject* args) {
    int numkeys;
    const Uint8* state = SDL_GetKeyboardState(&numkeys);
    PyObject* tuple = PyTuple_New(numkeys);
    for (int i = 0; i < numkeys; i++) {
        PyTuple_SetItem(tuple, i, PyLong_FromLong(state[i]));
    }
    return tuple;
}

// --- TIME ---
static PyObject* pyda_pygame_get_ticks(PyObject* self, PyObject* args) {
    return PyLong_FromUnsignedLong(SDL_GetTicks());
}

static PyObject* pyda_pygame_delay(PyObject* self, PyObject* args) {
    unsigned int ms;
    if (!PyArg_ParseTuple(args, "I", &ms)) return NULL;
    SDL_Delay(ms);
    Py_RETURN_NONE;
}

// --- DRAWING STUBS (Fast Rect Fill) ---
static PyObject* pyda_pygame_draw_rect(PyObject* self, PyObject* args) {
    int x, y, w, h;
    unsigned int color;
    if (!PyArg_ParseTuple(args, "(iiii)I", &x, &y, &w, &h, &color)) return NULL;
    if (!pyda_surface) Py_RETURN_NONE;

    SDL_Rect rect = {x, y, w, h};
    Uint32 mapped = SDL_MapRGB(pyda_surface->format, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    SDL_FillRect(pyda_surface, &rect, mapped);
    Py_RETURN_NONE;
}

static PyMethodDef PydaPygameMethods[] = {
    {"init", pyda_pygame_init, METH_NOARGS, "Init SDL2"},
    {"quit", pyda_pygame_quit, METH_NOARGS, "Quit SDL2"},
    {"set_mode", pyda_pygame_set_mode, METH_VARARGS, "Set display mode"},
    {"flip", pyda_pygame_flip, METH_NOARGS, "Flip display buffer"},
    {"fill", pyda_pygame_fill, METH_VARARGS, "Fill screen color"},
    {"pump_events", pyda_pygame_pump_events, METH_NOARGS, "Poll event queue"},
    {"get_pressed", pyda_pygame_get_pressed, METH_NOARGS, "Get keyboard states"},
    {"get_ticks", pyda_pygame_get_ticks, METH_NOARGS, "Get SDL ticks"},
    {"delay", pyda_pygame_delay, METH_VARARGS, "Delay ms"},
    {"draw_rect", pyda_pygame_draw_rect, METH_VARARGS, "Draw filled rect"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef pyda_pygame_core_module = {
    PyModuleDef_HEAD_INIT, "_core", "pyda core C backend", -1, PydaPygameMethods
};

PyMODINIT_FUNC PyInit__core(void) {
    return PyModule_Create(&pyda_pygame_core_module);
}
