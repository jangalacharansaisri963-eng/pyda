#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>

// --- 1. GET POSITION ---
static PyObject* mouse_get_pos(PyObject* self, PyObject* args) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    return Py_BuildValue("(ii)", x, y);
}

// --- 2. GET PRESSED BUTTONS (Left, Middle, Right, X1, X2) ---
static PyObject* mouse_get_pressed(PyObject* self, PyObject* args) {
    int x, y;
    Uint32 buttons = SDL_GetMouseState(&x, &y);
    return Py_BuildValue("(iiiii)", 
        (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) ? 1 : 0,
        (buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) ? 1 : 0,
        (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) ? 1 : 0,
        (buttons & SDL_BUTTON(SDL_BUTTON_X1)) ? 1 : 0,
        (buttons & SDL_BUTTON(SDL_BUTTON_X2)) ? 1 : 0
    );
}

// --- 3. GET RELATIVE MOTION (Delta since last call) ---
static PyObject* mouse_get_rel(PyObject* self, PyObject* args) {
    int x, y;
    SDL_GetRelativeMouseState(&x, &y);
    return Py_BuildValue("(ii)", x, y);
}

// --- 4. WARP CURSOR POSITION ---
static PyObject* mouse_set_pos(PyObject* self, PyObject* args) {
    int x, y;
    PyObject* window_capsule = NULL;
    
    // Supports either just (x, y) or passing a window capsule context
    if (!PyArg_ParseTuple(args, "ii|O", &x, &y, &window_capsule)) return NULL;

    SDL_Window* win = NULL;
    if (window_capsule && PyCapsule_CheckExact(window_capsule)) {
        win = (SDL_Window*)PyCapsule_GetPointer(window_capsule, "SDL_Window");
    }

    SDL_WarpMouseInWindow(win, x, y);
    Py_RETURN_NONE;
}

// --- 5. CURSOR VISIBILITY CONTROL ---
static PyObject* mouse_set_visible(PyObject* self, PyObject* args) {
    int toggle;
    if (!PyArg_ParseTuple(args, "i", &toggle)) return NULL;
    
    int prev = SDL_ShowCursor(toggle ? SDL_ENABLE : SDL_DISABLE);
    return PyLong_FromLong(prev == SDL_ENABLE ? 1 : 0);
}

// --- 6. GRAB / LOCK MOUSE TO WINDOW ---
static PyObject* mouse_set_grabbed(PyObject* self, PyObject* args) {
    PyObject* window_capsule;
    int grab;
    if (!PyArg_ParseTuple(args, "Oi", &window_capsule, &grab)) return NULL;

    SDL_Window* win = (SDL_Window*)PyCapsule_GetPointer(window_capsule, "SDL_Window");
    if (!win) return NULL;

    SDL_SetWindowGrab(win, grab ? SDL_TRUE : SDL_FALSE);
    Py_RETURN_NONE;
}

static PyMethodDef MouseMethods[] = {
    {"get_pos", mouse_get_pos, METH_NOARGS, "Get mouse coordinates (x, y)"},
    {"get_pressed", mouse_get_pressed, METH_NOARGS, "Get state of mouse buttons (left, middle, right, x1, x2)"},
    {"get_rel", mouse_get_rel, METH_NOARGS, "Get relative mouse motion delta since last call"},
    {"set_pos", mouse_set_pos, METH_VARARGS, "Warp mouse cursor to specified coordinates"},
    {"set_visible", mouse_set_visible, METH_VARARGS, "Show or hide the mouse cursor"},
    {"set_grabbed", mouse_set_grabbed, METH_VARARGS, "Confine or release mouse inside window bounds"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef mouse_module = {
    PyModuleDef_HEAD_INIT, 
    "_mouse", 
    "Zero-dependency native mouse extension", 
    -1, 
    MouseMethods
};

PyMODINIT_FUNC PyInit__mouse(void) {
    return PyModule_Create(&mouse_module);
}
