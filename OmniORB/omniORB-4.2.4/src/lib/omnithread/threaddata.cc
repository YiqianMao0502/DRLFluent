//				Package : omnithread
// omnithread/threaddata.cc	Created : 10/2000 dpg1
//
//    Copyright (C) 2010 Apasphere Ltd
//    Copyright (C) 2000 AT&T Laboratories Cambridge
//
//    This file is part of the omnithread library
//
//    The omnithread library is free software; you can redistribute it and/or
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

// Implementation of per-thread data

#include <omnithread.h>


static omni_thread::key_t allocated_keys = 0;

omni_thread::key_t
omni_thread::allocate_key()
{
  omni_mutex_lock l(*next_id_mutex);
  return ++allocated_keys;
}

omni_thread::value_t*
omni_thread::set_value(key_t k, value_t* v)
{
  if (k == 0) return 0;
  if (k > _value_alloc) {
    next_id_mutex->lock();
    key_t alloc = allocated_keys;
    next_id_mutex->unlock();

    if (k > alloc) return 0;

    value_t** nv = new value_t*[alloc];
    key_t i = 0;
    if (_values) {
      for (; i < _value_alloc; i++)
	nv[i] = _values[i];
      delete [] _values;
    }
    for (; i < alloc; i++)
      nv[i] = 0;

    _values = nv;
    _value_alloc = alloc;
  }
  if (_values[k-1]) delete _values[k-1];
  _values[k-1] = v;
  return v;
}

omni_thread::value_t*
omni_thread::get_value(key_t k)
{
  if (k > _value_alloc) return 0;
  return _values[k-1];
}

omni_thread::value_t*
omni_thread::remove_value(key_t k)
{
  if (k > _value_alloc) return 0;
  value_t* v = _values[k-1];
  _values[k-1] = 0;
  return v;
}


#ifdef OMNI_REFCOUNT_DEFAULT

omni_mutex omni_refcount::lock;


#endif // OMNI_REFCOUNT_DEFAULT
