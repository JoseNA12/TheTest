#include <stdio.h>
#include </usr/include/python2.7/Python.h>
#define ARCHIVO_PYTHON "json_handler" 


// Initialize the Python Interpreter
void initPython() {
    Py_Initialize();
    PyObject *sysmodule = PyImport_ImportModule("sys");
    PyObject *syspath = PyObject_GetAttrString(sysmodule, "path");
    PyList_Append(syspath, PyString_FromString("."));
    Py_DECREF(syspath);
    Py_DECREF(sysmodule);
}

// Finalize the Python Interpreter
void terminarPython() {
	Py_Finalize();
}

// https://stackoverflow.com/questions/37687770/embedding-python-in-c-segfault/37689456
// https://stackoverflow.com/questions/25316485/call-a-python-function-from-within-a-c-program

// Llama función de python y retorna el PyObject obtenido
PyObject* llamarFuncion(char* funcion) {
    // Archivo python
    PyObject *mymodule = PyImport_ImportModule(ARCHIVO_PYTHON);
    assert(mymodule != NULL);

    // Funcion python a llamar
    PyObject *myfunc = PyObject_GetAttrString(mymodule, funcion);
    assert(myfunc != NULL);

    // Resultado obtenido
    PyObject *result = PyObject_CallObject(myfunc, NULL);
  //  assert(result != NULL);

    // Limpiar memoria
    Py_DECREF(myfunc);
    Py_DECREF(mymodule);
    return result;
}

