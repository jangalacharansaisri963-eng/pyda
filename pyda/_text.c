#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// 1. Blazing Fast Substring Count (Zero Python overhead)
static PyObject* text_count_substring(PyObject* self, PyObject* args) {
    const char *haystack, *needle;
    if (!PyArg_ParseTuple(args, "ss", &haystack, &needle)) return NULL;
    
    long count = 0;
    size_t needle_len = strlen(needle);
    if (needle_len == 0) return PyLong_FromLong(0);
    
    const char *tmp = haystack;
    while ((tmp = strstr(tmp, needle)) != NULL) {
        count++;
        tmp += needle_len;
    }
    return PyLong_FromLong(count);
}

// 2. Fast Snake Case to CamelCase Converter
static PyObject* text_to_camel_case(PyObject* self, PyObject* args) {
    const char *input;
    if (!PyArg_ParseTuple(args, "s", &input)) return NULL;
    
    size_t len = strlen(input);
    char *result = (char*)malloc(len + 1);
    if (!result) return PyErr_NoMemory();
    
    size_t j = 0;
    int capitalize_next = 0;
    
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '_' || input[i] == '-') {
            capitalize_next = 1;
        } else {
            if (capitalize_next) {
                result[j++] = toupper((unsigned char)input[i]);
                capitalize_next = 0;
            } else {
                result[j++] = input[i];
            }
        }
    }
    result[j] = '\0';
    
    PyObject *py_res = PyUnicode_FromString(result);
    free(result);
    return py_res;
}

// 3. Reverse String in Place (Blazing Fast C pointer swap)
static PyObject* text_reverse(PyObject* self, PyObject* args) {
    const char *input;
    if (!PyArg_ParseTuple(args, "s", &input)) return NULL;
    
    size_t len = strlen(input);
    char *result = (char*)malloc(len + 1);
    if (!result) return PyErr_NoMemory();
    
    for (size_t i = 0; i < len; i++) {
        result[i] = input[len - 1 - i];
    }
    result[len] = '\0';
    
    PyObject *py_res = PyUnicode_FromString(result);
    free(result);
    return py_res;
}

// 4. Fast Word Counter (Splits by whitespace entirely in C)
static PyObject* text_word_count(PyObject* self, PyObject* args) {
    const char *input;
    if (!PyArg_ParseTuple(args, "s", &input)) return NULL;
    
    long count = 0;
    int in_word = 0;
    
    while (*input) {
        if (isspace((unsigned char)*input)) {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            count++;
        }
        input++;
    }
    return PyLong_FromLong(count);
}

// --- METHOD TABLE REGISTRATION ---
static PyMethodDef TextModuleMethods[] = {
    {"count_substring", text_count_substring, METH_VARARGS, "Fast substring occurrence counter"},
    {"to_camel_case", text_to_camel_case, METH_VARARGS, "Convert snake_case to CamelCase instantly"},
    {"reverse", text_reverse, METH_VARARGS, "Reverse a string natively"},
    {"word_count", text_word_count, METH_VARARGS, "Count words separated by whitespace"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef text_module = {
    PyModuleDef_HEAD_INIT,
    "_text",
    "Supercharged zero-dependency text manipulation engine",
    -1,
    TextModuleMethods
};

PyMODINIT_FUNC PyInit__text(void) {
    return PyModule_Create(&text_module);
}
