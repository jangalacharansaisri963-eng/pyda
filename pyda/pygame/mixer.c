#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// --- MIXER INIT & QUIT ---
static PyObject* pyda_mixer_init(PyObject* self, PyObject* args) {
    int frequency = 44100;
    Uint16 format = AUDIO_S16SYS;
    int channels = 2;
    int chunksize = 1024;

    if (!PyArg_ParseTuple(args, "|iiii", &frequency, &format, &channels, &chunksize)) {
        return NULL;
    }

    if (Mix_OpenAudio(frequency, format, channels, chunksize) < 0) {
        PyErr_Format(PyExc_RuntimeError, "Mix_OpenAudio failed: %s", Mix_GetError());
        return NULL;
    }
    Py_RETURN_TRUE;
}

static PyObject* pyda_mixer_quit(PyObject* self, PyObject* args) {
    Mix_CloseAudio();
    Py_RETURN_NONE;
}

// --- SOUND EFFECTS (WAV) ---
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

// --- MUSIC STREAMING (MP3/OGG) ---
static PyObject* pyda_load_music(PyObject* self, PyObject* args) {
    char* filepath;
    if (!PyArg_ParseTuple(args, "s", &filepath)) return NULL;

    Mix_Music* music = Mix_LoadMUS(filepath);
    if (!music) {
        PyErr_Format(PyExc_RuntimeError, "Failed to load music %s: %s", filepath, Mix_GetError());
        return NULL;
    }
    return PyCapsule_New((void*)music, "Mix_Music", NULL);
}

static PyObject* pyda_play_music(PyObject* self, PyObject* args) {
    PyObject* capsule;
    int loops = -1;
    if (!PyArg_ParseTuple(args, "O|i", &capsule, &loops)) return NULL;

    Mix_Music* music = (Mix_Music*)PyCapsule_GetPointer(capsule, "Mix_Music");
    if (!music) return NULL;

    if (Mix_PlayMusic(music, loops) < 0) {
        PyErr_Format(PyExc_RuntimeError, "Failed to play music: %s", Mix_GetError());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* pyda_stop_music(PyObject* self, PyObject* args) {
    Mix_HaltMusic();
    Py_RETURN_NONE;
}

// --- METHOD TABLE ---
static PyMethodDef MixerMethods[] = {
    {"init", pyda_mixer_init, METH_VARARGS, "Initialize SDL_mixer"},
    {"quit", pyda_mixer_quit, METH_NOARGS, "Close SDL_mixer"},
    {"load_sound", pyda_load_sound, METH_VARARGS, "Load wav sound file"},
    {"play_sound", pyda_play_sound, METH_VARARGS, "Play sound chunk"},
    {"load_music", pyda_load_music, METH_VARARGS, "Load music stream file"},
    {"play_music", pyda_play_music, METH_VARARGS, "Play music stream"},
    {"stop_music", pyda_stop_music, METH_NOARGS, "Stop music stream"},
    {NULL, NULL, 0, NULL}
};

// --- MODULE DEFINITION ---
static struct PyModuleDef mixer_module = {
    PyModuleDef_HEAD_INIT,
    "_mixer",
    "High-performance C backend for pyda.pygame.mixer",
    -1,
    MixerMethods
};

PyMODINIT_FUNC PyInit__mixer(void) {
    return PyModule_Create(&mixer_module);
}
