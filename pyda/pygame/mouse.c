#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>

static PyObject* pyda_mouse_get_pos(PyObject* self, PyObject* args) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    return Py_BuildValue("(ii)", x, y);
}

static PyObject* pyda_mouse_get_pressed(PyObject* self, PyObject* args) {
    int x, y;
    Uint32 buttons = SDL_GetMouseState(&x, &y);
    
    int left = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) ? 1 : 0;
    int middle = (buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) ? 1 : 0;
    int right = (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) ? 1 : 0;
    
    return Py_BuildValue("(iii)", left, middle, right);
}

static PyObject* pyda_mouse_set_visible(PyObject* self, PyObject* args) {
    int toggle;
    if (!PyArg_ParseTuple(args, "i", &toggle)) return NULL;
    int prev = SDL_ShowCursor(toggle);
    return PyLong_FromLong(prev);
}

static PyMethodDef MouseMethods[] = {
    {"get_pos", pyda_mouse_get_pos, METH_NOARGS, "Get mouse cursor (x, y) coordinates"},
    {"get_pressed", pyda_mouse_get_pressed, METH_NOARGS, "Get mouse button states (left, middle, right)"},
    {"set_visible", pyda_mouse_set_visible, METH_VARARGS, "Show or hide the mouse cursor"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef mouse_module = {
    PyModuleDef_HEAD_INIT,
    "_mouse",
    "High-performance C backend for pyda.pygame.mouse",
    -1,
    MouseMethods
};

PyMODINIT_FUNC PyInit__mouse(void) {
    return PyModule_Create(&mouse_module);
}
