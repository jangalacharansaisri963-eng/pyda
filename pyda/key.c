#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>

// --- 1. GET PRESSED (High-speed key state tuple) ---
static PyObject* key_get_pressed(PyObject* self, PyObject* args) {
    int numkeys;
    const Uint8* state = SDL_GetKeyboardState(&numkeys);
    PyObject* tuple = PyTuple_New(numkeys);
    if (!tuple) return NULL;

    for (int i = 0; i < numkeys; i++) {
        // Using Py_NewRef / direct integers for maximum speed
        PyObject* val = state[i] ? Py_True : Py_False;
        Py_INCREF(val);
        PyTuple_SetItem(tuple, i, val);
    }
    return tuple;
}

// --- 2. MODIFIER KEYS CHECK ---
static PyObject* key_get_mods(PyObject* self, PyObject* args) {
    SDL_Keymod mods = SDL_GetModState();
    return PyLong_FromLong((long)mods);
}

// --- 3. KEY NAME UTILITY ---
static PyObject* key_name(PyObject* self, PyObject* args) {
    int keycode;
    if (!PyArg_ParseTuple(args, "i", &keycode)) return NULL;

    SDL_Keycode scancode = SDL_GetKeyFromScancode((SDL_Scancode)keycode);
    const char* name = SDL_GetKeyName(scancode);
    if (!name) name = "";

    return PyUnicode_FromString(name);
}

// --- 4. KEY TO SCANCODE MAPPING ---
static PyObject* key_scancode(PyObject* self, PyObject* args) {
    int keycode;
    if (!PyArg_ParseTuple(args, "i", &keycode)) return NULL;

    SDL_Scancode scancode = SDL_GetScancodeFromKey((SDL_Keycode)keycode);
    return PyLong_FromLong((long)scancode);
}

static PyMethodDef KeyMethods[] = {
    {"get_pressed", key_get_pressed, METH_NOARGS, "Return a sequence of boolean values representing keyboard state"},
    {"get_mods", key_get_mods, METH_NOARGS, "Determine which modifier keys are currently pressed"},
    {"name", key_name, METH_VARARGS, "Get name of key identifier"},
    {"scancode", key_scancode, METH_VARARGS, "Get scancode from keycode"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef key_module = {
    PyModuleDef_HEAD_INIT, 
    "_key", 
    "Zero-dependency native keyboard extension", 
    -1, 
    KeyMethods
};

PyMODINIT_FUNC PyInit__key(void) {
    return PyModule_Create(&key_module);
}
