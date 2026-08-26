#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

// Forward declarations of types
static PyTypeObject PydaSpriteType;
static PyTypeObject PydaGroupType;

// ==========================================
// 1. SPRITE OBJECT
// ==========================================
typedef struct {
    PyObject_HEAD
    PyObject* groups; // Python set containing parent groups
    PyObject* rect;   // Associated Rect object
    PyObject* image;  // Associated surface capsule/object
} PydaSpriteObject;

static PyObject* sprite_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    PydaSpriteObject* self = (PydaSpriteObject*)type->tp_alloc(type, 0);
    if (self) {
        self->groups = PySet_New(NULL);
        self->rect = Py_None;
        Py_INCREF(Py_None);
        self->image = Py_None;
        Py_INCREF(Py_None);
    }
    return (PyObject*)self;
}

static int sprite_init(PydaSpriteObject* self, PyObject* args, PyObject* kwds) {
    PyObject* group_arg = NULL;
    if (!PyArg_ParseTuple(args, "|O", &group_arg)) return -1;

    if (group_arg) {
        // Can be initialized with a single group or a sequence of groups
        if (PyObject_IsInstance(group_arg, (PyObject*)&PydaGroupType)) {
            PySet_Add(self->groups, group_arg);
            // Call group.add(self)
            PyObject_CallMethod(group_arg, "add", "(O)", self);
        } else if (PySequence_Check(group_arg)) {
            Py_ssize_t size = PySequence_Size(group_arg);
            for (Py_ssize_t i = 0; i < size; i++) {
                PyObject* g = PySequence_GetItem(group_arg, i);
                PySet_Add(self->groups, g);
                PyObject_CallMethod(g, "add", "(O)", self);
                Py_DECREF(g);
            }
        }
    }
    return 0;
}

static void sprite_dealloc(PydaSpriteObject* self) {
    // Remove self from all containing groups first
    PyObject* iterator = PyObject_GetIter(self->groups);
    PyObject* item;
    while ((item = PyIter_Next(iterator))) {
        PyObject_CallMethod(item, "remove", "(O)", self);
        Py_DECREF(item);
    }
    Py_DECREF(iterator);

    Py_XDECREF(self->groups);
    Py_XDECREF(self->rect);
    Py_XDECREF(self->image);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* sprite_add(PydaSpriteObject* self, PyObject* args) {
    Py_ssize_t argc = PyTuple_Size(args);
    for (Py_ssize_t i = 0; i < argc; i++) {
        PyObject* g = PyTuple_GetItem(args, i);
        PySet_Add(self->groups, g);
        PyObject_CallMethod(g, "add", "(O)", self);
    }
    Py_RETURN_NONE;
}

static PyObject* sprite_remove(PydaSpriteObject* self, PyObject* args) {
    Py_ssize_t argc = PyTuple_Size(args);
    for (Py_ssize_t i = 0; i < argc; i++) {
        PyObject* g = PyTuple_GetItem(args, i);
        PySet_Discard(self->groups, g);
        PyObject_CallMethod(g, "remove", "(O)", self);
    }
    Py_RETURN_NONE;
}

static PyObject* sprite_kill(PydaSpriteObject* self, PyObject* args) {
    PyObject* iterator = PyObject_GetIter(self->groups);
    PyObject* item;
    while ((item = PyIter_Next(iterator))) {
        PyObject_CallMethod(item, "remove", "(O)", self);
        Py_DECREF(item);
    }
    Py_DECREF(iterator);
    PySet_Clear(self->groups);
    Py_RETURN_NONE;
}

static PyObject* sprite_alive(PydaSpriteObject* self, PyObject* args) {
    if (PySet_Size(self->groups) > 0) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyMethodDef sprite_methods[] = {
    {"add", (PyCFunction)sprite_add, METH_VARARGS, "Add sprite to group(s)"},
    {"remove", (PyCFunction)sprite_remove, METH_VARARGS, "Remove sprite from group(s)"},
    {"kill", (PyCFunction)sprite_kill, METH_NOARGS, "Remove sprite from all groups"},
    {"alive", (PyCFunction)sprite_alive, METH_NOARGS, "Check if sprite belongs to any groups"},
    {NULL}
};

static PyMemberDef sprite_members[] = {
    {"groups", T_OBJECT_EX, offsetof(PydaSpriteObject, groups), 0, "Groups container"},
    {"rect", T_OBJECT_EX, offsetof(PydaSpriteObject, rect), 0, "Surface bounding rect"},
    {"image", T_OBJECT_EX, offsetof(PydaSpriteObject, image), 0, "Surface image"},
    {NULL}
};

static PyTypeObject PydaSpriteType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyda.pygame.sprite.Sprite",
    .tp_doc = "Base class for visible game objects",
    .tp_basicsize = sizeof(PydaSpriteObject),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = sprite_new,
    .tp_init = (initproc)sprite_init,
    .tp_dealloc = (destructor)sprite_dealloc,
    .tp_methods = sprite_methods,
    .tp_members = sprite_members,
};


// ==========================================
// 2. GROUP OBJECT
// ==========================================
typedef struct {
    PyObject_HEAD
    PyObject* sprites; // Python set containing sprite instances
} PydaGroupObject;

static PyObject* group_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    PydaGroupObject* self = (PydaGroupObject*)type->tp_alloc(type, 0);
    if (self) {
        self->sprites = PySet_New(NULL);
    }
    return (PyObject*)self;
}

