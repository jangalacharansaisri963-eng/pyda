#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

typedef struct {
    PyObject_HEAD
    double x, y, w, h;
} PydaRectObject;

// Forward declaration
static PyTypeObject PydaRectType;

// --- CUSTOM MATH-FREE HELPERS ---
static inline double pyda_min(double a, double b) {
    return (a < b) ? a : b;
}

static inline double pyda_max(double a, double b) {
    return (a > b) ? a : b;
}

// --- CONSTRUCTOR & INITIALIZER ---
static PyObject* rect_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    PydaRectObject* self = (PydaRectObject*)type->tp_alloc(type, 0);
    if (self) {
        self->x = 0.0; self->y = 0.0; self->w = 0.0; self->h = 0.0;
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
        if (PyObject_TypeCheck(arg, &PydaRectType)) {
            PydaRectObject* other = (PydaRectObject*)arg;
            x = other->x; y = other->y; w = other->w; h = other->h;
        } else if (PySequence_Check(arg) && PySequence_Size(arg) == 4) {
            x = PyFloat_AsDouble(PySequence_GetItem(arg, 0));
            y = PyFloat_AsDouble(PySequence_GetItem(arg, 1));
            w = PyFloat_AsDouble(PySequence_GetItem(arg, 2));
            h = PyFloat_AsDouble(PySequence_GetItem(arg, 3));
        }
    } else if (argc == 2) {
        PyObject *pos = PyTuple_GetItem(args, 0);
        PyObject *size = PyTuple_GetItem(args, 1);
        if (PySequence_Check(pos) && PySequence_Size(pos) == 2 &&
            PySequence_Check(size) && PySequence_Size(size) == 2) {
            x = PyFloat_AsDouble(PySequence_GetItem(pos, 0));
            y = PyFloat_AsDouble(PySequence_GetItem(pos, 1));
            w = PyFloat_AsDouble(PySequence_GetItem(size, 0));
            h = PyFloat_AsDouble(PySequence_GetItem(size, 1));
        }
    }

    self->x = x; self->y = y; self->w = w; self->h = h;
    return 0;
}

// --- REPRESENTATION ---
static PyObject* rect_repr(PydaRectObject* self) {
    return PyUnicode_FromFormat("<Rect(%.1f, %.1f, %.1f, %.1f)>", self->x, self->y, self->w, self->h);
}

// --- VIRTUAL GETTERS & SETTERS (Full Pygame Geometry Suite) ---
static PyObject* rect_get_top(PydaRectObject* self, void* c) { return PyFloat_FromDouble(self->y); }
static int rect_set_top(PydaRectObject* self, PyObject* v, void* c) { if(!v)return-1; self->y = PyFloat_AsDouble(v); return 0; }

static PyObject* rect_get_left(PydaRectObject* self, void* c) { return PyFloat_FromDouble(self->x); }
static int rect_set_left(PydaRectObject* self, PyObject* v, void* c) { if(!v)return-1; self->x = PyFloat_AsDouble(v); return 0; }

static PyObject* rect_get_bottom(PydaRectObject* self, void* c) { return PyFloat_FromDouble(self->y + self->h); }
static int rect_set_bottom(PydaRectObject* self, PyObject* v, void* c) { if(!v)return-1; self->y = PyFloat_AsDouble(v) - self->h; return 0; }

static PyObject* rect_get_right(PydaRectObject* self, void* c) { return PyFloat_FromDouble(self->x + self->w); }
static int rect_set_right(PydaRectObject* self, PyObject* v, void* c) { if(!v)return-1; self->x = PyFloat_AsDouble(v) - self->w; return 0; }

static PyObject* rect_get_size(PydaRectObject* self, void* c) { return Py_BuildValue("(dd)", self->w, self->h); }
static int rect_set_size(PydaRectObject* self, PyObject* v, void* c) {
    if(!v || !PySequence_Check(v) || PySequence_Size(v) != 2) return -1;
    self->w = PyFloat_AsDouble(PySequence_GetItem(v, 0));
    self->h = PyFloat_AsDouble(PySequence_GetItem(v, 1));
    return 0;
}

static PyObject* rect_get_topleft(PydaRectObject* self, void* c) { return Py_BuildValue("(dd)", self->x, self->y); }
static int rect_set_topleft(PydaRectObject* self, PyObject* v, void* c) {
    if(!v || !PySequence_Check(v) || PySequence_Size(v) != 2) return -1;
    self->x = PyFloat_AsDouble(PySequence_GetItem(v, 0));
    self->y = PyFloat_AsDouble(PySequence_GetItem(v, 1));
    return 0;
}

static PyObject* rect_get_bottomright(PydaRectObject* self, void* c) { return Py_BuildValue("(dd)", self->x + self->w, self->y + self->h); }
static int rect_set_bottomright(PydaRectObject* self, PyObject* v, void* c) {
    if(!v || !PySequence_Check(v) || PySequence_Size(v) != 2) return -1;
    self->x = PyFloat_AsDouble(PySequence_GetItem(v, 0)) - self->w;
    self->y = PyFloat_AsDouble(PySequence_GetItem(v, 1)) - self->h;
    return 0;
}

