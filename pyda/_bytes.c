#define PY_SSIZE_T_CLEAN
#include <Python.h>

// Fast XOR encryption/decryption key application on byte buffers
static PyObject* bytes_xor_cipher(PyObject* self, PyObject* args) {
    const char *data;
    Py_ssize_t len;
    unsigned char key;
    if (!PyArg_ParseTuple(args, "s#b", &data, &len, &key)) return NULL;
    
    char *result = (char*)malloc(len);
    if (!result) return PyErr_NoMemory();
    
    for (Py_ssize_t i = 0; i < len; i++) {
        result[i] = data[i] ^ key;
    }
    
    PyObject *py_res = PyBytes_FromStringAndSize(result, len);
    free(result);
    return py_res;
}

// Fast calculation of simple checksum for data verification
static PyObject* bytes_simple_checksum(PyObject* self, PyObject* args) {
    const char *data;
    Py_ssize_t len;
    if (!PyArg_ParseTuple(args, "s#", &data, &len)) return NULL;
    
    unsigned int sum = 0;
    for (Py_ssize_t i = 0; i < len; i++) {
        sum += (unsigned char)data[i];
    }
    return PyLong_FromUnsignedLong(sum % 256);
}

static PyMethodDef BytesMethods[] = {
    {"xor_cipher", bytes_xor_cipher, METH_VARARGS, "Blazing-fast byte array XOR cipher"},
    {"simple_checksum", bytes_simple_checksum, METH_VARARGS, "Compute fast 8-bit data checksum"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef bytes_module = {
    PyModuleDef_HEAD_INIT, "_bytes", "Fast binary data manipulation", -1, BytesMethods
};

PyMODINIT_FUNC PyInit__bytes(void) { return PyModule_Create(&bytes_module); }
