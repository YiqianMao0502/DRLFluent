// -*- Mode: C++; -*-
//                            Package   : omniORB2
// codeSetsImpl.h             Created on: 23/10/2000
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2000 AT&T Laboratories, Cambridge
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

#ifndef __CODESETUTIL_H__
#define __CODESETUTIL_H__

#include <omniORB4/IOP_S.h>
#include <exceptiondefs.h>
#include <giopStream.h>
#include <giopStrand.h>
#include <giopRope.h>
#include <GIOP_S.h>
#include <GIOP_C.h>

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

OMNI_NAMESPACE_BEGIN(omni)

class omniCodeSetUtil {
public:

  static _core_attr const GIOP::Version GIOP10;
  static _core_attr const GIOP::Version GIOP11;
  static _core_attr const GIOP::Version GIOP12;

  //
  // Memory management
  //

  static inline char* allocC(_CORBA_ULong len) {
    char* buf = _CORBA_String_helper::alloc(len - 1);
    if (!buf) OMNIORB_THROW(NO_MEMORY, 0, CORBA::COMPLETED_MAYBE);
    return buf;
  }

  static inline void freeC(char* buf) {
    _CORBA_String_helper::dealloc(buf);
  }

  static inline char* reallocC(char*        oldbuf,
			       _CORBA_ULong oldlen,
			       _CORBA_ULong newlen)
  {
    char* newbuf = allocC(newlen);
    for (_CORBA_ULong i=0; i < oldlen; i++) newbuf[i] = oldbuf[i];
    freeC(oldbuf);
    return newbuf;
  }

  static inline omniCodeSet::UniChar* allocU(_CORBA_ULong len) {
#if (SIZEOF_WCHAR == 2)
    omniCodeSet::UniChar* buf = _CORBA_WString_helper::alloc(len - 1);
#else
    omniCodeSet::UniChar* buf = new omniCodeSet::UniChar[len];
#endif
    if (!buf) OMNIORB_THROW(NO_MEMORY, 0, CORBA::COMPLETED_MAYBE);
    return buf;
  }

  static inline void freeU(omniCodeSet::UniChar* buf) {
#if (SIZEOF_WCHAR == 2)
    _CORBA_WString_helper::dealloc(buf);
#else
    delete [] buf;
#endif
  }

  static inline omniCodeSet::UniChar* reallocU(omniCodeSet::UniChar* oldbuf,
					       _CORBA_ULong   oldlen,
					       _CORBA_ULong   newlen)
  {
    omniCodeSet::UniChar* newbuf = allocU(newlen);
    for (_CORBA_ULong i=0; i < oldlen; i++) newbuf[i] = oldbuf[i];
    freeU(oldbuf);
    return newbuf;
  }

  static inline _CORBA_WChar* allocW(_CORBA_ULong len) {
    _CORBA_WChar* buf = _CORBA_WString_helper::alloc(len - 1);
    if (!buf) OMNIORB_THROW(NO_MEMORY, 0, CORBA::COMPLETED_MAYBE);
    return buf;
  }

  static inline void freeW(_CORBA_WChar* buf) {
    _CORBA_WString_helper::dealloc(buf);
  }

  static inline _CORBA_WChar* reallocW(_CORBA_WChar* oldbuf,
				       _CORBA_ULong  oldlen,
				       _CORBA_ULong  newlen)
  {
    _CORBA_WChar* newbuf = allocW(newlen);
    for (_CORBA_ULong i=0; i < oldlen; i++) newbuf[i] = oldbuf[i];
    freeW(oldbuf);
    return newbuf;
  }

  //
  // Memory buffers
  //

  class BufferC {
  public:
    inline BufferC(_CORBA_ULong len = 32) : pd_i(0) {
      pd_len = len;
      pd_buf = allocC(len);
    }
    inline ~BufferC() {
      if (pd_buf) freeC(pd_buf);
    }
    inline void insert(_CORBA_Char c) {
      if (pd_i >= pd_len) {
	_CORBA_ULong newlen = pd_len * 2;
	pd_buf = reallocC(pd_buf, pd_len, newlen);
	pd_len = newlen;
      }
      pd_buf[pd_i++] = c;
    }
    inline _CORBA_ULong length() { return pd_i; }
    inline char*        buffer() { return pd_buf; }
    inline char* extract() {
      char* buf = pd_buf;
      pd_buf = 0;
      return buf;
    }
  private:
    _CORBA_ULong pd_i;
    _CORBA_ULong pd_len;
    char*        pd_buf;
  };

