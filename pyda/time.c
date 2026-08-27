#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>

typedef struct {
    PyObject_HEAD
    Uint32 last_time;
    double framerate;
} PydaClockObject;

static PyObject* clock_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    PydaClockObject* self = (PydaClockObject*)type->tp_alloc(type, 0);
    if (self) {
        self->last_time = SDL_GetTicks();
        self->framerate = 0.0;
    }
    return (PyObject*)self;
}

static PyObject* clock_tick(PydaClockObject* self, PyObject* args) {
    int target_fps = 0;
    if (!PyArg_ParseTuple(args, "|i", &target_fps)) return NULL;

    Uint32 current_time = SDL_GetTicks();
    Uint32 elapsed = current_time - self->last_time;
    self->last_time = current_time;

    if (target_fps > 0) {
        double target_delay = (1000.0 / target_fps) - (double)elapsed;
        if (target_delay > 0) {
            SDL_Delay((Uint32)target_delay);
            current_time = SDL_GetTicks();
            elapsed = current_time - (self->last_time - (Uint32)target_delay);
        }
    }

    if (elapsed > 0) {
        self->framerate = 1000.0 / (double)elapsed;
    } else {
        self->framerate = 0.0;
    }

    return PyLong_FromUnsignedLong(elapsed);
}

static PyObject* clock_get_fps(PydaClockObject* self, PyObject* args) {
    return PyFloat_FromDouble(self->framerate);
}

static PyMethodDef clock_methods[] = {
    {"tick", (PyCFunction)clock_tick, METH_VARARGS, "Tick clock"},
    {"get_fps", (PyCFunction)clock_get_fps, METH_NOARGS, "Get FPS"},
    {NULL}
};

static PyTypeObject PydaClockType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyda.time.Clock",
    .tp_basicsize = sizeof(PydaClockObject),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = clock_new,
    .tp_methods = clock_methods,
};

static PyModuleDef timemodule = {
    PyModuleDef_HEAD_INIT, "_time", "C extension for time", -1, NULL
};

PyMODINIT_FUNC PyInit__time(void) {
    PyObject* m;
    if (PyType_Ready(&PydaClockType) < 0) return NULL;
    m = PyModule_Create(&timemodule);
    if (!m) return NULL;
    Py_INCREF(&PydaClockType);
    PyModule_AddObject(m, "Clock", (PyObject*)&PydaClockType);
    return m;
}
