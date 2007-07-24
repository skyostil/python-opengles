/**
 * ====================================================================
 * gles_util.h
 *
 * Copyright (c) 2006 Nokia Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "glesmodule.h"

static GLES_Arrays *arrayData;

int gles_PySequence_Dimension(PyObject *seq) {
  assert(PySequence_Check(seq));
  PyObject *item = PySequence_GetItem(seq, 0);
  if( !PySequence_Check(item) ) {
    return -1;
  }
  // The dimension is determined from the lenght of the first element
  int dimension = PySequence_Length(item);
  return dimension;
}

/**
 * Convert a freely structured sequence into a flat sequence
 * Also converts data to the given type.
 *
 * @param type Can be one of: GL_FLOAT, GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_FIXED.
 * @param source A Python sequence where the data is read from.
 * @param target A pointer to an existing target sequence. If NULL, a new sequence is created.
 * @return A collapsed sequence.
 */
PyObject *gles_PySequence_Collapse(GLenum type, PyObject *source, PyObject *target)
{
  int count;
  int i;
  PyObject *k;
  
  // We need a sequence to operate.
  if(!PySequence_Check(source)) {
    PyErr_SetString(PyExc_TypeError, "Expecting a sequence");
    return NULL;
  }
  
  // Make a new list if not supplied by caller.
  if(target == NULL) {
    target = PyList_New(0);
    if(!target) {
      assert(PyErr_Occurred() != NULL);
      return NULL;
    }
  }
  
  count = PySequence_Length(source);
  for(i=0; i < count; i++) {
    k = PySequence_GetItem(source, i);
    // See if its a sequence. Strings are sequences too, but we should ignore them here.
    if(PySequence_Check(k) && !PyString_Check(k)) {
      // Call a new function go get the contents of the sequence.
      if(gles_PySequence_Collapse(type, k, target) == NULL) {
        // There should be an exception
        assert(PyErr_Occurred() != NULL);
        return NULL;
      }
    } else {
      assert(k != NULL);
      if(!PyNumber_Check(k)) {
        PyErr_SetString(PyExc_TypeError, "Expecting a number");
        return NULL;
      }
      
      // Try to convert the object into desired type.
      // Using the number interface we can handle all
      // types that can be converted into a number.
      PyObject *t=NULL;
      switch(type) {
        case GL_FLOAT:
          k = PyNumber_Float(k);
          if(k==NULL) {
            // Something went wrong with the conversion.
            assert(PyErr_Occurred() != NULL);
            return NULL;
          }
          t = Py_BuildValue("f", PyFloat_AsDouble(k));
          break;
        case GL_SHORT:
          k = PyNumber_Int(k);
          if(k==NULL) {
            assert(PyErr_Occurred() != NULL);
            return NULL;
          }
          t = Py_BuildValue("h", PyInt_AsLong(k));
          break;
        case GL_UNSIGNED_SHORT:
          k = PyNumber_Int(k);
          if(k==NULL) {
            assert(PyErr_Occurred() != NULL);
            return NULL;
          }
          t = Py_BuildValue("H", PyInt_AsLong(k));
          break;
        case GL_BYTE:
          k = PyNumber_Int(k);
          if(k==NULL) {
            assert(PyErr_Occurred() != NULL);
            return NULL;
          }
          t = Py_BuildValue("b", PyInt_AsLong(k));
          break;
        case GL_UNSIGNED_BYTE:
          k = PyNumber_Int(k);
          if(k==NULL) {
            assert(PyErr_Occurred() != NULL);
            return NULL;
          }
          t = Py_BuildValue("B", PyInt_AsLong(k));
          break;
        case GL_FIXED:
          k = PyNumber_Int(k);
          if(k==NULL) {
            assert(PyErr_Occurred() != NULL);
            return NULL;
          }
          t = Py_BuildValue("i", PyInt_AsLong(k));
          break;
        default:
          PyErr_SetString(PyExc_TypeError, "Invalid type for conversion");
          return NULL;
      }
      if(t == NULL) {
        assert(PyErr_Occurred() != NULL);
        return NULL;
      }
      // Add item to target.
      PyList_Append(target, t);
    }
  }
  
  return target;
}

