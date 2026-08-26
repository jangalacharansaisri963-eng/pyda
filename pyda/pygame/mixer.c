#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// Capsule destructor for safe memory cleanup of Mix_Chunk
static void free_mix_chunk(PyObject* capsule) {
    Mix_Chunk* chunk = (Mix_Chunk*)PyCapsule_GetPointer(capsule, "Mix_Chunk");
    if (chunk) {
        Mix_FreeChunk(chunk);
    }
}

// Capsule destructor for safe memory cleanup of Mix_Music
static void free_mix_music(PyObject* capsule) {
    Mix_Music* music = (Mix_Music*)PyCapsule_GetPointer(capsule, "Mix_Music");
    if (music) {
        Mix_FreeMusic(music);
    }
}

// --- 1. INITIALIZATION & SHUTDOWN ---
static PyObject* pyda_mixer_init(PyObject* self, PyObject* args) {
    int freq = 44100;
    Uint16 format = MIX_DEFAULT_FORMAT;
    int channels = 2;
    int chunksize = 1024;

    if (!PyArg_ParseTuple(args, "|ihii", &freq, &format, &channels, &chunksize)) return NULL;
    
    if (Mix_OpenAudio(freq, format, channels, chunksize) < 0) {
        PyErr_Format(PyExc_RuntimeError, "Mix_OpenAudio failed: %s", Mix_GetError());
        return NULL;
    }
    
    // Allocate default mixing channels
    Mix_AllocateChannels(16);
    Py_RETURN_TRUE;
}

static PyObject* pyda_mixer_quit(PyObject* self, PyObject* args) {
    while (Mix_Init(0)) {
        Mix_Quit();
    }
    Mix_CloseAudio();
    Py_RETURN_NONE;
}

// --- 2. SOUND CHUNK MANAGEMENT (SFX) ---
static PyObject* pyda_load_sound(PyObject* self, PyObject* args) {
    char* filepath;
    if (!PyArg_ParseTuple(args, "s", &filepath)) return NULL;
    
    Mix_Chunk* chunk = Mix_LoadWAV(filepath);
    if (!chunk) {
        PyErr_Format(PyExc_RuntimeError, "Failed to load sound '%s': %s", filepath, Mix_GetError());
        return NULL;
    }
    return PyCapsule_New((void*)chunk, "Mix_Chunk", free_mix_chunk);
}

static PyObject* pyda_play_sound(PyObject* self, PyObject* args) {
    PyObject* capsule;
    int loops = 0;
    int ticks = -1;
    if (!PyArg_ParseTuple(args, "O|ii", &capsule, &loops, &ticks)) return NULL;
    
    Mix_Chunk* chunk = (Mix_Chunk*)PyCapsule_GetPointer(capsule, "Mix_Chunk");
    if (!chunk) return NULL;
    
    int channel = (ticks < 0) ? Mix_PlayChannel(-1, chunk, loops) : Mix_PlayChannelTimed(-1, chunk, loops, ticks);
    return PyLong_FromLong(channel);
}

// --- 3. BACKGROUND MUSIC MANAGEMENT ---
static PyObject* pyda_load_music(PyObject* self, PyObject* args) {
    char* filepath;
    if (!PyArg_ParseTuple(args, "s", &filepath)) return NULL;
    
    Mix_Music* music = Mix_LoadMUS(filepath);
    if (!music) {
        PyErr_Format(PyExc_RuntimeError, "Failed to load music '%s': %s", filepath, Mix_GetError());
        return NULL;
    }
    return PyCapsule_New((void*)music, "Mix_Music", free_mix_music);
}

static PyObject* pyda_play_music(PyObject* self, PyObject* args) {
    PyObject* capsule;
    int loops = -1; // Default to infinite loop for background music
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

static PyObject* pyda_set_volume(PyObject* self, PyObject* args) {
    PyObject* capsule;
    int volume;
    if (!PyArg_ParseTuple(args, "Oi", &capsule, &volume)) return NULL;
    
    // Check if it's a sound chunk capsule
    if (PyCapsule_CheckExact(capsule) && strcmp(PyCapsule_GetName(capsule), "Mix_Chunk") == 0) {
        Mix_Chunk* chunk = (Mix_Chunk*)PyCapsule_GetPointer(capsule, "Mix_Chunk");
        if (chunk) {
            int set_vol = Mix_VolumeChunk(chunk, volume);
            return PyLong_FromLong(set_vol);
        }
    }
    Py_RETURN_NONE;
}

static PyMethodDef MixerMethods[] = {
    {"init", pyda_mixer_init, METH_VARARGS, "Initialize audio mixer subsystem"},
    {"quit", pyda_mixer_quit, METH_NOARGS, "Shut down audio mixer subsystem"},
    {"load_sound", pyda_load_sound, METH_VARARGS, "Load sound chunk from file"},
    {"play_sound", pyda_play_sound, METH_VARARGS, "Play sound chunk on first available channel"},
    {"load_music", pyda_load_music, METH_VARARGS, "Load background music stream"},
    {"play_music", pyda_play_music, METH_VARARGS, "Play background music stream"},
    {"stop_music", pyda_stop_music, METH_NOARGS, "Stop background music playback"},
    {"set_volume", pyda_set_volume, METH_VARARGS, "Set volume for sound chunks"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef mixer_module = {
    PyModuleDef_HEAD_INIT, 
    "_mixer", 
    "Zero-dependency native audio mixer extension", 
    -1, 
    MixerMethods
};

PyMODINIT_FUNC PyInit__mixer(void) {
    return PyModule_Create(&mixer_module);
}
