#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

typedef struct {
    PyObject_HEAD
    double x, y, w, h;
} PydaRectObject;

static PyObject* rect_colliderect(PydaRectObject* self, PyObject* args) {
    PydaRectObject* other;
    if (!PyArg_ParseTuple(args, "O!", &Py_Type(self), &other)) return NULL;

    int collide = (
        self->x < other->x + other->w &&
        self->x + self->w > other->x &&
        self->y < other->y + other->h &&
        self->y + self->h > other->y
    );

    if (collide) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject* rect_collidepoint(PydaRectObject* self, PyObject* args) {
    double px, py;
    PyObject *arg1, *arg2 = NULL;

    if (!PyArg_ParseTuple(args, "O|O", &arg1, &arg2)) return NULL;

    if (arg2) {
        px = PyFloat_AsDouble(arg1);
        py = PyFloat_AsDouble(arg2);
    } else if (PyTuple_Check(arg1) && PyTuple_Size(arg1) == 2) {
        px = PyFloat_AsDouble(PyTuple_GetItem(arg1, 0));
        py = PyFloat_AsDouble(PyTuple_GetItem(arg1, 1));
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid point coordinates");
        return NULL;
    }

    int collide = (px >= self->x && px < self->x + self->w &&
                   py >= self->y && py < self->y + self->h);
    if (collide) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject* rect_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    PydaRectObject* self = (PydaRectObject*)type->tp_alloc(type, 0);
    if (self) {
        self->x = 0; self->y = 0; self->w = 0; self->h = 0;
    }
    return (PyObject*)self;
}

static int rect_init(PydaRectObject* self, PyObject* args, PyObject* kwds) {
    double x = 0, y = 0, w = 0, h = 0;
    Py_ssize_t argc = PyTuple_Size(args);

    if (argc == 4) {
        if (!PyArg_ParseTuple(args, "dddd", &x, &y, &w, &h)) return -1;
    } else if (argc == 1) {
        PyObject* arg = PyTuple_GetItem(args, 0);
        if (PyTuple_Check(arg) && PyTuple_Size(arg) == 4) {
            x = PyFloat_AsDouble(PyTuple_GetItem(arg, 0));
            y = PyFloat_AsDouble(PyTuple_GetItem(arg, 1));
            w = PyFloat_AsDouble(PyTuple_GetItem(arg, 2));
            h = PyFloat_AsDouble(PyTuple_GetItem(arg, 3));
        }
    } else if (argc == 2) {
        PyObject *pos = PyTuple_GetItem(args, 0);
        PyObject *size = PyTuple_GetItem(args, 1);
        if (PyTuple_Check(pos) && PyTuple_Check(size)) {
            x = PyFloat_AsDouble(PyTuple_GetItem(pos, 0));
            y = PyFloat_AsDouble(PyTuple_GetItem(pos, 1));
            w = PyFloat_AsDouble(PyTuple_GetItem(size, 0));
            h = PyFloat_AsDouble(PyTuple_GetItem(size, 1));
        }
    }

    self->x = x; self->y = y; self->w = w; self->h = h;
    return 0;
}

// --- VIRTUAL GETTERS & SETTERS FOR ADVANCED PROPERTIES ---
static PyObject* rect_get_top(PydaRectObject* self, void* closure) { return PyFloat_FromDouble(self->y); }
static int rect_set_top(PydaRectObject* self, PyObject* value, void* closure) {
    if (!value) return -1;
    self->y = PyFloat_AsDouble(value);
    return 0;
}

static PyObject* rect_get_left(PydaRectObject* self, void* closure) { return PyFloat_FromDouble(self->x); }
static int rect_set_left(PydaRectObject* self, PyObject* value, void* closure) {
    if (!value) return -1;
    self->x = PyFloat_AsDouble(value);
    return 0;
}

static PyObject* rect_get_bottom(PydaRectObject* self, void* closure) { return PyFloat_FromDouble(self->y + self->h); }
static int rect_set_bottom(PydaRectObject* self, PyObject* value, void* closure) {
    if (!value) return -1;
    self->y = PyFloat_AsDouble(value) - self->h;
    return 0;
}

static PyObject* rect_get_right(PydaRectObject* self, void* closure) { return PyFloat_FromDouble(self->x + self->w); }
static int rect_set_right(PydaRectObject* self, PyObject* value, void* closure) {
    if (!value) return -1;
    self->x = PyFloat_AsDouble(value) - self->w;
    return 0;
}

static PyObject* rect_get_center(PydaRectObject* self, void* closure) {
    return Py_BuildValue("(dd)", self->x + (self->w / 2.0), self->y + (self->h / 2.0));
}
static int rect_set_center(PydaRectObject* self, PyObject* value, void* closure) {
    if (!value || !PyTuple_Check(value) || PyTuple_Size(value) != 2) return -1;
    double cx = PyFloat_AsDouble(PyTuple_GetItem(value, 0));
    double cy = PyFloat_AsDouble(PyTuple_GetItem(value, 1));
    self->x = cx - (self->w / 2.0);
    self->y = cy - (self->h / 2.0);
    return 0;
}

static PyMemberDef rect_members[] = {
    {"x", T_DOUBLE, offsetof(PydaRectObject, x), 0, "x position"},
    {"y", T_DOUBLE, offsetof(PydaRectObject, y), 0, "y position"},
    {"width", T_DOUBLE, offsetof(PydaRectObject, w), 0, "width"},
    {"height", T_DOUBLE, offsetof(PydaRectObject, h), 0, "height"},
    {NULL}
};

static PyGetSetDef rect_getset[] = {
    {"top", (getter)rect_get_top, (setter)rect_set_top, "top coordinate", NULL},
    {"left", (getter)rect_get_left, (setter)rect_set_left, "left coordinate", NULL},
    {"bottom", (getter)rect_get_bottom, (setter)rect_set_bottom, "bottom coordinate", NULL},
    {"right", (getter)rect_get_right, (setter)rect_set_right, "right coordinate", NULL},
    {"center", (getter)rect_get_center, (setter)rect_set_center, "center position", NULL},
    {NULL}
};

static PyMethodDef rect_methods[] = {
    {"colliderect", (PyCFunction)rect_colliderect, METH_VARARGS, "Collision check"},
    {"collidepoint", (PyCFunction)rect_collidepoint, METH_VARARGS, "Point collision check"},
    {NULL}
};

static PyTypeObject PydaRectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyda.pygame.rect.Rect",
    .tp_doc = "Advanced native Rect object",
    .tp_basicsize = sizeof(PydaRectObject),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = rect_new,
    .tp_init = (initproc)rect_init,
    .tp_members = rect_members,
    .tp_methods = rect_methods,
    .tp_getset = rect_getset,
};

static PyModuleDef rectmodule = {
    PyModuleDef_HEAD_INIT, "_rect", "C extension for rect", -1, NULL
};

PyMODINIT_FUNC PyInit__rect(void) {
    PyObject* m;
    if (PyType_Ready(&PydaRectType) < 0) return NULL;
    m = PyModule_Create(&rectmodule);
    if (!m) return NULL;
    Py_INCREF(&PydaRectType);
    PyModule_AddObject(m, "Rect", (PyObject*)&PydaRectType);
    return m;
}