static PyObject* rect_get_topright(PydaRectObject* self, void* c) { return Py_BuildValue("(dd)", self->x + self->w, self->y); }
static int rect_set_topright(PydaRectObject* self, PyObject* v, void* c) {
    if(!v || !PySequence_Check(v) || PySequence_Size(v) != 2) return -1;
    self->x = PyFloat_AsDouble(PySequence_GetItem(v, 0)) - self->w;
    self->y = PyFloat_AsDouble(PySequence_GetItem(v, 1));
    return 0;
}

static PyObject* rect_get_bottomleft(PydaRectObject* self, void* c) { return Py_BuildValue("(dd)", self->x, self->y + self->h); }
static int rect_set_bottomleft(PydaRectObject* self, PyObject* v, void* c) {
    if(!v || !PySequence_Check(v) || PySequence_Size(v) != 2) return -1;
    self->x = PyFloat_AsDouble(PySequence_GetItem(v, 0));
    self->y = PyFloat_AsDouble(PySequence_GetItem(v, 1)) - self->h;
    return 0;
}

static PyObject* rect_get_midtop(PydaRectObject* self, void* c) { return Py_BuildValue("(dd)", self->x + (self->w / 2.0), self->y); }
static int rect_set_midtop(PydaRectObject* self, PyObject* v, void* c) {
    if(!v || !PySequence_Check(v) || PySequence_Size(v) != 2) return -1;
    self->x = PyFloat_AsDouble(PySequence_GetItem(v, 0)) - (self->w / 2.0);
    self->y = PyFloat_AsDouble(PySequence_GetItem(v, 1));
    return 0;
}

static PyObject* rect_get_midbottom(PydaRectObject* self, void* c) { return Py_BuildValue("(dd)", self->x + (self->w / 2.0), self->y + self->h); }
static int rect_set_midbottom(PydaRectObject* self, PyObject* v, void* c) {
    if(!v || !PySequence_Check(v) || PySequence_Size(v) != 2) return -1;
    self->x = PyFloat_AsDouble(PySequence_GetItem(v, 0)) - (self->w / 2.0);
    self->y = PyFloat_AsDouble(PySequence_GetItem(v, 1)) - self->h;
    return 0;
}

static PyObject* rect_get_midleft(PydaRectObject* self, void* c) { return Py_BuildValue("(dd)", self->x, self->y + (self->h / 2.0)); }
static int rect_set_midleft(PydaRectObject* self, PyObject* v, void* c) {
    if(!v || !PySequence_Check(v) || PySequence_Size(v) != 2) return -1;
    self->x = PyFloat_AsDouble(PySequence_GetItem(v, 0));
    self->y = PyFloat_AsDouble(PySequence_GetItem(v, 1)) - (self->h / 2.0);
    return 0;
}

static PyObject* rect_get_midright(PydaRectObject* self, void* c) { return Py_BuildValue("(dd)", self->x + self->w, self->y + (self->h / 2.0)); }
static int rect_set_midright(PydaRectObject* self, PyObject* v, void* c) {
    if(!v || !PySequence_Check(v) || PySequence_Size(v) != 2) return -1;
    self->x = PyFloat_AsDouble(PySequence_GetItem(v, 0)) - self->w;
    self->y = PyFloat_AsDouble(PySequence_GetItem(v, 1)) - (self->h / 2.0);
    return 0;
}

static PyObject* rect_get_center(PydaRectObject* self, void* c) { return Py_BuildValue("(dd)", self->x + (self->w / 2.0), self->y + (self->h / 2.0)); }
static int rect_set_center(PydaRectObject* self, PyObject* v, void* c) {
    if(!v || !PySequence_Check(v) || PySequence_Size(v) != 2) return -1;
    self->x = PyFloat_AsDouble(PySequence_GetItem(v, 0)) - (self->w / 2.0);
    self->y = PyFloat_AsDouble(PySequence_GetItem(v, 1)) - (self->h / 2.0);
    return 0;
}

// --- METHODS ---
static PyObject* rect_colliderect(PydaRectObject* self, PyObject* args) {
    PydaRectObject* other;
    if (!PyArg_ParseTuple(args, "O!", &PydaRectType, &other)) return NULL;

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
    } else if (PySequence_Check(arg1) && PySequence_Size(arg1) == 2) {
        px = PyFloat_AsDouble(PySequence_GetItem(arg1, 0));
        py = PyFloat_AsDouble(PySequence_GetItem(arg1, 1));
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid point coordinates");
        return NULL;
    }

    int collide = (px >= self->x && px < self->x + self->w &&
                   py >= self->y && py < self->y + self->h);
    if (collide) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject* rect_inflate(PydaRectObject* self, PyObject* args) {
    double dw, dh;
    if (!PyArg_ParseTuple(args, "dd", &dw, &dh)) return NULL;

    PydaRectObject* res = (PydaRectObject*)PyObject_CallObject((PyObject*)&PydaRectType, NULL);
    if (!res) return NULL;

    res->w = self->w + dw;
    res->h = self->h + dh;
    res->x = self->x - (dw / 2.0);
    res->y = self->y - (dh / 2.0);
    return (PyObject*)res;
}

