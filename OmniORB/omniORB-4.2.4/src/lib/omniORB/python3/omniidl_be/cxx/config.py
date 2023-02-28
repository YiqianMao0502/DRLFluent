 # -*- python -*-
#                           Package   : omniidl
# config.py                 Created on: 2000/10/8
#                           Author    : David Scott (djs)
#
#    Copyright (C) 2002-2011 Apasphere Ltd
#    Copyright (C) 1999 AT&T Laboratories Cambridge
#
#  This file is part of omniidl.
#
#  omniidl is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see http://www.gnu.org/licenses/
#
# Description:
#
#   Global state of the C++ backend is stored here

state = {
    # Name of this program
    'Program Name':          'omniidl (C++ backend)',
    # Relevant omniORB C++ library version
    'Library Version':       'omniORB_4_2',
    # Suffix of generated header file
    'HH Suffix':             '.hh',
    # Suffix of generated Skeleton file
    'SK Suffix':             'SK.cc',
    # Suffix of generated Dynamic code
    'DYNSK Suffix':          'DynSK.cc',
    # Suffix of example interface implementation code
    'IMPL Suffix':           '_i.cc',

    # Are we in "fragment" mode?
    'Fragment':              0,
    # In fragment mode, suffix of definitions file
    '_DEFS Fragment':        '_defs',
    # In fragment mode, suffix of file containing operators (eg <<)
    '_OPERS Fragment':       '_operators',
    # In fragment mode, suffix of file containing POA code
    '_POA Fragment':         '_poa',

    # Private prefix for internal names
    'Private Prefix':        '_0RL',
    # Prefix used to avoid clashes with C++ keywords
    'Reserved Prefix':       '_cxx_',
    # Base name of file being processed
    'Basename':              None,
    # Directory name of file being processed
    'Directory':             None,
    # Do we generate code for TypeCodes and Any?
    'Typecode':              0,
    # Do we splice reopened modules together into one large chunk?
    # (not guaranteed to always work)
    'Splice Modules':        0,
    # Do we generate example code implementing all of the interfaces
    # found in the input IDL?
    'Example Code':          0,
    # Do we generate normal (non-flattened) tie templates?
    'Normal Tie':            0,
    # Do we generate flattened tie templates?
    'Flattened Tie':         0,
    # Do we generate BOA compatible skeleton classes?
    'BOA Skeletons':         0,
    # Do we generate old CORBA 2.1 signatures for skeletons?
    'Old Signatures':        0,
    # Do we preserve the #include'd IDL path name in the generated
    # header (eg #include <A/B.idl> -> #include <A/B.hh>?
    'Keep Include Path':     0,
    # Do we #include files using double-quotes rather than
    # angled brackets (the default)
    'Use Quotes':            0,

    # Do we make all the objref methods virtual?
    'Virtual Objref Methods':0,

    # Do we use the impl mapping for objref methods?
    'Impl Mapping':          0,

    # Are #included files output inline with the main output?
    'Inline Includes':       0,

    # Generate local servant shortcut code?
    'Shortcut':              0,

    # Extra ifdefs for stubs in dlls?
    'DLLIncludes':           0,

    # Prefix for include guard in generated header
    'GuardPrefix':           '',

    # Generate AMI?
    'AMI':                   0,

    # Are we in DEBUG mode?
    'Debug':                 0,

    }