/**
 * Place given array into internal memory structure
 *
 * If there already is something in memory, it will be freed,
 * if the memory does not belong to an array object
 *
 * @param targetarr Specifies where the given array should be placed in internal memory. Can be one of: GL_VERTEX_ARRAY, GL_NORMAL_ARRAY, GL_COLOR_ARRAY, GL_TEXTURE_COORD_ARRAY.
 * @param *ptr Pointer to an array.
 * @param object Python object of type gles.array. Can be NULL, in which case the memory will be freed when the next array is placed in the same location.
 * @return The given pointer
*/
void *gles_assign_array(GLenum targetarr, void *ptr, array_object *object) {
  //GLES_Arrays *arrays = (GLES_Arrays*)Dll::Tls();
  gles_array_t *arrptr;
  
  assert(arrayData!=NULL);
  
  // Get the array from internal struct.
  switch(targetarr) {
    case GL_VERTEX_ARRAY:
      arrptr = &(arrayData->vertex);
#ifdef DEBUG_GLES
      DEBUGMSG("Assigning GL_VERTEX_ARRAY\n");
#endif
      break;
    case GL_NORMAL_ARRAY:
      arrptr = &(arrayData->normal);
#ifdef DEBUG_GLES
      DEBUGMSG("Assigning GL_NORMAL_ARRAY\n");
#endif
      break;
    case GL_COLOR_ARRAY:
      arrptr = &(arrayData->color);
#ifdef DEBUG_GLES
      DEBUGMSG("Assigning GL_COLOR_ARRAY\n");
#endif
      break;
    case GL_TEXTURE_COORD_ARRAY:
      arrptr = &(arrayData->texcoord);
#ifdef DEBUG_GLES
      DEBUGMSG("Assigning GL_TEXTURE_COORD_ARRAY\n");
#endif
      break;
#ifdef GL_OES_VERSION_1_1
    case GL_MATRIX_INDEX_ARRAY_OES:
      arrptr = &(arrayData->matrix);
#ifdef DEBUG_GLES
      DEBUGMSG("Assigning GL_MATRIX_INDEX_ARRAY_OES\n");
#endif
      break;
    case GL_POINT_SIZE_ARRAY_OES:
      arrptr = &(arrayData->pointsize);
#ifdef DEBUG_GLES
      DEBUGMSG("Assigning GL_POINT_SIZE_ARRAY_OES\n");
#endif
      break;
    case GL_WEIGHT_ARRAY_OES:
      arrptr = &(arrayData->weight);
#ifdef DEBUG_GLES
      DEBUGMSG("Assigning GL_WEIGHT_ARRAY_OES\n");
#endif
      break;
#endif
    default:
      return NULL;
  }
  // Set the previous array free
  gles_free_array(targetarr);
  
  // See if we got a new object
  if(object != NULL) {
    arrptr->obj = (PyObject*)object;
    Py_XINCREF(object);
    arrptr->ptr = object->arrdata;
  } // Otherwise just take the pointer into custody
  else {
    arrptr->obj = NULL;
    arrptr->ptr = ptr;
  }
  
  return arrptr->ptr;
}

/**
 * Convert a Python sequence to a float array
 * 
 * @param seq A Python sequence object
 * @return A pointer to a C array of GLfloat values
 */
GLfloat *gles_PySequence_AsGLfloatArray(PyObject *seq) {
  int count;
  GLfloat *dataptr;
  int i;
  PyObject *item;
  
  if(! PySequence_Check(seq)) {
    PyErr_SetString(PyExc_TypeError, "Invalid type");
    return NULL;
  }
  
  count = PySequence_Length(seq);
  dataptr = (GLfloat*)gles_alloc( sizeof(GLfloat)*count);
  if(dataptr == NULL) {
    return (GLfloat*)PyErr_NoMemory();
  }
  
  for( i = 0; i < count; i++ ) {
		item = PySequence_GetItem(seq, i);
    if( !PyFloat_Check(item) ) {
      PyErr_SetString(PyExc_TypeError, "Expecting a float");
      return NULL;
    }
    dataptr[i] = (GLfloat)PyFloat_AsDouble(item);
	}
  
  return dataptr;
}

/**
 * Convert a Python sequence into unsigned short array
 *
 * @param seq A Python sequence object
 * @return A pointer to a C array of GLushort values
 */
GLushort *gles_PySequence_AsGLushortArray(PyObject *seq) {
  int count;
  GLushort *dataptr;
  int i;
  PyObject *item;
  
  if(! PySequence_Check(seq)) {
    PyErr_SetString(PyExc_TypeError, "Expecting a sequence");
    return NULL;
  }
  
  count = PySequence_Length(seq);
  dataptr = (GLushort*)gles_alloc( sizeof(GLushort)*count);
  if(dataptr == NULL) {
    return (GLushort*)PyErr_NoMemory();
  }
  
  for( i = 0; i < count; i++ ) {
		item = PySequence_GetItem(seq, i);
    if( !PyInt_Check(item) ) {
      gles_free(dataptr);
      PyErr_SetString(PyExc_TypeError, "Expecting an integer");
      return NULL;
    }
    dataptr[i] = (GLushort)PyInt_AsLong(item);
	}
  
  return dataptr;
}

