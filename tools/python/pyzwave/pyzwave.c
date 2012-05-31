#include <Python.h>

#include "pyzwave-testrtt.h"

int initialised = 0;

//#define DEBUGF(...)  printf(__VA_ARGS__)
#define DEBUGF(...) 


static PyObject* pyzwave_receive(PyObject *self, PyObject *args) {
  int wait_msec, len;

  if (!initialised) {
    PyErr_SetString(PyExc_IOError, "Call pyzwave.init first.");
    return NULL;
  }
  
  if (!PyArg_ParseTuple( args, "i", &wait_msec))
    return NULL;
  
  len = PyZwave_receive(wait_msec);
  if (len == 0) {
    Py_RETURN_NONE;
  } else {
    int i;

    DEBUGF("PYZWAVE: Received %i bytes: ", len);
    for (i=0; i<len; i++)
      DEBUGF("[%x] ", PyZwave_messagebuffer[i]);
    DEBUGF("\n");

    PyObject* message_list = PyList_New(0);
    for (i=0; i<len; i++) {
      PyList_Append(message_list, PyInt_FromLong((long)PyZwave_messagebuffer[i] & 0xFF));
    }
    PyObject* return_value_list = PyList_New(0);
    PyList_Append(return_value_list, PyInt_FromLong((long)PyZwave_src));
    PyList_Append(return_value_list, message_list);
    return return_value_list;
  }
}

static PyObject* pyzwave_init(PyObject *self, PyObject *args) {
  char *host;
  
  if (!PyArg_ParseTuple( args, "s", &host))
    return NULL;
  if (PyZwave_init(host) < 0) {
    PyErr_SetString(PyExc_IOError, "Call to zwave_init failed.");
    return NULL;
  }

  initialised = 1;
  Py_RETURN_NONE;
}

static PyObject* pyzwave_send(PyObject *self, PyObject *args) {
  int dest_address, i, length;
  PyObject *data;
  uint8_t buf[256];

  if (!initialised) {
    PyErr_SetString(PyExc_IOError, "Call pyzwave.init first.");
    return NULL;
  }
  
  if (!PyArg_ParseTuple( args, "iO!", &dest_address, &PyList_Type, &data))
    return NULL;

  length = (int)PyList_Size(data);
  if (length < 0) {
    PyErr_SetString(PyExc_TypeError, "Second argument must be a list of bytes");
    return NULL;
  }
  if (length > 256) {
    PyErr_SetString(PyExc_ValueError, "Payload cannot be longer than 256 bytes");
    return NULL;
  }
  
  for (i=0; i<length; i++) {
    PyObject *byteAsObject = PyList_GetItem(data, i); /* Can't fail */
    long byteAsLong = PyInt_AsLong(byteAsObject);
    if (byteAsLong > 255 || byteAsLong < 0) {
      PyErr_SetString(PyExc_ValueError, "Data must consist of single bytes");
      return NULL;
    }
    buf[i] = (uint8_t)byteAsLong;
  }

  DEBUGF("PYZWAVE: Sending %i bytes to %i: ", length, dest_address);
  for (i=0; i<length; i++) {
    DEBUGF("[%x] ", buf[i]);
  }
  if(PyZwave_send(dest_address, buf, length) == 0) {
    DEBUGF("\nPYZWAVE: Done.\n");
    Py_RETURN_NONE;
  } else {
    DEBUGF("pyzwave.c pyzwave_send: Call to ZW_senddata failed.\n");
    PyErr_SetString(PyExc_IOError, "Call to ZW_senddata failed.");
    return NULL;
  }
}

PyMethodDef methods[] = {
  {"init", pyzwave_init, METH_VARARGS, "Sets the IP address to connect to"},
  {"send", pyzwave_send, METH_VARARGS, "Sends a list of bytes to a node"},
  {"receive", pyzwave_receive, METH_VARARGS, "Receive data"},
  {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC 
initpyzwave() {
    (void) Py_InitModule("pyzwave", methods); 
}
