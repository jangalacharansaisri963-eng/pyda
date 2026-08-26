#define PY_SSIZE_T_CLEAN
#include <Python.h>

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

static PyObject* rect_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    PydaRectObject* self = (PydaRectObject*)type->tp_alloc(type, 0);
    if (self) {
        self->x = 0; self->y = 0; self->w = 0; self->h = 0;
    }
    return (PyObject*)self;
}

static int rect_init(PydaRectObject* self, PyObject* args, PyObject* kwds) {
    double x = 0, y = 0, w = 0, h = 0;
    if (!PyArg_ParseTuple(args, "|dddd", &x, &y, &w, &h)) return -1;
    self->x = x; self->y = y; self->w = w; self->h = h;
    return 0;
}

static PyMemberDef rect_members[] = {
    {"x", T_DOUBLE, offsetof(PydaRectObject, x), 0, "x position"},
    {"y", T_DOUBLE, offsetof(PydaRectObject, y), 0, "y position"},
    {"width", T_DOUBLE, offsetof(PydaRectObject, w), 0, "width"},
    {"height", T_DOUBLE, offsetof(PydaRectObject, h), 0, "height"},
    {NULL}
};

static PyMethodDef rect_methods[] = {
    {"colliderect", (PyCFunction)rect_colliderect, METH_VARARGS, "Collision check"},
    {NULL}
};

static PyTypeObject PydaRectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyda.pygame.rect.Rect",
    .tp_doc = "Native Rect object",
    .tp_basicsize = sizeof(PydaRectObject),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = rect_new,
    .tp_init = (initproc)rect_init,
    .tp_members = rect_members,
    .tp_methods = rect_methods,
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