/**
 * Convert a Python sequence into short array
 *
 * @param seq A Python sequence object
 * @return A pointer to a C array of GLshort values
 */
GLshort *gles_PySequence_AsGLshortArray(PyObject *seq) {
  int count;
  GLshort *dataptr;
  int i;
  PyObject *item;
  
  if(! PySequence_Check(seq)) {
    PyErr_SetString(PyExc_TypeError, "Expecting a sequence");
    return NULL;
  }
  
  count = PySequence_Length(seq);
  dataptr = (GLshort*)gles_alloc( sizeof(GLshort)*count);
  if(dataptr == NULL) {
    return (GLshort*)PyErr_NoMemory();
  }
  
  for( i = 0; i < count; i++ ) {
		item = PySequence_GetItem(seq, i);
    if( !PyInt_Check(item) ) {
      gles_free(dataptr);
      PyErr_SetString(PyExc_TypeError, "Expecting an integer");
      return NULL;
    }
    dataptr[i] = (GLshort)PyInt_AsLong(item);
	}
  
  return dataptr;
}

/**
 * Convert a Python sequence into a byte array
 *
 * @param seq A Python sequence object
 * @return A pointer to a C array of GLbyte values
 */
GLbyte *gles_PySequence_AsGLbyteArray(PyObject *seq) {
  int count;
  GLbyte *dataptr;
  int i;
  PyObject *item;
  
  if(! PySequence_Check(seq)) {
    PyErr_SetString(PyExc_TypeError, "Invalid type");
    return NULL;
  }
  
  count = PySequence_Length(seq);
  dataptr = (GLbyte*)gles_alloc( sizeof(GLbyte)*count);
  if(dataptr == NULL) {
    return (GLbyte*)PyErr_NoMemory();
  }
  
  for( i = 0; i < count; i++ ) {
		item = PySequence_GetItem(seq, i);
    if( !PyInt_Check(item) ) {
      // Free the indices.
      gles_free(dataptr);
      PyErr_SetString(PyExc_TypeError, "Expecting an integer");
      return NULL;
    }
    dataptr[i] = (GLbyte)PyInt_AsLong(item);
	}
  
  return dataptr;
}

/**
 * Converts a Python sequence into unsigned byte array
 *
 * @param seq A Python sequence object
 * @return A pointer to a C array of GLubyte values
 */
GLubyte *gles_PySequence_AsGLubyteArray(PyObject *seq) {
  int count;
  GLubyte *dataptr;
  int i;
  PyObject *item;
  
  if(! PySequence_Check(seq)) {
    PyErr_SetString(PyExc_TypeError, "Invalid type");
    return NULL;
  }
  
  count = PySequence_Length(seq);
  dataptr = (GLubyte*)gles_alloc( sizeof(GLubyte)*count);
  if(dataptr == NULL) {
    return (GLubyte*)PyErr_NoMemory();
  }
  
    for( i = 0; i < count; i++ ) {
		item = PySequence_GetItem(seq, i);
    if( !PyInt_Check(item) ) {
      // Free the indices
      gles_free(dataptr);
      PyErr_SetString(PyExc_TypeError, "Expecting an integer");
      return NULL;
    }
    dataptr[i] = (GLubyte)PyInt_AsLong(item);
	}
  
  return dataptr;
}

/**
 * Converts a Python sequence into int array
 *
 * @param seq A Python sequence object
 * @return A pointer to a C array of GLint values
 */
GLint *gles_PySequence_AsGLintArray(PyObject *seq) {
  int count;
  GLint *dataptr;
  int i;
  PyObject *item;
  
  if(! PySequence_Check(seq)) {
    PyErr_SetString(PyExc_TypeError, "Invalid type");
    return NULL;
  }
  
  count = PySequence_Length(seq);
  dataptr = (GLint*)gles_alloc( sizeof(GLint)*count);
  if(dataptr == NULL) {
    return (GLint*)PyErr_NoMemory();
  }
  
  for( i = 0; i < count; i++ ) {
		item = PySequence_GetItem(seq, i);
    if( !PyInt_Check(item) ) {
      // Free the indices
      gles_free(dataptr);
      PyErr_SetString(PyExc_TypeError, "Expecting an integer");
      return NULL;
    }
    dataptr[i] = (GLint)PyInt_AsLong(item);
	}
  
  return dataptr;
}

