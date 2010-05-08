/*=========================================================================

  Program:   Visualization Toolkit
  Module:    PyVTKSpecialObject.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "PyVTKSpecialObject.h"
#include "vtkPythonUtil.h"

#include <vtksys/ios/sstream>

//--------------------------------------------------------------------
PyVTKSpecialType::PyVTKSpecialType(
    char *cname, char *cdocs[], PyMethodDef *cmethods, PyMethodDef *ccons,
    PyVTKSpecialMethods *smethods)
{
  this->classname = PyString_FromString(cname);
  this->docstring = vtkPythonUtil::BuildDocString(cdocs);
  this->methods = cmethods;
  this->constructors = ccons;
  this->copy_func = smethods->copy_func;
  this->delete_func = smethods->delete_func;
  this->print_func = smethods->print_func;
  this->compare_func = smethods->compare_func;
  this->hash_func = smethods->hash_func;
}

//--------------------------------------------------------------------
static PyObject *PyVTKSpecialObject_PyString(PyVTKSpecialObject *self)
{
  vtksys_ios::ostringstream os;
  self->vtk_info->print_func(os, self->vtk_ptr);
  const vtksys_stl::string &s = os.str();
  return PyString_FromStringAndSize(s.data(), s.size());
}

//--------------------------------------------------------------------
static PyObject *PyVTKSpecialObject_PyRepr(PyVTKSpecialObject *self)
{
  char buf[255];
  sprintf(buf,"<%s %s at %p>", self->ob_type->tp_name,
          PyString_AsString(self->vtk_info->classname), self);
  return PyString_FromString(buf);
}

//--------------------------------------------------------------------
static PyObject *PyVTKSpecialObject_PyGetAttr(PyVTKSpecialObject *self,
                                              PyObject *attr)
{
  char *name = PyString_AsString(attr);
  PyMethodDef *meth;

  if (name[0] == '_')
    {
    if (strcmp(name,"__name__") == 0)
      {
      Py_INCREF(self->vtk_info->classname);
      return self->vtk_info->classname;
      }
    if (strcmp(name,"__doc__") == 0)
      {
      Py_INCREF(self->vtk_info->docstring);
      return self->vtk_info->docstring;
      }
    if (strcmp(name,"__methods__") == 0)
      {
      meth = self->vtk_info->methods;
      PyObject *lst;
      int i, n;

      for (n = 0; meth && meth[n].ml_name; n++)
        {
        ;
        }

      if ((lst = PyList_New(n)) != NULL)
        {
        meth = self->vtk_info->methods;
        for (i = 0; i < n; i++)
          {
          PyList_SetItem(lst, i, PyString_FromString(meth[i].ml_name));
          }
        PyList_Sort(lst);
        }
      return lst;
      }

    if (strcmp(name,"__members__") == 0)
      {
      PyObject *lst;
      if ((lst = PyList_New(4)) != NULL)
        {
        PyList_SetItem(lst,0,PyString_FromString("__doc__"));
        PyList_SetItem(lst,1,PyString_FromString("__members__"));
        PyList_SetItem(lst,2,PyString_FromString("__methods__"));
        PyList_SetItem(lst,3,PyString_FromString("__name__"));
        }
      return lst;
      }
    }

  for (meth = self->vtk_info->methods; meth && meth->ml_name; meth++)
    {
    if (name[0] == meth->ml_name[0] && strcmp(name+1, meth->ml_name+1) == 0)
      {
      return PyCFunction_New(meth, (PyObject *)self);
      }
    }

  PyErr_SetString(PyExc_AttributeError, name);
  return NULL;
}

//--------------------------------------------------------------------
static void PyVTKSpecialObject_PyDelete(PyVTKSpecialObject *self)
{
  if (self->vtk_ptr)
    {
    self->vtk_info->delete_func(self->vtk_ptr);
    }
  self->vtk_ptr = NULL;
#if (PY_MAJOR_VERSION >= 2)
  PyObject_Del(self);
#else
  PyMem_DEL(self);
#endif
}

//--------------------------------------------------------------------
static long PyVTKSpecialObject_PyHash(PyVTKSpecialObject *self)
{
  if (self->vtk_ptr && self->vtk_info->hash_func)
    {
    long val = self->vtk_info->hash_func(self->vtk_ptr);
    if (val != -1)
      {
      return val;
      }
    }
// python 2.6
#if PY_VERSION_HEX >= 0x02060000
  return PyObject_HashNotImplemented((PyObject *)self);
#else
  PyErr_SetString(PyExc_TypeError, (char *)"object is not hashable");
  return -1;
#endif
}

//--------------------------------------------------------------------
static PyObject *PyVTKSpecialObject_PyRichCompare(
  PyVTKSpecialObject *self, PyVTKSpecialObject *o, int opid)
{
  if (self->vtk_ptr && self->vtk_info->compare_func)
    {
    int val = self->vtk_info->compare_func(self->vtk_ptr, o->vtk_ptr, opid);
    if (val == 0)
      {
      Py_INCREF(Py_False);
      return Py_False;
      }
    else if (val != -1)
      {
      Py_INCREF(Py_True);
      return Py_True;
      }
    }

  PyErr_SetString(PyExc_TypeError, (char *)"operation not available");
  return NULL;
// Returning "None" is also valid, but not as informative
#if PY_VERSION_HEX >= 0x02010000
//  Py_INCREF(Py_NotImplemented);
//  return Py_NotImplemented;
#else
//  Py_INCREF(Py_None);
//  return Py_None;
#endif
}

//--------------------------------------------------------------------
static PyTypeObject PyVTKSpecialObjectType = {
  PyObject_HEAD_INIT(&PyType_Type)
  0,
  (char*)"vtkspecialobject",             // tp_name
  sizeof(PyVTKSpecialObject),            // tp_basicsize
  0,                                     // tp_itemsize
  (destructor)PyVTKSpecialObject_PyDelete, // tp_dealloc
  (printfunc)0,                          // tp_print
  (getattrfunc)0,                        // tp_getattr
  (setattrfunc)0,                        // tp_setattr
  (cmpfunc)0,                            // tp_compare
  (reprfunc)PyVTKSpecialObject_PyRepr,   // tp_repr
  0,                                     // tp_as_number
  0,                                     // tp_as_sequence
  0,                                     // tp_as_mapping
  (hashfunc)PyVTKSpecialObject_PyHash,   // tp_hash
  (ternaryfunc)0,                        // tp_call
  (reprfunc)PyVTKSpecialObject_PyString, // tp_string
  (getattrofunc)PyVTKSpecialObject_PyGetAttr, // tp_getattro
  (setattrofunc)0,                       // tp_setattro
  0,                                     // tp_as_buffer
  Py_TPFLAGS_HAVE_RICHCOMPARE,           // tp_flags
  (char*)"vtkspecialobject - a vtk object not derived from vtkObjectBase.", // tp_doc
  0,                                     // tp_traverse
  0,                                     // tp_clear
  (richcmpfunc)PyVTKSpecialObject_PyRichCompare,  // tp_richcompare
  0,                                     // tp_weaklistoffset
  VTK_PYTHON_UTIL_SUPRESS_UNINITIALIZED
};

//--------------------------------------------------------------------
int PyVTKSpecialObject_Check(PyObject *obj)
{
  return (obj->ob_type == &PyVTKSpecialObjectType);
}

//--------------------------------------------------------------------
PyObject *PyVTKSpecialObject_New(char *classname, void *ptr, int copy)
{
#if (PY_MAJOR_VERSION >= 2)
  PyVTKSpecialObject *self = PyObject_New(PyVTKSpecialObject,
                                          &PyVTKSpecialObjectType);
#else
  PyVTKSpecialObject *self = PyObject_NEW(PyVTKSpecialObject,
                                          &PyVTKSpecialObjectType);
#endif

  PyVTKSpecialType *info = vtkPythonUtil::FindSpecialType(classname);

  if (info == 0)
    {
    char buf[256];
    sprintf(buf,"cannot create object of unknown type \"%s\"",classname);
    PyErr_SetString(PyExc_ValueError,buf);
    return NULL;
    }

  if (copy)
    {
    ptr = info->copy_func(ptr);
    }

  self->vtk_ptr = ptr;
  self->vtk_info = info;

  return (PyObject *)self;
}

//--------------------------------------------------------------------
PyObject *PyVTKSpecialType_New(
  PyMethodDef *newmethod, PyMethodDef *methods, PyMethodDef *constructors,
  char *classname, char *docstring[], PyVTKSpecialMethods *smethods)
{
  // Add this type to the special type map
  PyVTKSpecialType *info =
    vtkPythonUtil::AddSpecialTypeToMap(
      classname, docstring, methods, constructors, smethods);

  // Add the built docstring to the method
  newmethod->ml_doc = PyString_AsString(info->docstring);

  return PyCFunction_New(newmethod, Py_None);
}