static PyObject* rect_move(PydaRectObject* self, PyObject* args) {
    double dx, dy;
    if (!PyArg_ParseTuple(args, "dd", &dx, &dy)) return NULL;

    PydaRectObject* res = (PydaRectObject*)PyObject_CallObject((PyObject*)&PydaRectType, NULL);
    if (!res) return NULL;

    res->x = self->x + dx;
    res->y = self->y + dy;
    res->w = self->w;
    res->h = self->h;
    return (PyObject*)res;
}

static PyObject* rect_clip(PydaRectObject* self, PyObject* args) {
    PydaRectObject* other;
    if (!PyArg_ParseTuple(args, "O!", &PydaRectType, &other)) return NULL;

    double x1 = pyda_max(self->x, other->x);
    double y1 = pyda_max(self->y, other->y);
    double x2 = pyda_min(self->x + self->w, other->x + other->w);
    double y2 = pyda_min(self->y + self->h, other->y + other->h);

    PydaRectObject* res = (PydaRectObject*)PyObject_CallObject((PyObject*)&PydaRectType, NULL);
    if (!res) return NULL;

    if (x2 > x1 && y2 > y1) {
        res->x = x1; res->y = y1; res->w = x2 - x1; res->h = y2 - y1;
    } else {
        res->x = 0; res->y = 0; res->w = 0; res->h = 0;
    }
    return (PyObject*)res;
}

// --- MAPPING & MEMBERS ---
static PyMemberDef rect_members[] = {
    {"x", T_DOUBLE, offsetof(PydaRectObject, x), 0, "x coordinate"},
    {"y", T_DOUBLE, offsetof(PydaRectObject, y), 0, "y coordinate"},
    {"width", T_DOUBLE, offsetof(PydaRectObject, w), 0, "width"},
    {"height", T_DOUBLE, offsetof(PydaRectObject, h), 0, "height"},
    {NULL}
};

static PyGetSetDef rect_getset[] = {
    {"top", (getter)rect_get_top, (setter)rect_set_top, "top edge", NULL},
    {"left", (getter)rect_get_left, (setter)rect_set_left, "left edge", NULL},
    {"bottom", (getter)rect_get_bottom, (setter)rect_set_bottom, "bottom edge", NULL},
    {"right", (getter)rect_get_right, (setter)rect_set_right, "right edge", NULL},
    {"size", (getter)rect_get_size, (setter)rect_set_size, "size tuple (width, height)", NULL},
    {"topleft", (getter)rect_get_topleft, (setter)rect_set_topleft, "topleft coordinates", NULL},
    {"bottomright", (getter)rect_get_bottomright, (setter)rect_set_bottomright, "bottomright coordinates", NULL},
    {"topright", (getter)rect_get_topright, (setter)rect_set_topright, "topright coordinates", NULL},
    {"bottomleft", (getter)rect_get_bottomleft, (setter)rect_set_bottomleft, "bottomleft coordinates", NULL},
    {"midtop", (getter)rect_get_midtop, (setter)rect_set_midtop, "midtop coordinate", NULL},
    {"midbottom", (getter)rect_get_midbottom, (setter)rect_set_midbottom, "midbottom coordinate", NULL},
    {"midleft", (getter)rect_get_midleft, (setter)rect_set_midleft, "midleft coordinate", NULL},
    {"midright", (getter)rect_get_midright, (setter)rect_set_midright, "midright coordinate", NULL},
    {"center", (getter)rect_get_center, (setter)rect_set_center, "center coordinate", NULL},
    {NULL}
};

static PyMethodDef rect_methods[] = {
    {"colliderect", (PyCFunction)rect_colliderect, METH_VARARGS, "Test if two rectangles overlap"},
    {"collidepoint", (PyCFunction)rect_collidepoint, METH_VARARGS, "Test if a point is inside the rectangle"},
    {"inflate", (PyCFunction)rect_inflate, METH_VARARGS, "Grow or shrink rectangle size"},
    {"move", (PyCFunction)rect_move, METH_VARARGS, "Move rectangle by offset"},
    {"clip", (PyCFunction)rect_clip, METH_VARARGS, "Crop rectangle by another rectangle intersection"},
    {NULL}
};

static PyTypeObject PydaRectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyda.rect.Rect",
    .tp_doc = "Advanced native geometry Rect object",
    .tp_basicsize = sizeof(PydaRectObject),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = rect_new,
    .tp_init = (initproc)rect_init,
    .tp_repr = (reprfunc)rect_repr,
    .tp_members = rect_members,
    .tp_methods = rect_methods,
    .tp_getset = rect_getset,
};

static PyModuleDef rectmodule = {
    PyModuleDef_HEAD_INIT, "_rect", "Advanced C extension for Rect geometry", -1, NULL
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