/**
 * Converts a Python sequence into unsigned int array
 *
 * @param seq A Python sequence object
 * @return A pointer to a C array of GLuint values
 */
GLuint *gles_PySequence_AsGLuintArray(PyObject *seq) {
  int count;
  GLuint *dataptr;
  int i;
  PyObject *item;
  
  if(! PySequence_Check(seq)) {
    PyErr_SetString(PyExc_TypeError, "Invalid type");
    return NULL;
  }
  
  count = PySequence_Length(seq);
  dataptr = (GLuint*)gles_alloc( sizeof(GLuint)*count);
  if(dataptr == NULL) {
    return (GLuint*)PyErr_NoMemory();
  }
  
    for( i = 0; i < count; i++ ) {
		item = PySequence_GetItem(seq, i);
    if( !PyInt_Check(item) ) {
      // Free the indices
      gles_free(dataptr);
      PyErr_SetString(PyExc_TypeError, "Expecting an integer");
      return NULL;
    }
    dataptr[i] = (GLuint)PyInt_AsLong(item);
	}
  
  return dataptr;
}

/**
 * Converts a Python sequence into fixed array
 *
 * @param seq A Python sequence object
 * @return A pointer to a C array of GLfixed values
 */
GLfixed *gles_PySequence_AsGLfixedArray(PyObject *seq) {
  int count;
  GLfixed *dataptr;
  int i;
  PyObject *item;
  
  if(! PySequence_Check(seq)) {
    PyErr_SetString(PyExc_TypeError, "Invalid type");
    return NULL;
  }
  
  count = PySequence_Length(seq);
  dataptr = (GLfixed*)gles_alloc( sizeof(GLfixed)*count);
  if(dataptr == NULL) {
    return (GLfixed*)PyErr_NoMemory();
  }
  
    for( i = 0; i < count; i++ ) {
		item = PySequence_GetItem(seq, i);
    if( !PyInt_Check(item) ) {
      // Free the indices
      gles_free(dataptr);
      PyErr_SetString(PyExc_TypeError, "Expecting an integer");
      return NULL;
    }
    dataptr[i] = (GLfixed)PyInt_AsLong(item);
	}
  
  return dataptr;
}

/**
 * Frees all the internally used array memory
 *
 */
void gles_uninit_arrays() {
    //GLES_Arrays *arrayData;
    //arrayData = (GLES_Arrays*)Dll::Tls();
    if(arrayData != NULL) {
      gles_free_array(GL_VERTEX_ARRAY);
      gles_free_array(GL_NORMAL_ARRAY);
      gles_free_array(GL_COLOR_ARRAY);
      gles_free_array(GL_TEXTURE_COORD_ARRAY);
#ifdef GL_OES_VERSION_1_1
      gles_free_array(GL_MATRIX_INDEX_ARRAY_OES);
      gles_free_array(GL_POINT_SIZE_ARRAY_OES);
      gles_free_array(GL_WEIGHT_ARRAY_OES);
#endif
      gles_free(arrayData);
    }
}

/**
 * Initializes internal arrays used for glXXXPointer data
 *
 */
void gles_init_arrays() {
    
    
    arrayData = (GLES_Arrays*)gles_alloc( sizeof(GLES_Arrays) );
    if(arrayData == NULL) {
      PyErr_NoMemory();
      return;
    }
    
    // Reset array pointers
    arrayData->vertex.ptr = NULL;
    arrayData->vertex.obj = NULL;
    
    arrayData->color.ptr = NULL;
    arrayData->color.obj = NULL;
    
    arrayData->normal.ptr = NULL;
    arrayData->normal.obj = NULL;
    
    arrayData->texcoord.ptr = NULL;
    arrayData->texcoord.obj = NULL;
    
#ifdef GL_OES_VERSION_1_1
    arrayData->matrix.ptr = NULL;
    arrayData->matrix.obj = NULL;
    
    arrayData->pointsize.ptr = NULL;
    arrayData->pointsize.obj = NULL;
    
    arrayData->weight.ptr = NULL;
    arrayData->weight.obj = NULL;
#endif
}

