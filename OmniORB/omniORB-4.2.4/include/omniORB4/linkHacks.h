// -*- Mode: C++; -*-
//                            Package   : omniORB2
// linkHacks.h                Created on: 26/07/2001
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories, Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library. If not, see http://www.gnu.org/licenses/
//
//
// Description:
//	*** PROPRIETARY INTERFACE ***
//

// Macros to alleviate the sheer ugliness of forcing modules with
// static initialisers to run, even when statically linked.

#ifndef __LINKHACKS_H__
#define __LINKHACKS_H__

//
// OMNI_EXPORT_LINK_FORCE_SYMBOL exports a symbol from a module
//

#if defined(__vxWorks__)

#define OMNI_EXPORT_LINK_FORCE_SYMBOL(modname) \
  int _omni_ ## modname ## _should_be_linked_but_is_not_; \
  void _dummy_ ## modname ## _workaround_for_bug_in_munch_2_cdtor_c_ () {}

#else

#define OMNI_EXPORT_LINK_FORCE_SYMBOL(modname) \
  int _omni_ ## modname ## _should_be_linked_but_is_not_ = 0

#endif


//
// OMNI_FORCE_LINK forces linking with a module
//

#if defined(OMNI_NEED_STATIC_FUNC_TO_FORCE_LINK)

// Some compilers/linkers are too clever for their own good and remove
// the unused reference to the forcelink symbol. Having a function
// that references it seems to help.

#define OMNI_FORCE_LINK(modname) \
  extern int _omni_ ## modname ## _should_be_linked_but_is_not_; \
  static int _omni_ ## modname ## _forcelink_ = \
                        _omni_ ## modname ## _should_be_linked_but_is_not_++; \
  static int _omni_ ## modname ## _value_ () { \
    return _omni_ ## modname ## _forcelink_; \
  }

#elif defined(_MSC_VER)

// The Windows non-uniform memory model means that we cannot access
// the integer's value. Referring to its address is sufficient to
// force MSVC to link correctly.

#define OMNI_FORCE_LINK(modname) \
  extern int _omni_ ## modname ## _should_be_linked_but_is_not_; \
  static int* _omni_ ## modname ## _forcelink_ = \
                         &_omni_ ## modname ## _should_be_linked_but_is_not_

#else

// On other platforms, we modify the integer, ensuring we properly
// link to it.

#define OMNI_FORCE_LINK(modname) \
  extern int _omni_ ## modname ## _should_be_linked_but_is_not_; \
  static int _omni_ ## modname ## _forcelink_ = \
                         _omni_ ## modname ## _should_be_linked_but_is_not_++

#endif


#endif // __LINKHACKS_H__
