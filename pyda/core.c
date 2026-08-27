#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

static SDL_Window* pyda_window = NULL;
static SDL_Surface* pyda_surface = NULL;
static int pyda_is_initialized = 0;

// Timer callback wrapper structure for set_timer
typedef struct {
    int event_type;
} TimerUserData;

static Uint32 pyda_timer_callback(Uint32 interval, void* param) {
    TimerUserData* data = (TimerUserData*)param;
    SDL_Event event;
    SDL_UserEvent userevent;

    userevent.type = data->event_type;
    userevent.code = 0;
    userevent.data1 = NULL;
    userevent.data2 = NULL;

    event.type = data->event_type;
    event.user = userevent;

    SDL_PushEvent(&event);
    return interval; // Return interval to repeat, or 0 to stop
}

// --- 1. BASE SUBSYSTEM ---
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

static PyObject* pyda_get_init(PyObject* self, PyObject* args) {
    if (pyda_is_initialized) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject* pyda_get_error(PyObject* self, PyObject* args) {
    return PyUnicode_FromString(SDL_GetError());
}

static PyObject* pyda_set_error(PyObject* self, PyObject* args) {
    char* err_str;
    if (!PyArg_ParseTuple(args, "s", &err_str)) return NULL;
    SDL_SetError("%s", err_str);
    Py_RETURN_NONE;
}


// --- 2. DISPLAY SUBSYSTEM ---
static PyObject* pyda_set_mode(PyObject* self, PyObject* args) {
    int width, height;
    int flags = SDL_WINDOW_SHOWN;
    
    if (!PyArg_ParseTuple(args, "(ii)|i", &width, &height, &flags)) {
        return NULL;
    }
    
    if (!pyda_is_initialized) {
        PyErr_SetString(PyExc_RuntimeError, "pygame not initialized.");
        return NULL;
    }
    
    if (pyda_window) {
        SDL_DestroyWindow(pyda_window);
        pyda_window = NULL;
        pyda_surface = NULL;
    }

    pyda_window = SDL_CreateWindow("pyda Native Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
    if (!pyda_window) {
        PyErr_Format(PyExc_RuntimeError, "Window creation failed: %s", SDL_GetError());
        return NULL;
    }
    
    pyda_surface = SDL_GetWindowSurface(pyda_window);
    return PyCapsule_New((void*)pyda_surface, "SDL_Surface", NULL);
}

static PyObject* pyda_flip(PyObject* self, PyObject* args) {
    if (pyda_window && pyda_surface) {
        SDL_UpdateWindowSurface(pyda_window);
    }
    Py_RETURN_NONE;
}

static PyObject* pyda_update(PyObject* self, PyObject* args) {
    PyObject* rect_seq = Py_None;
    if (!PyArg_ParseTuple(args, "|O", &rect_seq)) return NULL;

    if (!pyda_window || !pyda_surface) Py_RETURN_NONE;

    if (rect_seq == Py_None) {
        SDL_UpdateWindowSurface(pyda_window);
    } else {
        // Handle list/sequence of Rect objects/tuples for partial updates
        Py_ssize_t size = PySequence_Size(rect_seq);
        if (size > 0) {
            SDL_Rect* rects = (SDL_Rect*)malloc(sizeof(SDL_Rect) * size);
            for (Py_ssize_t i = 0; i < size; i++) {
                PyObject* item = PySequence_GetItem(rect_seq, i);
                int rx, ry, rw, rh;
                // Supports both custom Rect objects (via attributes) or tuples (x, y, w, h)
                if (PyObject_HasAttrString(item, "x")) {
                    rx = (int)PyLong_AsLong(PyObject_GetAttrString(item, "x"));
                    ry = (int)PyLong_AsLong(PyObject_GetAttrString(item, "y"));
                    rw = (int)PyLong_AsLong(PyObject_GetAttrString(item, "width"));
                    rh = (int)PyLong_AsLong(PyObject_GetAttrString(item, "height"));
                } else {
                    PyArg_ParseTuple(item, "iiii", &rx, &ry, &rw, &rh);
                }
                rects[i] = (SDL_Rect){rx, ry, rw, rh};
                Py_DECREF(item);
            }
            SDL_UpdateWindowSurfaceRects(pyda_window, rects, (int)size);
            free(rects);
        } else {
            SDL_UpdateWindowSurface(pyda_window);
        }
    }
    Py_RETURN_NONE;
}

static PyObject* pyda_get_surface(PyObject* self, PyObject* args) {
    if (!pyda_surface) Py_RETURN_NONE;
    return PyCapsule_New((void*)pyda_surface, "SDL_Surface", NULL);
}

static PyObject* pyda_set_caption(PyObject* self, PyObject* args) {
    char *title, *icontitle = NULL;
    if (!PyArg_ParseTuple(args, "s|s", &title, &icontitle)) return NULL;
    if (pyda_window) {
        SDL_SetWindowTitle(pyda_window, title);
    }
    Py_RETURN_NONE;
}

static PyObject* pyda_get_caption(PyObject* self, PyObject* args) {
    if (pyda_window) {
        return PyUnicode_FromString(SDL_GetWindowTitle(pyda_window));
    }
    return PyUnicode_FromString("");
}

static PyObject* pyda_set_icon(PyObject* self, PyObject* args) {
    PyObject* surf_capsule;
    if (!PyArg_ParseTuple(args, "O", &surf_capsule)) return NULL;
    SDL_Surface* icon = (SDL_Surface*)PyCapsule_GetPointer(surf_capsule, "SDL_Surface");
    if (icon && pyda_window) {
        SDL_SetWindowIcon(pyda_window, icon);
    }
    Py_RETURN_NONE;
}

static PyObject* pyda_toggle_fullscreen(PyObject* self, PyObject* args) {
    if (pyda_window) {
        Uint32 flags = SDL_GetWindowFlags(pyda_window);
        Uint32 is_full = (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP;
        SDL_SetWindowFullscreen(pyda_window, is_full);
        return PyLong_FromLong(is_full ? 1 : 0);
    }
    return PyLong_FromLong(0);
}

static PyObject* pyda_get_num_displays(PyObject* self, PyObject* args) {
    return PyLong_FromLong(SDL_GetNumVideoDisplays());
}


// --- 3. EVENT SUBSYSTEM ---
static PyObject* event_to_dict(SDL_Event* event) {
    PyObject* dict = PyDict_New();
    if (!dict) return NULL;

    PyDict_SetItemString(dict, "type", PyLong_FromLong(event->type));

    if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
        PyDict_SetItemString(dict, "key", PyLong_FromLong(event->key.keysym.sym));
        PyDict_SetItemString(dict, "mod", PyLong_FromLong(event->key.keysym.mod));
    } else if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP) {
        PyDict_SetItemString(dict, "button", PyLong_FromLong(event->button.button));
        PyObject* pos = Py_BuildValue("(ii)", event->button.x, event->button.y);
        PyDict_SetItemString(dict, "pos", pos);
        Py_DECREF(pos);
    } else if (event->type == SDL_MOUSEMOTION) {
        PyObject* pos = Py_BuildValue("(ii)", event->motion.x, event->motion.y);
        PyObject* rel = Py_BuildValue("(ii)", event->motion.xrel, event->motion.yrel);
        PyDict_SetItemString(dict, "pos", pos);
        PyDict_SetItemString(dict, "rel", rel);
        Py_DECREF(pos);
        Py_DECREF(rel);
    } else if (event->type == SDL_WINDOWEVENT) {
        PyDict_SetItemString(dict, "event", PyLong_FromLong(event->window.event));
    }
    return dict;
}

static PyObject* pyda_pump_events(PyObject* self, PyObject* args) {
    SDL_PumpEvents();
    Py_RETURN_NONE;
}

static PyObject* pyda_get_events(PyObject* self, PyObject* args) {
    int event_type_filter = -1;
    PyArg_ParseTuple(args, "|i", &event_type_filter);

    SDL_Event event;
    PyObject* event_list = PyList_New(0);

    // Pump first to update internal state
    SDL_PumpEvents();

    while (SDL_PollEvent(&event)) {
        if (event_type_filter == -1 || event.type == (Uint32)event_type_filter) {
            PyObject* dict = event_to_dict(&event);
            if (dict) {
                PyList_Append(event_list, dict);
                Py_DECREF(dict);
            }
        } else {
            // Put un-matched events back or handle filtering if needed
        }
    }
    return event_list;
}

static PyObject* pyda_poll_event(PyObject* self, PyObject* args) {
    SDL_Event event;
    SDL_PumpEvents();
    if (SDL_PollEvent(&event)) {
        return event_to_dict(&event);
    }
    // Return empty dict or None when no events
    Py_RETURN_NONE;
}

static PyObject* pyda_wait_event(PyObject* self, PyObject* args) {
    SDL_Event event;
    if (SDL_WaitEvent(&event)) {
        return event_to_dict(&event);
    }
    Py_RETURN_NONE;
}

static PyObject* pyda_clear_events(PyObject* self, PyObject* args) {
    int event_type_filter = -1;
    PyArg_ParseTuple(args, "|i", &event_type_filter);
    
    SDL_Event event;
    if (event_type_filter == -1) {
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    } else {
        SDL_FlushEvent((Uint32)event_type_filter);
    }
    Py_RETURN_NONE;
}

static PyObject* pyda_post_event(PyObject* self, PyObject* args) {
    int type;
    if (!PyArg_ParseTuple(args, "i", &type)) return NULL;

    SDL_Event event;
    SDL_memset(&event, 0, sizeof(event));
    event.type = (Uint32)type;
    
    int success = SDL_PushEvent(&event);
    return PyBool_FromLong(success >= 0);
}


// --- 4. TIME SUBSYSTEM ---
static PyObject* pyda_get_ticks(PyObject* self, PyObject* args) {
    return PyLong_FromUnsignedLong(SDL_GetTicks());
}

static PyObject* pyda_wait_time(PyObject* self, PyObject* args) {
    unsigned int ms;
    if (!PyArg_ParseTuple(args, "I", &ms)) return NULL;
    // SDL_Delay uses CPU yielding/sleep
    SDL_Delay(ms);
    return PyLong_FromUnsignedLong(SDL_GetTicks());
}

static PyObject* pyda_delay(PyObject* self, PyObject* args) {
    unsigned int ms;
    if (!PyArg_ParseTuple(args, "I", &ms)) return NULL;
    SDL_Delay(ms);
    Py_RETURN_NONE;
}

static PyObject* pyda_set_timer(PyObject* self, PyObject* args) {
    int event_type;
    unsigned int interval;
    if (!PyArg_ParseTuple(args, "iI", &event_type, &interval)) return NULL;

    if (interval == 0) {
        SDL_RemoveTimer(event_type); // Using type ID as pseudo-timer lookup or pass ID tracking
    } else {
        TimerUserData* data = (TimerUserData*)malloc(sizeof(TimerUserData));
        data->event_type = event_type;
        SDL_AddTimer(interval, pyda_timer_callback, data);
    }
    Py_RETURN_NONE;
}


// --- METHOD DEFINITIONS TABLE ---
static PyMethodDef CoreMethods[] = {
    // Base
    {"init", pyda_init, METH_NOARGS, "Initialize modules"},
    {"quit", pyda_quit, METH_NOARGS, "Uninitialize modules"},
    {"get_init", pyda_get_init, METH_NOARGS, "Check if initialized"},
    {"get_error", pyda_get_error, METH_NOARGS, "Get SDL error string"},
    {"set_error", pyda_set_error, METH_VARARGS, "Set SDL error string"},

    // Display
    {"set_mode", pyda_set_mode, METH_VARARGS, "Initialize window surface"},
    {"flip", pyda_flip, METH_NOARGS, "Update full display surface"},
    {"update", pyda_update, METH_VARARGS, "Update display rectangle areas"},
    {"get_surface", pyda_get_surface, METH_NOARGS, "Get current display surface"},
    {"set_caption", pyda_set_caption, METH_VARARGS, "Set window title"},
    {"get_caption", pyda_get_caption, METH_NOARGS, "Get window title"},
    {"set_icon", pyda_set_icon, METH_VARARGS, "Set window taskbar icon"},
    {"toggle_fullscreen", pyda_toggle_fullscreen, METH_NOARGS, "Toggle fullscreen mode"},
    {"get_num_displays", pyda_get_num_displays, METH_NOARGS, "Get display count"},

    // Event
    {"pump_events", pyda_pump_events, METH_NOARGS, "Internal event pump"},
    {"get_events", pyda_get_events, METH_VARARGS, "Get event queue items"},
    {"poll_event", pyda_poll_event, METH_NOARGS, "Poll single event"},
    {"wait_event", pyda_wait_event, METH_NOARGS, "Wait blocking for event"},
    {"clear_events", pyda_clear_events, METH_VARARGS, "Clear event queue"},
    {"post_event", pyda_post_event, METH_VARARGS, "Post custom event"},

    // Time
    {"get_ticks", pyda_get_ticks, METH_NOARGS, "Get elapsed milliseconds"},
    {"wait_time", pyda_wait_time, METH_VARARGS, "Pause with delay"},
    {"delay", pyda_delay, METH_VARARGS, "Precise delay"},
    {"set_timer", pyda_set_timer, METH_VARARGS, "Set periodic timer event"},

    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef core_module = {
    PyModuleDef_HEAD_INIT, 
    "_core", 
    "Complete C core module for pyda", 
    -1, 
    CoreMethods
};

PyMODINIT_FUNC PyInit__core(void) {
    return PyModule_Create(&core_module);
}
