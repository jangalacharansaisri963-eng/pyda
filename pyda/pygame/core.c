#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>

static SDL_Window* pyda_window = NULL;
static SDL_Surface* pyda_surface = NULL;
static int pyda_is_initialized = 0;

// --- INIT / QUIT ---
static PyObject* pyda_init(PyObject* self, PyObject* args) {
    if (pyda_is_initialized) Py_RETURN_TRUE;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        PyErr_Format(PyExc_RuntimeError, "SDL_Init failed: %s", SDL_GetError());
        return NULL;
    }
    pyda_is_initialized = 1;
    Py_RETURN_TRUE;
}

static PyObject* pyda_quit(PyObject* self, PyObject* args) {
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
static PyObject* pyda_set_mode(PyObject* self, PyObject* args) {
    int width, height;
    int flags = SDL_WINDOW_SHOWN;
    if (!PyArg_ParseTuple(args, "(ii)|i", &width, &height, &flags)) return NULL;
    
    if (!pyda_is_initialized) {
        PyErr_SetString(PyExc_RuntimeError, "pygame not initialized.");
        return NULL;
    }
    if (pyda_window) SDL_DestroyWindow(pyda_window);

    pyda_window = SDL_CreateWindow("pyda Native Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
    if (!pyda_window) {
        PyErr_Format(PyExc_RuntimeError, "Window creation failed: %s", SDL_GetError());
        return NULL;
    }
    pyda_surface = SDL_GetWindowSurface(pyda_window);
    return PyCapsule_New((void*)pyda_surface, "SDL_Surface", NULL);
}

static PyObject* pyda_flip(PyObject* self, PyObject* args) {
    if (pyda_window && pyda_surface) SDL_UpdateWindowSurface(pyda_window);
    Py_RETURN_NONE;
}

// --- EVENTS ---
static PyObject* pyda_pump_events(PyObject* self, PyObject* args) {
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
        }
        PyList_Append(event_list, dict);
        Py_DECREF(dict);
    }
    return event_list;
}

// --- TIME ---
static PyObject* pyda_get_ticks(PyObject* self, PyObject* args) {
    return PyLong_FromUnsignedLong(SDL_GetTicks());
}

static PyObject* pyda_delay(PyObject* self, PyObject* args) {
    unsigned int ms;
    if (!PyArg_ParseTuple(args, "I", &ms)) return NULL;
    SDL_Delay(ms);
    Py_RETURN_NONE;
}

static PyMethodDef CoreMethods[] = {
    {"init", pyda_init, METH_NOARGS, "Initialize SDL2"},
    {"quit", pyda_quit, METH_NOARGS, "Quit SDL2"},
    {"set_mode", pyda_set_mode, METH_VARARGS, "Create window surface"},
    {"flip", pyda_flip, METH_NOARGS, "Update window surface buffer"},
    {"pump_events", pyda_pump_events, METH_NOARGS, "Poll event queue"},
    {"get_ticks", pyda_get_ticks, METH_NOARGS, "Get SDL ticks"},
    {"delay", pyda_delay, METH_VARARGS, "Delay execution in ms"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef core_module = {
    PyModuleDef_HEAD_INIT, "_core", "pyda native C core", -1, CoreMethods
};

PyMODINIT_FUNC PyInit__core(void) {
    return PyModule_Create(&core_module);
}