static int group_init(PydaGroupObject* self, PyObject* args, PyObject* kwds) {
    PyObject* sprites_arg = NULL;
    if (!PyArg_ParseTuple(args, "|O", &sprites_arg)) return -1;
    if (sprites_arg) {
        PyObject_CallMethod((PyObject*)self, "add", "(O)", sprites_arg);
    }
    return 0;
}

static void group_dealloc(PydaGroupObject* self) {
    Py_XDECREF(self->sprites);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* group_add(PydaGroupObject* self, PyObject* args) {
    Py_ssize_t argc = PyTuple_Size(args);
    for (Py_ssize_t i = 0; i < argc; i++) {
        PyObject* item = PyTuple_GetItem(args, i);
        if (PyObject_IsInstance(item, (PyObject*)&PydaSpriteType)) {
            PySet_Add(self->sprites, item);
            // Also register group inside sprite.groups
            PyObject* sprite_groups = PyObject_GetAttrString(item, "groups");
            if (sprite_groups) {
                PySet_Add(sprite_groups, (PyObject*)self);
                Py_DECREF(sprite_groups);
            }
        } else if (PySequence_Check(item)) {
            Py_ssize_t seq_size = PySequence_Size(item);
            for (Py_ssize_t j = 0; j < seq_size; j++) {
                PyObject* sub = PySequence_GetItem(item, j);
                if (PyObject_IsInstance(sub, (PyObject*)&PydaSpriteType)) {
                    PySet_Add(self->sprites, sub);
                    PyObject* sprite_groups = PyObject_GetAttrString(sub, "groups");
                    if (sprite_groups) {
                        PySet_Add(sprite_groups, (PyObject*)self);
                        Py_DECREF(sprite_groups);
                    }
                }
                Py_DECREF(sub);
            }
        }
    }
    Py_RETURN_NONE;
}

static PyObject* group_remove(PydaGroupObject* self, PyObject* args) {
    Py_ssize_t argc = PyTuple_Size(args);
    for (Py_ssize_t i = 0; i < argc; i++) {
        PyObject* item = PyTuple_GetItem(args, i);
        if (PyObject_IsInstance(item, (PyObject*)&PydaSpriteType)) {
            PySet_Discard(self->sprites, item);
        } else if (PySequence_Check(item)) {
            Py_ssize_t seq_size = PySequence_Size(item);
            for (Py_ssize_t j = 0; j < seq_size; j++) {
                PyObject* sub = PySequence_GetItem(item, j);
                PySet_Discard(self->sprites, sub);
                Py_DECREF(sub);
            }
        }
    }
    Py_RETURN_NONE;
}

static PyObject* group_has(PydaGroupObject* self, PyObject* args) {
    Py_ssize_t argc = PyTuple_Size(args);
    for (Py_ssize_t i = 0; i < argc; i++) {
        PyObject* item = PyTuple_GetItem(args, i);
        int contains = PySet_Contains(self->sprites, item);
        if (contains <= 0) Py_RETURN_FALSE;
    }
    Py_RETURN_TRUE;
}

static PyObject* group_update(PydaGroupObject* self, PyObject* args, PyObject* kwds) {
    PyObject* iterator = PyObject_GetIter(self->sprites);
    PyObject* sprite;
    while ((sprite = PyIter_Next(iterator))) {
        PyObject_CallMethod(sprite, "update", "O*", args);
        Py_DECREF(sprite);
    }
    Py_DECREF(iterator);
    Py_RETURN_NONE;
}

static PyObject* group_draw(PydaGroupObject* self, PyObject* args) {
    PyObject* surface;
    if (!PyArg_ParseTuple(args, "O", &surface)) return NULL;

    PyObject* iterator = PyObject_GetIter(self->sprites);
    PyObject* sprite;
    while ((sprite = PyIter_Next(iterator))) {
        PyObject* image = PyObject_GetAttrString(sprite, "image");
        PyObject* rect = PyObject_GetAttrString(sprite, "rect");
        if (image && rect && image != Py_None && rect != Py_None) {
            // Call surface.blit(image, rect)
            PyObject_CallMethod(surface, "blit", "OO", image, rect);
        }
        Py_XDECREF(image);
        Py_XDECREF(rect);
        Py_DECREF(sprite);
    }
    Py_DECREF(iterator);
    Py_RETURN_NONE;
}

static Py_ssize_t group_length(PydaGroupObject* self) {
    return PySet_Size(self->sprites);
}

static PyObject* group_iter(PydaGroupObject* self) {
    return PyObject_GetIter(self->sprites);
}

static PySequenceMethods group_as_sequence = {
    .sq_length = (lenfunc)group_length,
};

static PyMethodDef group_methods[] = {
    {"add", (PyCFunction)group_add, METH_VARARGS, "Add sprites to group"},
    {"remove", (PyCFunction)group_remove, METH_VARARGS, "Remove sprites from group"},
    {"has", (PyCFunction)group_has, METH_VARARGS, "Check if group contains sprite(s)"},
    {"update", (PyCFunction)group_update, METH_VARARGS | METH_KEYWORDS, "Call update on all sprites"},
    {"draw", (PyCFunction)group_draw, METH_VARARGS, "Draw all sprite images to surface"},
    {NULL}
};

static PyMemberDef group_members[] = {
    {"sprites_set", T_OBJECT_EX, offsetof(PydaGroupObject, sprites), 0, "Internal sprites set"},
    {NULL}
};

static PyTypeObject PydaGroupType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyda.pygame.sprite.Group",
    .tp_doc = "Container class for managing sprites",
    .tp_basicsize = sizeof(PydaGroupObject),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = group_new,
    .tp_init = (initproc)group_init,
    .tp_dealloc = (destructor)group_dealloc,
    .tp_as_sequence = &group_as_sequence,
    .tp_iter = (getiterfunc)group_iter,
    .tp_methods = group_methods,
    .tp_members = group_members,
};


