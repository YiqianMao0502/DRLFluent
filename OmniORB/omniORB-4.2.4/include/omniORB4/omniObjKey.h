// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniObjKey.h               Created on: 30/3/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
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
//    Representation of an object key.
//      
 
#ifndef __OMNIOBJKEY_H__
#define __OMNIOBJKEY_H__

#include <string.h>


// This is just big enough to fit a root poa key.
#define INLINE_BUF_SIZE  14


class omniObjKey {
public:
  inline omniObjKey() : pd_key(pd_inline_buf), pd_size(0) {}

  inline omniObjKey(const _CORBA_Octet* key_, int size_)
    : pd_key(pd_inline_buf), pd_size(size_) {
    if( size_ > INLINE_BUF_SIZE )  pd_key = new _CORBA_Octet[size_];
    memcpy(pd_key, key_, size_);
  }
  // Copies <key>.

  inline omniObjKey(_CORBA_Octet* key_, int size_)
    : pd_key(key_), pd_size(size_) {
    if( size_ <= INLINE_BUF_SIZE ) {
      pd_key = pd_inline_buf;
      memcpy(pd_key, key_, size_);
      delete[] key_;
    }
  }
  // Consumes <key>.

  inline omniObjKey(omniObjKey& k, int release)
    : pd_size(k.pd_size) {
    if( pd_size <= INLINE_BUF_SIZE ) {
      pd_key = pd_inline_buf;
    }
    else if( release ) {
      //assert(k.pd_key != k.pd_inline_buf);
      pd_key = k.pd_key;
      k.pd_key = k.pd_inline_buf;
      k.pd_size = 0;
      return;
    }
    else {
      pd_key = new _CORBA_Octet[pd_size];
    }
    memcpy(pd_key, k.pd_key, pd_size);
  }
  // If <release>, may (or may not) grab the key storage
  // (if any) from <k>.  ie. On return, <k> may be
  // empty.

  inline ~omniObjKey() {
    if( pd_key != pd_inline_buf )  delete[] pd_key;
  }

  inline const _CORBA_Octet* key() const { return pd_key;  }
  inline int size() const                { return pd_size; }

  inline int is_equal(const _CORBA_Octet* key_, int keysize) const {
    return keysize == pd_size && !memcmp(key_, pd_key, keysize);
  }

  inline void set_size(int size_) {
    if( size_ > pd_size && size_ > INLINE_BUF_SIZE ) {
      if( pd_key != pd_inline_buf )  delete[] pd_key;
      pd_key = new _CORBA_Octet[size_];
    }
    pd_size = size_;
  }
  inline _CORBA_Octet* write_key() { return pd_key; }

  inline void copy(const _CORBA_Octet* key_, int keysize) {
    set_size(keysize);
    memcpy(pd_key, key_, keysize);
  }
#if 0
  inline void consume(_CORBA_Octet* key, int keysize) {
    if( pd_key != pd_inline_buf )  delete[] pd_key;
    pd_key = key;
    pd_size = keysize;
  }
#endif
  inline _CORBA_Octet* return_key() {
    if( pd_key != pd_inline_buf ) {
      _CORBA_Octet* tmp = pd_key;
      pd_key = pd_inline_buf;
      pd_size = 0;
      return tmp;
    }
    _CORBA_Octet* tmp = new _CORBA_Octet[pd_size];
    memcpy(tmp, pd_key, pd_size);
    return tmp;
  }

private:
  _CORBA_Octet  pd_inline_buf[INLINE_BUF_SIZE];
  _CORBA_Octet* pd_key;
  int           pd_size;
};


#undef INLINE_BUF_SIZE


#endif  // __OMNIOBJREF_H__