/**
 * Frees a specified array from internal memory
 *
 * @param arrtype Which array to free. Can be one of: GL_VERTEX_ARRAY, GL_NORMAL_ARRAY, GL_COLOR_ARRAY or GL_TEXTURE_COORD_ARRAY.
*/
void gles_free_array(GLenum arrtype) {
  gles_array_t *arr=NULL;
  switch(arrtype) {
    case GL_VERTEX_ARRAY:
      arr = &(arrayData->vertex);
#ifdef DEBUG_GLES
      DEBUGMSG("gles_free_array() (VERTEX)");
#endif
      break;
    case GL_NORMAL_ARRAY:
      arr = &(arrayData->normal);
#ifdef DEBUG_GLES
      DEBUGMSG("gles_free_array() (NORMAL)");
#endif
      break;
    case GL_COLOR_ARRAY:
      arr = &(arrayData->color);
#ifdef DEBUG_GLES
      DEBUGMSG("gles_free_array() (COLOR)");
#endif
      break;
    case GL_TEXTURE_COORD_ARRAY:
      arr = &(arrayData->texcoord);
#ifdef DEBUG_GLES
      DEBUGMSG("gles_free_array() (TEXTURE_COORD)");
#endif
      break;
#ifdef GL_OES_VERSION_1_1
    case GL_MATRIX_INDEX_ARRAY_OES:
      arr = &(arrayData->matrix);
#ifdef DEBUG_GLES
      DEBUGMSG("gles_free_array() (GL_MATRIX_INDEX_ARRAY_OES)");
#endif
      break;
    case GL_POINT_SIZE_ARRAY_OES:
      arr = &(arrayData->pointsize);
#ifdef DEBUG_GLES
      DEBUGMSG("gles_free_array() (GL_POINT_SIZE_ARRAY_OES)");
#endif
      break;
    case GL_WEIGHT_ARRAY_OES:
      arr = &(arrayData->weight);
#ifdef DEBUG_GLES
      DEBUGMSG("gles_free_array() (GL_WEIGHT_ARRAY_OES)");
#endif
      break;
#endif
    default:
      return;
  }
  
  if(arr->obj != NULL) {
    Py_XDECREF(arr->obj);
  } else if(arr->ptr != NULL) {
    gles_free(arr->ptr);
  }
  arr->ptr=NULL;
}

/**
 * Checks if an OpenGL error has occurred, and sets an exception if necessary.
 *
 * @return 1 if an OpenGL error has occurred, 0 otherwise
 */
unsigned int gles_check_error() {
  GLenum error = GL_NO_ERROR;
  
  error = glGetError();
  if(error != GL_NO_ERROR) {
    PyObject *exc;
    exc = PyErr_NewException("gles.GLerror", NULL, NULL);
    switch(error) {
      case GL_INVALID_ENUM:
        PyErr_Format(exc, "Invalid enum [Errno %x]", error);
#ifdef DEBUG_GLES
        DEBUGMSG1("gles_check_error(): Invalid enum [Errno %x]", error);
#endif
        break;
      case GL_INVALID_VALUE:
        PyErr_Format(exc, "Invalid value [Errno %x]", error);
#ifdef DEBUG_GLES
        DEBUGMSG1("gles_check_error(): Invalid value [Errno %x]", error);
#endif
        break;
      case GL_INVALID_OPERATION:
        PyErr_Format(exc, "Invalid operation [Errno %x]", error);
#ifdef DEBUG_GLES
        DEBUGMSG1("gles_check_error(): Invalid operation [Errno %x]", error);
#endif
        break;
      case GL_STACK_OVERFLOW:
        PyErr_Format(exc, "Stack overflow [Errno %x]", error);
#ifdef DEBUG_GLES
        DEBUGMSG1("gles_check_error(): Stack overflow [Errno %x]", error);
#endif
        break;
      case GL_STACK_UNDERFLOW:
        PyErr_Format(exc, "Stack underflow [Errno %x]", error);
#ifdef DEBUG_GLES
        DEBUGMSG1("gles_check_error(): Stack underflow [Errno %x]", error);
#endif
        break;
      case GL_OUT_OF_MEMORY:
        PyErr_NoMemory();
#ifdef DEBUG_GLES
        DEBUGMSG("gles_check_error(): Out of memory");
#endif
        break;
      default:
        PyErr_Format(PyExc_Exception, "Unknown error: %x", error );
#ifdef DEBUG_GLES
        DEBUGMSG1("gles_check_error(): Unknown error: %x", error);
#endif
        break;
    }
    return 1;
  }
  return 0;
}

/**
 * Helper function for allocating memory
 *
 * @param size Size of desired memory block
 * @return A pointer to a newly allocated block of memory
 */
void *gles_alloc(size_t size) {
  return malloc(size);
}

/**
 * Helper function for freeing allocated memory
 *
 * @param ptr A pointer to a block of memory to be freed
 */
void gles_free(void *ptr) {
  free(ptr);
}