// ==========================================
// 3. MODULE COLLISION UTILITIES
// ==========================================
static PyObject* sprite_collide_any(PyObject* module, PyObject* args) {
    PyObject *sprite, *group;
    if (!PyArg_ParseTuple(args, "OO", &sprite, &group)) return NULL;

    PyObject* iterator = PyObject_GetIter(group);
    if (!iterator) return NULL;

    PyObject* other;
    PyObject* collided = NULL;
    while ((other = PyIter_Next(iterator))) {
        if (sprite == other) {
            Py_DECREF(other);
            continue;
        }
        PyObject* s_rect = PyObject_GetAttrString(sprite, "rect");
        PyObject* o_rect = PyObject_GetAttrString(other, "rect");
        if (s_rect && o_rect && s_rect != Py_None && o_rect != Py_None) {
            PyObject* res = PyObject_CallMethod(s_rect, "colliderect", "O", o_rect);
            if (res == Py_True) {
                collided = other;
                Py_DECREF(res);
                Py_DECREF(s_rect);
                Py_DECREF(o_rect);
                break;
            }
            Py_XDECREF(res);
        }
        Py_XDECREF(s_rect);
        Py_XDECREF(o_rect);
        Py_DECREF(other);
    }
    Py_DECREF(iterator);

    if (collided) return collided;
    Py_RETURN_NONE;
}

static PyObject* sprite_sprite_collide(PyObject* module, PyObject* args) {
    PyObject *sprite, *group;
    int dokill = 0;
    if (!PyArg_ParseTuple(args, "OO|p", &sprite, &group, &dokill)) return NULL;

    PyObject* list = PyList_New(0);
    PyObject* iterator = PyObject_GetIter(group);
    if (!iterator) {
        Py_DECREF(list);
        return NULL;
    }

    PyObject* other;
    while ((other = PyIter_Next(iterator))) {
        PyObject* s_rect = PyObject_GetAttrString(sprite, "rect");
        PyObject* o_rect = PyObject_GetAttrString(other, "rect");
        if (s_rect && o_rect && s_rect != Py_None && o_rect != Py_None) {
            PyObject* res = PyObject_CallMethod(s_rect, "colliderect", "O", o_rect);
            if (res == Py_True) {
                PyList_Append(list, other);
                if (dokill) {
                    PyObject_CallMethod(other, "kill", NULL);
                }
                Py_DECREF(res);
                Py_DECREF(s_rect);
                Py_DECREF(o_rect);
                Py_DECREF(other);
                break; // or continue if collecting all
            }
            Py_XDECREF(res);
        }
        Py_XDECREF(s_rect);
        Py_XDECREF(o_rect);
        Py_DECREF(other);
    }
    Py_DECREF(iterator);
    return list;
}

static PyMethodDef SpriteModuleMethods[] = {
    {"spritecollideany", sprite_collide_any, METH_VARARGS, "Finds any sprites in a group that intersect with a sprite"},
    {"spritecollide", sprite_sprite_collide, METH_VARARGS, "Collision detection between a sprite and a group"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef sprite_module = {
    PyModuleDef_HEAD_INIT,
    "_sprite",
    "Zero-dependency native Sprite and Group engine",
    -1,
    SpriteModuleMethods
};

PyMODINIT_FUNC PyInit__sprite(void) {
    PyObject* m;
    if (PyType_Ready(&PydaSpriteType) < 0) return NULL;
    if (PyType_Ready(&PydaGroupType) < 0) return NULL;

    m = PyModule_Create(&sprite_module);
    if (!m) return NULL;

    Py_INCREF(&PydaSpriteType);
    PyModule_AddObject(m, "Sprite", (PyObject*)&PydaSpriteType);

    Py_INCREF(&PydaGroupType);
    PyModule_AddObject(m, "Group", (PyObject*)&PydaGroupType);

    return m;
}
