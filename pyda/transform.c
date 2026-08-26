#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>

// Capsule destructor for safe memory cleanup of new SDL_Surface instances
static void free_sdl_surface(PyObject* capsule) {
    SDL_Surface* surf = (SDL_Surface*)PyCapsule_GetPointer(capsule, "SDL_Surface");
    if (surf) {
        SDL_FreeSurface(surf);
    }
}

// Helper to extract surface from capsule
static SDL_Surface* get_surface_from_capsule(PyObject* capsule) {
    SDL_Surface* surf = (SDL_Surface*)PyCapsule_GetPointer(capsule, "SDL_Surface");
    if (!surf) {
        PyErr_SetString(PyExc_TypeError, "Invalid surface capsule");
        return NULL;
    }
    return surf;
}

// --- 1. SCALE SURFACE ---
static PyObject* pyda_transform_scale(PyObject* self, PyObject* args) {
    PyObject* surf_capsule;
    int w, h;
    if (!PyArg_ParseTuple(args, "O(ii)", &surf_capsule, &w, &h)) {
        // Fallback parse if size is passed as separate parameters instead of tuple
        PyErr_Clear();
        if (!PyArg_ParseTuple(args, "Oii", &surf_capsule, &w, &h)) return NULL;
    }

    SDL_Surface* src = get_surface_from_capsule(surf_capsule);
    if (!src) return NULL;

    if (w <= 0 || h <= 0) {
        PyErr_SetString(PyExc_ValueError, "Target width and height must be greater than zero");
        return NULL;
    }

    SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(0, w, h, src->format->BitsPerPixel, src->format->format);
    if (!dst) {
        PyErr_Format(PyExc_RuntimeError, "Failed to create destination surface: %s", SDL_GetError());
        return NULL;
    }

    // Preserve color key / alpha blending settings if present
    if (src->flags & SDL_SRCCOLORKEY) {
        Uint32 colorkey;
        SDL_GetColorKey(src, &colorkey);
        SDL_SetColorKey(dst, SDL_TRUE, colorkey);
    }

    SDL_Rect rect = {0, 0, w, h};
    if (SDL_BlitScaled(src, NULL, dst, &rect) < 0) {
        SDL_FreeSurface(dst);
        PyErr_Format(PyExc_RuntimeError, "SDL_BlitScaled failed: %s", SDL_GetError());
        return NULL;
    }

    return PyCapsule_New((void*)dst, "SDL_Surface", free_sdl_surface);
}

// --- 2. ROTATE SURFACE (90-degree increments via pure pixel rearrangement) ---
static PyObject* pyda_transform_rotate(PyObject* self, PyObject* args) {
    PyObject* surf_capsule;
    int angle;
    if (!PyArg_ParseTuple(args, "Oi", &surf_capsule, &angle)) return NULL;

    SDL_Surface* src = get_surface_from_capsule(surf_capsule);
    if (!src) return NULL;

    // Normalize angle to 0, 90, 180, 270
    angle = angle % 360;
    if (angle < 0) angle += 360;

    int w = src->w;
    int h = src->h;
    int new_w = (angle == 90 || angle == 270) ? h : w;
    int new_h = (angle == 90 || angle == 270) ? w : h;

    SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(0, new_w, new_h, src->format->BitsPerPixel, src->format->format);
    if (!dst) return NULL;

    if (SDL_MUSTLOCK(src)) SDL_LockSurface(src);
    if (SDL_MUSTLOCK(dst)) SDL_LockSurface(dst);

    int bpp = src->format->BytesPerPixel;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Uint8* src_pixel = (Uint8*)src->pixels + y * src->pitch + x * bpp;
            int nx = x, ny = y;

            if (angle == 90) {
                nx = y;
                ny = w - 1 - x;
            } else if (angle == 180) {
                nx = w - 1 - x;
                ny = h - 1 - y;
            } else if (angle == 270) {
                nx = h - 1 - y;
                ny = x;
            }

            if (nx >= 0 && nx < new_w && ny >= 0 && ny < new_h) {
                Uint8* dst_pixel = (Uint8*)dst->pixels + ny * dst->pitch + nx * bpp;
                if (bpp == 4) {
                    *(Uint32*)dst_pixel = *(Uint32*)src_pixel;
                } else if (bpp == 3) {
                    dst_pixel[0] = src_pixel[0];
                    dst_pixel[1] = src_pixel[1];
                    dst_pixel[2] = src_pixel[2];
                } else if (bpp == 1) {
                    *dst_pixel = *src_pixel;
                }
            }
        }
    }

    if (SDL_MUSTLOCK(dst)) SDL_UnlockSurface(dst);
    if (SDL_MUSTLOCK(src)) SDL_UnlockSurface(src);

    return PyCapsule_New((void*)dst, "SDL_Surface", free_sdl_surface);
}

// --- 3. FLIP SURFACE (Horizontal / Vertical) ---
static PyObject* pyda_transform_flip(PyObject* self, PyObject* args) {
    PyObject* surf_capsule;
    int flip_x, flip_y;
    if (!PyArg_ParseTuple(args, "Oii", &surf_capsule, &flip_x, &flip_y)) return NULL;

    SDL_Surface* src = get_surface_from_capsule(surf_capsule);
    if (!src) return NULL;

    int w = src->w;
    int h = src->h;

    SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(0, w, h, src->format->BitsPerPixel, src->format->format);
    if (!dst) return NULL;

    if (SDL_MUSTLOCK(src)) SDL_LockSurface(src);
    if (SDL_MUSTLOCK(dst)) SDL_LockSurface(dst);

    int bpp = src->format->BytesPerPixel;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int nx = flip_x ? (w - 1 - x) : x;
            int ny = flip_y ? (h - 1 - y) : y;

            Uint8* src_pixel = (Uint8*)src->pixels + y * src->pitch + x * bpp;
            Uint8* dst_pixel = (Uint8*)dst->pixels + ny * dst->pitch + nx * bpp;

            if (bpp == 4) {
                *(Uint32*)dst_pixel = *(Uint32*)src_pixel;
            } else if (bpp == 3) {
                dst_pixel[0] = src_pixel[0];
                dst_pixel[1] = src_pixel[1];
                dst_pixel[2] = src_pixel[2];
            } else if (bpp == 1) {
                *dst_pixel = *src_pixel;
            }
        }
    }

    if (SDL_MUSTLOCK(dst)) SDL_UnlockSurface(dst);
    if (SDL_MUSTLOCK(src)) SDL_UnlockSurface(src);

    return PyCapsule_New((void*)dst, "SDL_Surface", free_sdl_surface);
}

static PyMethodDef TransformMethods[] = {
    {"scale", pyda_transform_scale, METH_VARARGS, "Resize surface to new resolution"},
    {"rotate", pyda_transform_rotate, METH_VARARGS, "Rotate surface by angle (90 degree increments)"},
    {"flip", pyda_transform_flip, METH_VARARGS, "Flip surface horizontally and/or vertically"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef transform_module = {
    PyModuleDef_HEAD_INIT, 
    "_transform", 
    "Zero-dependency native transformation extension", 
    -1, 
    TransformMethods
};

PyMODINIT_FUNC PyInit__transform(void) {
    return PyModule_Create(&transform_module);
}
