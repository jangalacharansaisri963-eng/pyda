#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <math.h>

// Helper to extract SDL_Surface from Python capsule
static SDL_Surface* get_surface_from_capsule(PyObject* capsule) {
    SDL_Surface* surf = (SDL_Surface*)PyCapsule_GetPointer(capsule, "SDL_Surface");
    if (!surf) {
        PyErr_SetString(PyExc_TypeError, "Invalid surface capsule");
        return NULL;
    }
    return surf;
}

// Helper to parse color argument (supports tuples, lists, or packed integer/hex values)
static int parse_color(PyObject* obj, Uint8* r, Uint8* g, Uint8* b, Uint8* a) {
    if (PyTuple_Check(obj) || PyList_Check(obj)) {
        Py_ssize_t size = PySequence_Size(obj);
        if (size < 3) {
            PyErr_SetString(PyExc_ValueError, "Color sequence must have at least 3 elements (R, G, B)");
            return 0;
        }
        *r = (Uint8)PyLong_AsLong(PySequence_GetItem(obj, 0));
        *g = (Uint8)PyLong_AsLong(PySequence_GetItem(obj, 1));
        *b = (Uint8)PyLong_AsLong(PySequence_GetItem(obj, 2));
        *a = (size >= 4) ? (Uint8)PyLong_AsLong(PySequence_GetItem(obj, 3)) : 255;
        return 1;
    } else if (PyLong_Check(obj)) {
        unsigned long val = PyLong_AsUnsignedLong(obj);
        *r = (Uint8)((val >> 16) & 0xFF);
        *g = (Uint8)((val >> 8) & 0xFF);
        *b = (Uint8)(val & 0xFF);
        *a = 255;
        return 1;
    }
    PyErr_SetString(PyExc_TypeError, "Invalid color format");
    return 0;
}

// --- 1. RECTANGLE DRAWING (Solid & Outlined with width) ---
static PyObject* draw_rect(PyObject* self, PyObject* args) {
    PyObject* surf_capsule;
    PyObject* color_obj;
    int x, y, w, h;
    int width = 0; // 0 means filled

    if (!PyArg_ParseTuple(args, "OO(iiii)|i", &surf_capsule, &color_obj, &x, &y, &w, &h, &width)) {
        return NULL;
    }

    SDL_Surface* surface = get_surface_from_capsule(surf_capsule);
    if (!surface) return NULL;

    Uint8 r, g, b, a;
    if (!parse_color(color_obj, &r, &g, &b, &a)) return NULL;
    Uint32 mapped = SDL_MapRGBA(surface->format, r, g, b, a);

    if (width <= 0) {
        SDL_Rect rect = {x, y, w, h};
        SDL_FillRect(surface, &rect, mapped);
    } else {
        // Draw outline borders using custom rectangles
        SDL_Rect top = {x, y, w, width};
        SDL_Rect bottom = {x, y + h - width, w, width};
        SDL_Rect left = {x, y, width, h};
        SDL_Rect right = {x + w - width, y, width, h};
        SDL_FillRect(surface, &top, mapped);
        SDL_FillRect(surface, &bottom, mapped);
        SDL_FillRect(surface, &left, mapped);
        SDL_FillRect(surface, &right, mapped);
    }

    Py_RETURN_NONE;
}

// --- 2. LINE DRAWING (Bresenham's Algorithm) ---
static PyObject* draw_line(PyObject* self, PyObject* args) {
    PyObject* surf_capsule;
    PyObject* color_obj;
    int x0, y0, x1, y1;
    int width = 1;

    if (!PyArg_ParseTuple(args, "OO(ii)(ii)|i", &surf_capsule, &color_obj, &x0, &y0, &x1, &y1, &width)) {
        return NULL;
    }

    SDL_Surface* surface = get_surface_from_capsule(surf_capsule);
    if (!surface) return NULL;

    Uint8 r, g, b, a;
    if (!parse_color(color_obj, &r, &g, &b, &a)) return NULL;
    Uint32 mapped = SDL_MapRGBA(surface->format, r, g, b, a);

    // Lock surface if necessary
    if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);

    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        // Draw thick pixel point if width > 1
        for (int wx = 0; wx < width; wx++) {
            for (int wy = 0; wy < width; wy++) {
                int px = x0 + wx;
                int py = y0 + wy;
                if (px >= 0 && px < surface->w && py >= 0 && py < surface->h) {
                    Uint8* target = (Uint8*)surface->pixels + py * surface->pitch + px * surface->format->BytesPerPixel;
                    if (surface->format->BytesPerPixel == 4) *(Uint32*)target = mapped;
                    else if (surface->format->BytesPerPixel == 3) {
                        target[0] = r; target[1] = g; target[2] = b;
                    }
                }
            }
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }

    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
    Py_RETURN_NONE;
}

// --- 3. CIRCLE DRAWING (Midpoint Circle Algorithm) ---
static PyObject* draw_circle(PyObject* self, PyObject* args) {
    PyObject* surf_capsule;
    PyObject* color_obj;
    int cx, cy, radius;
    int width = 0; // 0 = filled

    if (!PyArg_ParseTuple(args, "OO(ii)i|i", &surf_capsule, &color_obj, &cx, &cy, &radius, &width)) {
        return NULL;
    }

    SDL_Surface* surface = get_surface_from_capsule(surf_capsule);
    if (!surface) return NULL;

    Uint8 r, g, b, a;
    if (!parse_color(color_obj, &r, &g, &b, &a)) return NULL;
    Uint32 mapped = SDL_MapRGBA(surface->format, r, g, b, a);

    if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);

    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        // Symmetric points mapping helper macro
        #define DRAW_CIRCLE_POINTS(px, py) do { \
            int pts[8][2] = { \
                {cx + px, cy + py}, {cx - px, cy + py}, \
                {cx + px, cy - py}, {cx - px, cy - py}, \
                {cx + py, cy + px}, {cx - py, cy + px}, \
                {cx + py, cy - px}, {cx - py, cy - px}  \
            }; \
            for(int i=0; i<8; i++) { \
                int rx = pts[i][0], ry = pts[i][1]; \
                if (rx >= 0 && rx < surface->w && ry >= 0 && ry < surface->h) { \
                    Uint8* target = (Uint8*)surface->pixels + ry * surface->pitch + rx * surface->format->BytesPerPixel; \
                    if (surface->format->BytesPerPixel == 4) *(Uint32*)target = mapped; \
                } \
            } \
        } while(0)

        if (width <= 0) {
            // Filled circle scanlines
            for (int dy = -y; dy <= y; dy++) {
                int rx1 = cx - x, rx2 = cx + x;
                SDL_Rect scanline = {rx1, cy + dy, (rx2 - rx1) + 1, 1};
                SDL_FillRect(surface, &scanline, mapped);
            }
        } else {
            DRAW_CIRCLE_POINTS(x, y);
        }

        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        }
        if (err > 0) {
            x--;
            err -= 2 * x + 1;
        }
    }

    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
    Py_RETURN_NONE;
}

// --- MODULE METHODS TABLE ---
static PyMethodDef DrawMethods[] = {
    {"rect", draw_rect, METH_VARARGS, "Draw filled or outlined rectangle"},
    {"line", draw_line, METH_VARARGS, "Draw a straight line"},
    {"circle", draw_circle, METH_VARARGS, "Draw a circle"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef draw_module = {
    PyModuleDef_HEAD_INIT, 
    "_draw", 
    "High-performance native drawing extension", 
    -1, 
    DrawMethods
};

PyMODINIT_FUNC PyInit__draw(void) {
    return PyModule_Create(&draw_module);
}
