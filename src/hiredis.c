#include "hiredis.h"
#include "reader.h"
#include "pack.h"

static int hiredis_ModuleTraverse(PyObject *m, visitproc visit, void *arg) {
    Py_VISIT(GET_STATE(m)->HiErr_Base);
    Py_VISIT(GET_STATE(m)->HiErr_ProtocolError);
    Py_VISIT(GET_STATE(m)->HiErr_ReplyError);
    return 0;
}

static int hiredis_ModuleClear(PyObject *m) {
    Py_CLEAR(GET_STATE(m)->HiErr_Base);
    Py_CLEAR(GET_STATE(m)->HiErr_ProtocolError);
    Py_CLEAR(GET_STATE(m)->HiErr_ReplyError);
    return 0;
}

static PyObject*
py_pack_command(PyObject* self, PyObject* cmd)
{
    return pack_command(cmd);
}

PyDoc_STRVAR(pack_command_doc, "Pack a series of arguments into the Redis protocol");

PyMethodDef pack_command_method = {
    "pack_command",                 /* The name as a C string. */
    (PyCFunction) py_pack_command,  /* The C function to invoke. */
    METH_O,                         /* Flags telling Python how to invoke */
    pack_command_doc,               /* The docstring as a C string. */
};


PyMethodDef methods[] = {
    {"pack_command", (PyCFunction) py_pack_command, METH_O, pack_command_doc},
    {NULL},
};

/* Keep pointer around for other classes to access the module state. */
PyObject *mod_hiredis;

/* Multi-phase initialization exec function */
static int hiredis_exec(PyObject *m) {
    if (PyType_Ready(&hiredis_ReaderType) < 0) {
        return -1;
    }

    PushNotificationType.tp_base = &PyList_Type;
    if (PyType_Ready(&PushNotificationType) < 0) {
        return -1;
    }

    /* Store module pointer for other classes to access state */
    mod_hiredis = m;

    /* Setup custom exceptions */
    GET_STATE(m)->HiErr_Base =
        PyErr_NewException(MOD_HIREDIS ".HiredisError", PyExc_Exception, NULL);
    if (GET_STATE(m)->HiErr_Base == NULL) {
        return -1;
    }
    GET_STATE(m)->HiErr_ProtocolError =
        PyErr_NewException(MOD_HIREDIS ".ProtocolError", GET_STATE(m)->HiErr_Base, NULL);
    if (GET_STATE(m)->HiErr_ProtocolError == NULL) {
        return -1;
    }
    GET_STATE(m)->HiErr_ReplyError =
        PyErr_NewException(MOD_HIREDIS ".ReplyError", GET_STATE(m)->HiErr_Base, NULL);
    if (GET_STATE(m)->HiErr_ReplyError == NULL) {
        return -1;
    }

    if (PyModule_AddObjectRef(m, "HiredisError", GET_STATE(m)->HiErr_Base) < 0) {
        return -1;
    }
    if (PyModule_AddObjectRef(m, "ProtocolError", GET_STATE(m)->HiErr_ProtocolError) < 0) {
        return -1;
    }
    if (PyModule_AddObjectRef(m, "ReplyError", GET_STATE(m)->HiErr_ReplyError) < 0) {
        return -1;
    }

    if (PyModule_AddObjectRef(m, "Reader", (PyObject *)&hiredis_ReaderType) < 0) {
        return -1;
    }

    if (PyModule_AddObjectRef(m, "PushNotification", (PyObject *)&PushNotificationType) < 0) {
        return -1;
    }

    return 0;
}

static PyModuleDef_Slot hiredis_slots[] = {
    {Py_mod_exec, hiredis_exec},
#ifdef Py_GIL_DISABLED
    {Py_mod_gil, Py_MOD_GIL_NOT_USED},
#endif
    {0, NULL}
};

static struct PyModuleDef hiredis_ModuleDef = {
    PyModuleDef_HEAD_INIT,
    MOD_HIREDIS,
    NULL,
    sizeof(struct hiredis_ModuleState), /* m_size */
    methods, /* m_methods */
    hiredis_slots, /* m_slots */
    hiredis_ModuleTraverse, /* m_traverse */
    hiredis_ModuleClear, /* m_clear */
    NULL /* m_free */
};

PyMODINIT_FUNC PyInit_hiredis(void)
{
    return PyModuleDef_Init(&hiredis_ModuleDef);
}
