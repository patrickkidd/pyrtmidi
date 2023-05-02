/*
 #   Copyright (C) 2018 by Patrick Stinson                                 
 #   patrickkidd@gmail.com                                                   
 #                                                                         
 #   This program is free software; you can redistribute it and/or modify  
 #   it under the terms of the GNU General Public License as published by  
 #   the Free Software Foundation; either version 2 of the License, or     
 #   (at your option) any later version.                                   
 #                                                                         
 #   This program is distributed in the hope that it will be useful,       
 #   but WITHOUT ANY WARRANTY; without even the implied warranty of        
 #   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
 #   GNU General Public License for more details.                          
 #                                                                         
 #   You should have received a copy of the GNU General Public License     
 #   along with this program; if not, write to the                         
 #   Free Software Foundation, Inc.,                                       
 #   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  
 */


#include <Python.h>

#if PY_MAJOR_VERSION >= 3
#define PK_PYTHON3 1
#else
#define PK_PYTHON3 0
#endif

#if PK_PYTHON3
#define PK_BOOL(x) PyBool_FromLong(x)
#define PK_INT(x) PyLong_FromLong(x)
#define PK_FLOAT(x) PyFloat_FromDouble(x)
#define PK_STRING(x) PyUnicode_FromString(x)
#else
#define PK_BOOL(x) PyBool_FromLong(x)
#define PK_INT(x) PyInt_FromLong(x)
#define PK_FLOAT(x) PyFloat_FromDouble(x)
#define PK_STRING(x) PyString_FromString(x)
#endif

#ifndef Py_RETURN_NONE
#define Py_RETURN_NONE Py_INCREF(Py_None); return Py_None;
#endif

#ifdef __APPLE__
#define PK_WINDOWS 0
#endif
#ifdef __LINUX__
#define PK_WINDOWS 0
#endif
#ifdef __WINDOWS__
#define PK_WINDOWS 1
#endif

#if PK_WINDOWS
#include <windows.h>
//typedef void *HANDLE;
#endif
