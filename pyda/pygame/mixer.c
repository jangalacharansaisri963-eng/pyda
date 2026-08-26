#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

static PyObject* pyda_mixer_init(PyObject* self, PyObject* args) {
    int freq = 44100;
    Uint16 format = AUDIO_S16SYS;
    int channels = 2;
    int chunksize = 1024;

    if (!PyArg_ParseTuple(args, "|iiii", &freq, &format, &channels, &chunksize)) return NULL;
    if (Mix_OpenAudio(freq, format, channels, chunksize) < 0) {
        PyErr_Format(PyExc_RuntimeError, "Mix_OpenAudio failed: %s", Mix_GetError());
        return NULL;
    }
    Py_RETURN_TRUE;
}

static PyObject* pyda_mixer_quit(PyObject* self, PyObject* args) {
    Mix_CloseAudio();
    Py_RETURN_NONE;
}

static PyObject* pyda_load_sound(PyObject* self, PyObject* args) {
    char* filepath;
    if (!PyArg_ParseTuple(args, "s", &filepath)) return NULL;
    Mix_Chunk* chunk = Mix_LoadWAV(filepath);
    if (!chunk) {
        PyErr_Format(PyExc_RuntimeError, "Failed to load sound %s: %s", filepath, Mix_GetError());
        return NULL;
    }
    return PyCapsule_New((void*)chunk, "Mix_Chunk", NULL);
}

static PyObject* pyda_play_sound(PyObject* self, PyObject* args) {
    PyObject* capsule;
    int loops = 0;
    if (!PyArg_ParseTuple(args, "O|i", &capsule, &loops)) return NULL;
    Mix_Chunk* chunk = (Mix_Chunk*)PyCapsule_GetPointer(capsule, "Mix_Chunk");
    if (!chunk) return NULL;
    int channel = Mix_PlayChannel(-1, chunk, loops);
    return PyLong_FromLong(channel);
}

static PyMethodDef MixerMethods[] = {
    {"init", pyda_mixer_init, METH_VARARGS, "Init mixer"},
    {"quit", pyda_mixer_quit, METH_NOARGS, "Quit mixer"},
    {"load_sound", pyda_load_sound, METH_VARARGS, "Load sound chunk"},
    {"play_sound", pyda_play_sound, METH_VARARGS, "Play sound chunk"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef mixer_module = {
    PyModuleDef_HEAD_INIT, "_mixer", "C extension for audio mixer", -1, MixerMethods
};

PyMODINIT_FUNC PyInit__mixer(void) {
    return PyModule_Create(&mixer_module);
}