  class BufferU {
  public:
    inline BufferU(_CORBA_ULong len = 32) : pd_i(0) {
      pd_len = len;
      pd_buf = allocU(len);
    }
    inline ~BufferU() {
      if (pd_buf) freeU(pd_buf);
    }
    inline void insert(omniCodeSet::UniChar c) {
      if (pd_i >= pd_len) {
	_CORBA_ULong newlen = pd_len * 2;
	pd_buf = reallocU(pd_buf, pd_len, newlen);
	pd_len = newlen;
      }
      pd_buf[pd_i++] = c;
    }
    inline _CORBA_ULong          length() { return pd_i; }
    inline omniCodeSet::UniChar* buffer() { return pd_buf; }
    inline omniCodeSet::UniChar* extract() {
      omniCodeSet::UniChar* buf = pd_buf;
      pd_buf = 0;
      return buf;
    }
  private:
    _CORBA_ULong          pd_i;
    _CORBA_ULong          pd_len;
    omniCodeSet::UniChar* pd_buf;
  };

  class BufferW {
  public:
    inline BufferW(_CORBA_ULong len = 32) : pd_i(0) {
      pd_len = len;
      pd_buf = allocW(len);
    }
    inline ~BufferW() {
      if (pd_buf) freeW(pd_buf);
    }
    inline void insert(_CORBA_ULong c) {
      if (pd_i >= pd_len) {
	_CORBA_ULong newlen = pd_len * 2;
	pd_buf = reallocW(pd_buf, pd_len, newlen);
	pd_len = newlen;
      }
      pd_buf[pd_i++] = (_CORBA_WChar)c;
    }
    inline _CORBA_ULong  length() { return pd_i; }
    inline _CORBA_WChar* buffer() { return pd_buf; }
    inline _CORBA_WChar* extract() {
      _CORBA_WChar* buf = pd_buf;
      pd_buf = 0;
      return buf;
    }
  private:
    _CORBA_ULong  pd_i;
    _CORBA_ULong  pd_len;
    _CORBA_WChar* pd_buf;
  };

  //
  // Memory holders
  //

  class HolderC {
  public:
    inline HolderC(char* buf) : pd_buf(buf) {}
    inline ~HolderC() { if (pd_buf) freeC(pd_buf); }
    inline void drop() { pd_buf = 0; }
  private:
    char* pd_buf;
  };

  class HolderU {
  public:
    inline HolderU(omniCodeSet::UniChar* buf) : pd_buf(buf) {}
    inline ~HolderU() { if (pd_buf) freeU(pd_buf); }
    inline void drop() { pd_buf = 0; }
  private:
    omniCodeSet::UniChar* pd_buf;
  };

  class HolderW {
  public:
    inline HolderW(_CORBA_WChar* buf) : pd_buf(buf) {}
    inline ~HolderW() { if (pd_buf) freeW(pd_buf); }
    inline void drop() { pd_buf = 0; }
  private:
    _CORBA_WChar* pd_buf;
  };
};

// These macros check there is a wchar transmission code set and, if
// not, throw the appropriate exceptions.

#define OMNIORB_CHECK_TCS_W_FOR_UNMARSHAL(tcs, stream) \
do { \
  if (!tcs) { \
    giopStream* gs = giopStream::downcast(&stream); \
    if (gs) { \
      GIOP::Version v = gs->version(); \
      if (v.major == 1 && v.minor == 0) { \
        if (GIOP_S::downcast(&stream)) \
          OMNIORB_THROW(MARSHAL, MARSHAL_WCharSentByGIOP10Client, \
                        (CORBA::CompletionStatus)stream.completion()); \
        if (GIOP_C::downcast(&stream)) \
          OMNIORB_THROW(MARSHAL, MARSHAL_WCharSentByGIOP10Server, \
                        (CORBA::CompletionStatus)stream.completion()); \
      } \
    } \
    if (GIOP_C::downcast(&stream))                                 \
      OMNIORB_THROW(INV_OBJREF, INV_OBJREF_WCharNotSupported,      \
                    (CORBA::CompletionStatus)stream.completion()); \
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_WCharTCSNotKnown,            \
		  (CORBA::CompletionStatus)stream.completion());   \
  } \
} while(0)

#define OMNIORB_CHECK_TCS_W_FOR_MARSHAL(tcs, stream) \
do { \
  if (!tcs) { \
    if (GIOP_C::downcast(&stream))                                 \
      OMNIORB_THROW(INV_OBJREF, INV_OBJREF_WCharNotSupported,      \
                    (CORBA::CompletionStatus)stream.completion()); \
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_WCharTCSNotKnown,            \
                  (CORBA::CompletionStatus)stream.completion());   \
  } \
} while(0)


OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __CODESETUTIL_H__
