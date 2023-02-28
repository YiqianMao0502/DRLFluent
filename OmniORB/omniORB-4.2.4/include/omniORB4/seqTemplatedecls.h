// -*- Mode: C++; -*-
//                            Package   : omniORB
// seqtemplates.h             Created on: 14/5/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2009 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library.
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


#ifndef __SEQTEMPLATEDECLS_H__
#define __SEQTEMPLATEDECLS_H__

class cdrStream;

//////////////////////////////////////////////////////////////////////
/////////////////////////// _CORBA_Sequence //////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
class _CORBA_Sequence {
public:
  typedef _CORBA_Sequence<T> T_seq;

  inline _CORBA_ULong maximum() const { return pd_max; }
  inline _CORBA_ULong length() const { return pd_len; }
  inline void length(_CORBA_ULong len)
  {
    if (len) {
      if (len > pd_max) {
	if (pd_bounded) {
	  _CORBA_bound_check_error();
	  // Never reach here.
	}
	// Grow exponentially
	_CORBA_ULong newmax = pd_max * 2;
	copybuffer((len > newmax) ? len : newmax);
      }
      else if (!pd_buf) {
	// Allocate initial buffer
	copybuffer(pd_max);
      }
    }
    else {
      // Zero length. Deallocate buffer if it is owned by us.
      if (pd_rel && pd_buf) {
	freebuf(pd_buf);
	pd_buf = 0;
	if (!pd_bounded)
	  pd_max = 0;
      }
    }
    pd_len = len;
  }
  inline T& operator[] (_CORBA_ULong index_) {
    if (index_ >= pd_len) _CORBA_bound_check_error();
    return pd_buf[index_];
  }
  inline const T& operator[] (_CORBA_ULong index_) const {
    if (index_ >= pd_len) _CORBA_bound_check_error();
    return pd_buf[index_];
  }
  static inline T* allocbuf(_CORBA_ULong nelems) {
    T* tmp = 0;
    if (nelems) {
      tmp = new T[nelems];
    }
    return tmp;
  }
  static inline void freebuf(T* b) {
    if (b) delete [] b; 
  }

  // CORBA 2.3 additions
  inline _CORBA_Boolean release() const { return pd_rel; }
  
  inline T* get_buffer(_CORBA_Boolean orphan = 0) {
    if (pd_max && !pd_buf) {
      copybuffer(pd_max);
    }
    if (!orphan) {
      return pd_buf;
    }
    else {
      if (!pd_rel) {
	return 0;
      }
      else {
	T* tmp = pd_buf;
	pd_buf = 0;
	if (!pd_bounded) {
	  pd_max = 0;
	}
	pd_len = 0;
	return tmp;
      }
    }
  }

  inline const T* get_buffer() const { 
    if (pd_max && !pd_buf) {
      T_seq* s = OMNI_CONST_CAST(T_seq*, this);
      s->copybuffer(pd_max);
    }
    return pd_buf;
  }

  inline ~_CORBA_Sequence() {
    if (pd_rel && pd_buf) freebuf(pd_buf);
    pd_buf = 0;
  }

  // omniORB extensions
  inline T* NP_data() const   { return pd_buf; }
  inline void NP_norelease()  { pd_rel = 0;    }

protected:
  inline _CORBA_Sequence() : pd_max(0), pd_len(0), pd_rel(1), 
			     pd_bounded(0), pd_buf(0) { }

  inline _CORBA_Sequence(_CORBA_ULong max, _CORBA_Boolean bounded=0) :
    pd_max(max), pd_len(0), pd_rel(1), pd_bounded(bounded), pd_buf(0) {}

  inline _CORBA_Sequence(const T_seq& s) : pd_max(s.pd_max), 
					   pd_len(0),
					   pd_rel(1),
					   pd_bounded(s.pd_bounded), 
					   pd_buf(0) {
    length(s.pd_len);
    for (_CORBA_ULong i=0; i < pd_len; i++) {
      pd_buf[i] = s.pd_buf[i];
    }
  }

  inline _CORBA_Sequence(_CORBA_ULong 	max,
			 _CORBA_ULong 	len,
			 T*             value,
			 _CORBA_Boolean release_ = 0,
			 _CORBA_Boolean bounded  = 0) 
      : pd_max(max), 
	pd_len(len), 
	pd_rel(release_),
	pd_bounded(bounded),
	pd_buf(value)
  {
    if (len > max || (len && !value)) {
      _CORBA_bound_check_error();
      // never reach here
    }
  }

  inline T_seq &operator= (const T_seq &s)
  {
    if (&s != this) {
      // Prevent copybuffer from copying any of the current sequence members
      pd_len = 0;

      length(s.pd_len);
      for (unsigned long i=0; i < pd_len; i++) {
        pd_buf[i] = s.pd_buf[i];
      }
    }
    return *this;
  }

  // CORBA 2.3 additions
  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T* data,
		      _CORBA_Boolean release_ = 0) {
    if (len > max || (len && !data)) {
      _CORBA_bound_check_error();
      // never reach here
    }
    if (pd_rel && pd_buf) {
      freebuf(pd_buf);
    }
    pd_max = max;
    pd_len = len;
    pd_buf = data;
    pd_rel = release_;
  }

protected:
  void copybuffer(_CORBA_ULong newmax) {
    // replace pd_data with a new buffer of size newmax.
    // Invariant:  pd_len <= newmax
    //
    T* newbuf = allocbuf(newmax);
    if (!newbuf) {
      _CORBA_new_operator_return_null();
      // never reach here
    }
    for (unsigned long i=0; i < pd_len; i++) {
      newbuf[i] = pd_buf[i];
    }
    if (pd_rel && pd_buf) {
      freebuf(pd_buf);
    }
    else {
      pd_rel = 1;
    }
    pd_buf = newbuf;
    pd_max = newmax;
  }

  _CORBA_ULong   pd_max;
  _CORBA_ULong   pd_len;
  _CORBA_Boolean pd_rel;
  _CORBA_Boolean pd_bounded;
  T*             pd_buf;
};

//////////////////////////////////////////////////////////////////////
////////////////////// _CORBA_Unbounded_Sequence /////////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
class _CORBA_Unbounded_Sequence : public _CORBA_Sequence<T> {
public:
  typedef _CORBA_Unbounded_Sequence<T> T_seq;
  typedef _CORBA_Sequence<T> Base_T_seq ;

  inline _CORBA_Unbounded_Sequence() {}
  inline _CORBA_Unbounded_Sequence(_CORBA_ULong max) : Base_T_seq(max) {}
  inline _CORBA_Unbounded_Sequence(const T_seq& s) : Base_T_seq(s) {}
  inline _CORBA_Unbounded_Sequence(_CORBA_ULong max,
				   _CORBA_ULong len,
				   T           *value,
				   _CORBA_Boolean release_ = 0) 
    : Base_T_seq(max,len,value,release_) {}
  inline T_seq &operator= (const T_seq &s) {
    Base_T_seq::operator= (s);
    return *this;
  }

  inline ~_CORBA_Unbounded_Sequence() {}

  // CORBA 2.3 additions

  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T* data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }

  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);

};

//////////////////////////////////////////////////////////////////////
/////////////////////// _CORBA_Bounded_Sequence //////////////////////
//////////////////////////////////////////////////////////////////////

template <class T,int max>
class _CORBA_Bounded_Sequence : public _CORBA_Sequence<T> {
public:
  typedef _CORBA_Bounded_Sequence<T,max> T_seq;
  typedef _CORBA_Sequence<T> Base_T_seq ;

  inline _CORBA_Bounded_Sequence() : Base_T_seq(max,1) {}
  inline _CORBA_Bounded_Sequence(_CORBA_ULong len, T* value,
				 _CORBA_Boolean rel = 0) : 
    Base_T_seq(max,len,value,rel,1) {}
  inline _CORBA_Bounded_Sequence(const T_seq& s) : Base_T_seq(s) {}
  inline ~_CORBA_Bounded_Sequence() {}
  inline T_seq& operator= (const T_seq& s) {
    Base_T_seq::operator= (s);
    return *this;
  }

  static inline T* allocbuf(_CORBA_ULong /*nelems*/) {
    return Base_T_seq::allocbuf(max);
  }

  static inline T* allocbuf() { return allocbuf(max); }

  // CORBA 2.3 additions

  inline void replace(_CORBA_ULong len, T* data,_CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }

  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);
};

//////////////////////////////////////////////////////////////////////
///////////////////// _CORBA_Sequence_Forward ////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
class _CORBA_Sequence_Forward {
public:
  typedef _CORBA_Sequence_Forward<T> T_seq;

  inline _CORBA_ULong maximum() const { return pd_max; }
  inline _CORBA_ULong length() const { return pd_len; }
  inline void length(_CORBA_ULong len)
  {
    if (len) {
      if (len > pd_max) {
	if (pd_bounded) {
	  _CORBA_bound_check_error();
	  // Never reach here.
	}
	// Grow exponentially
	_CORBA_ULong newmax = pd_max * 2;
	NP_copybuffer((len > newmax) ? len : newmax);
      }
      else if (!pd_buf) {
	// Allocate initial buffer
	NP_copybuffer(pd_max);
      }
    }
    else {
      // Zero length. Deallocate buffer if it is owned by us.
      if (pd_rel && pd_buf) {
	NP_freebuf();
	pd_buf = 0;
	if (!pd_bounded)
	  pd_max = 0;
      }
    }
    pd_len = len;
  }

  virtual T& operator[] (_CORBA_ULong index) = 0;
  virtual const T& operator[] (_CORBA_ULong index) const = 0;

  inline _CORBA_Boolean release() const { return pd_rel; }
  
  inline T* get_buffer(_CORBA_Boolean orphan = 0) {
    if (pd_max && !pd_buf) {
      NP_copybuffer(pd_max);
    }
    if (!orphan) {
      return pd_buf;
    }
    else {
      if (!pd_rel)
	return 0;
      else {
	T* tmp = pd_buf;
	pd_buf = 0;
	if (!pd_bounded) {
	  pd_max = 0;
	}
	pd_len = 0;
	pd_rel = 1;
	return tmp;
      }
    }
  }

  inline const T* get_buffer() const { 
    if (pd_max && !pd_buf) {
      T_seq* s = OMNI_CONST_CAST(T_seq*, this);
      s->NP_copybuffer(pd_max);
    }
    return pd_buf;
  }

  virtual ~_CORBA_Sequence_Forward() { }

  // omniORB extensions
  inline T* NP_data() const   { return pd_buf; }
  inline void NP_norelease()  { pd_rel = 0;    }

protected:
  inline _CORBA_Sequence_Forward()
    : pd_max(0), pd_len(0), pd_rel(1), pd_bounded(0), pd_buf(0) {}

  inline _CORBA_Sequence_Forward(_CORBA_ULong max, _CORBA_Boolean bounded=0) :
    pd_max(max), pd_len(0), pd_rel(1), pd_bounded(bounded), pd_buf(0) {}

  inline _CORBA_Sequence_Forward(_CORBA_ULong   max,
				 _CORBA_ULong   len,
				 T*             value,
				 _CORBA_Boolean release_ = 0,
				 _CORBA_Boolean bounded = 0)
      : pd_max(max),
	pd_len(len),
	pd_rel(release_),
	pd_bounded(bounded),
	pd_buf(value)
  {
    if (len > max || (len && !value)) {
      _CORBA_bound_check_error();
      // never reach here
    }
  }

  // CORBA 2.3 additions
  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T* data,
		      _CORBA_Boolean release_ = 0) {
    if (len > max || (len && !data)) {
      _CORBA_bound_check_error();
      // never reach here
    }
    if (pd_rel && pd_buf) {
      NP_freebuf();
    }
    pd_max = max;
    pd_len = len;
    pd_buf = data;
    pd_rel = release_;
  }

protected:
  virtual void NP_copybuffer(_CORBA_ULong newmax) = 0;
  virtual void NP_freebuf() = 0;

  _CORBA_ULong   pd_max;
  _CORBA_ULong   pd_len;
  _CORBA_Boolean pd_rel;
  _CORBA_Boolean pd_bounded;
  T*             pd_buf;
};


//////////////////////////////////////////////////////////////////////
//////////////// _CORBA_Unbounded_Sequence_Forward ///////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
class _CORBA_Unbounded_Sequence_Forward : public _CORBA_Sequence_Forward<T> {
public:
  typedef _CORBA_Unbounded_Sequence_Forward<T> T_seq;
  typedef _CORBA_Sequence_Forward<T> Base_T_seq ;

  inline _CORBA_Unbounded_Sequence_Forward() {}
  inline _CORBA_Unbounded_Sequence_Forward(_CORBA_ULong max)
    : Base_T_seq(max) {}
  inline _CORBA_Unbounded_Sequence_Forward(_CORBA_ULong max,
					   _CORBA_ULong len,
					   T           *value,
					   _CORBA_Boolean release_ = 0) 
    : Base_T_seq(max,len,value,release_) {}

  inline ~_CORBA_Unbounded_Sequence_Forward() {}

  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T* data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Bounded_Sequence_Forward ///////////////////
//////////////////////////////////////////////////////////////////////

template <class T,int max>
class _CORBA_Bounded_Sequence_Forward : public _CORBA_Sequence_Forward<T> {
public:
  typedef _CORBA_Bounded_Sequence_Forward<T,max> T_seq;
  typedef _CORBA_Sequence_Forward<T> Base_T_seq ;

  inline _CORBA_Bounded_Sequence_Forward() : Base_T_seq(max,1) {}
  inline _CORBA_Bounded_Sequence_Forward(_CORBA_ULong len, T* value,
					 _CORBA_Boolean rel = 0) : 
    Base_T_seq(max,len,value,rel,1) {}
  inline ~_CORBA_Bounded_Sequence_Forward() {}

  inline void replace(_CORBA_ULong len, T* data,_CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};


//////////////////////////////////////////////////////////////////////
///////////// _CORBA_Unbounded_Sequence_w_FixSizeElement /////////////
//////////////////////////////////////////////////////////////////////

template <class T,int elmSize,int elmAlignment>
class _CORBA_Unbounded_Sequence_w_FixSizeElement : 
  public _CORBA_Unbounded_Sequence<T> {
public:
  typedef _CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment>  T_seq;
  typedef _CORBA_Unbounded_Sequence<T> Base_T_seq;

  inline _CORBA_Unbounded_Sequence_w_FixSizeElement() {}
  inline _CORBA_Unbounded_Sequence_w_FixSizeElement(_CORBA_ULong max) : 
    Base_T_seq(max) {}

  inline _CORBA_Unbounded_Sequence_w_FixSizeElement(const T_seq& s) : 
    Base_T_seq(s) {}

  inline _CORBA_Unbounded_Sequence_w_FixSizeElement(_CORBA_ULong max,
						    _CORBA_ULong len,
						    T           *value,
						    _CORBA_Boolean rel = 0) : 
    Base_T_seq(max,len,value,rel) {}

  inline T_seq& operator= (const T_seq& s)
  {
    Base_T_seq::operator= (s);
    return *this;
  }
  inline ~_CORBA_Unbounded_Sequence_w_FixSizeElement() {}

  // CORBA 2.3 additions

  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T* data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }

  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);
};


//////////////////////////////////////////////////////////////////////
////////////// _CORBA_Bounded_Sequence_w_FixSizeElement //////////////
//////////////////////////////////////////////////////////////////////

template <class T,int max,int elmSize, int elmAlignment>
class _CORBA_Bounded_Sequence_w_FixSizeElement : 
  public _CORBA_Bounded_Sequence<T,max> {
public:
  typedef _CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment> T_seq;
  typedef _CORBA_Bounded_Sequence<T,max> Base_T_seq;


  inline _CORBA_Bounded_Sequence_w_FixSizeElement() {}
  inline _CORBA_Bounded_Sequence_w_FixSizeElement(_CORBA_ULong len,
						  T           *value,
						  _CORBA_Boolean rel = 0) : 
    Base_T_seq(len,value,rel) {}

  inline _CORBA_Bounded_Sequence_w_FixSizeElement(const T_seq& s) : 
    Base_T_seq(s) {}

  inline ~_CORBA_Bounded_Sequence_w_FixSizeElement() {}

  inline T_seq& operator=(const T_seq& s) {
    Base_T_seq::operator= (s);
    return *this;
  }

  // CORBA 2.3 additions

  inline void replace(_CORBA_ULong len, T* data,_CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(len,data,release_);
  }

  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);
};


#ifdef OMNI_MIXED_ENDIAN_DOUBLE

template <int max>
class _CORBA_Bounded_Sequence_w_FixSizeElement<_CORBA_Double,max,8,8> : 
  public _CORBA_Bounded_Sequence<_CORBA_Double,max> {
public:
  typedef _CORBA_Bounded_Sequence_w_FixSizeElement<_CORBA_Double,max,8,8> T_seq;
  typedef _CORBA_Bounded_Sequence<_CORBA_Double,max> Base_T_seq;


  inline _CORBA_Bounded_Sequence_w_FixSizeElement() {}
  inline _CORBA_Bounded_Sequence_w_FixSizeElement(_CORBA_ULong   len,
						  _CORBA_Double  *value,
						  _CORBA_Boolean rel = 0) : 
    Base_T_seq(len,value,rel) {}

  inline _CORBA_Bounded_Sequence_w_FixSizeElement(const T_seq& s) : 
    Base_T_seq(s) {}

  inline ~_CORBA_Bounded_Sequence_w_FixSizeElement() {}

  inline T_seq& operator=(const T_seq& s) {
    Base_T_seq::operator= (s);
    return *this;
  }

  // CORBA 2.3 additions

  inline void replace(_CORBA_ULong len, _CORBA_Double* data,_CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(len,data,release_);
  }

  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);
};

#endif



//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Sequence_Char  /////////////////////////////
//////////////////////////////////////////////////////////////////////
class _CORBA_Sequence_Char : public _CORBA_Sequence<_CORBA_Char> {
protected:
  typedef _CORBA_Sequence_Char T_seq;
  typedef _CORBA_Sequence<_CORBA_Char> Base_T_seq;
  typedef _CORBA_Char T;

  inline _CORBA_Sequence_Char() {}

  inline _CORBA_Sequence_Char(_CORBA_ULong max, _CORBA_Boolean bounded=0) :
    Base_T_seq(max,bounded) {}

  inline _CORBA_Sequence_Char(const T_seq& s) : Base_T_seq(s) {}

  inline _CORBA_Sequence_Char(_CORBA_ULong max,
			      _CORBA_ULong len,
			      T           *value,
			      _CORBA_Boolean release_ = 0,
			      _CORBA_Boolean bounded = 0
			      )  : Base_T_seq(max,len,value,
					      release_,bounded) {}

  inline T_seq &operator= (const T_seq &s) {
    Base_T_seq::operator= (s);
    return *this;
  }

  inline ~_CORBA_Sequence_Char() {}

public:
  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);
};

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Sequence_Boolean  //////////////////////////
//////////////////////////////////////////////////////////////////////
class _CORBA_Sequence_Boolean : public _CORBA_Sequence<_CORBA_Boolean> {
protected:
  typedef _CORBA_Sequence_Boolean T_seq;
  typedef _CORBA_Sequence<_CORBA_Boolean> Base_T_seq;
  typedef _CORBA_Boolean T;

  inline _CORBA_Sequence_Boolean() {}

  inline _CORBA_Sequence_Boolean(_CORBA_ULong max, _CORBA_Boolean bounded=0) :
    Base_T_seq(max,bounded) {}

  inline _CORBA_Sequence_Boolean(const T_seq& s) : Base_T_seq(s) {}

  inline _CORBA_Sequence_Boolean(_CORBA_ULong max,
	       _CORBA_ULong len,
	       T           *value,
	       _CORBA_Boolean release_ = 0,
	       _CORBA_Boolean bounded = 0
	       )  : Base_T_seq(max,len,value,release_,bounded) {}

  inline T_seq &operator= (const T_seq &s) {
    Base_T_seq::operator= (s);
    return *this;
  }

  inline ~_CORBA_Sequence_Boolean() {}

public:
  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);
};


//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Sequence_Octet  ////////////////////////////
//////////////////////////////////////////////////////////////////////
class _CORBA_Sequence_Octet : public _CORBA_Sequence<_CORBA_Octet> {
protected:
  typedef _CORBA_Sequence_Octet T_seq;
  typedef _CORBA_Sequence<_CORBA_Octet> Base_T_seq;
  typedef _CORBA_Octet T;

  inline _CORBA_Sequence_Octet() {}

  inline _CORBA_Sequence_Octet(_CORBA_ULong max, _CORBA_Boolean bounded=0) :
    Base_T_seq(max,bounded) {}

  inline _CORBA_Sequence_Octet(const T_seq& s) : Base_T_seq(s) {}

  inline _CORBA_Sequence_Octet(_CORBA_ULong max,
			       _CORBA_ULong len,
			       T           *value,
			       _CORBA_Boolean release_ = 0,
			       _CORBA_Boolean bounded = 0
			       )  : 
    Base_T_seq(max,len,value,release_,bounded) {}

  inline T_seq &operator= (const T_seq &s) {
    Base_T_seq::operator= (s);
    return *this;
  }

  inline ~_CORBA_Sequence_Octet() {}

public:
  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);
};

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Sequence_WChar  ////////////////////////////
//////////////////////////////////////////////////////////////////////
class _CORBA_Sequence_WChar : public _CORBA_Sequence<_CORBA_WChar> {
protected:
  typedef _CORBA_Sequence_WChar T_seq;
  typedef _CORBA_Sequence<_CORBA_WChar> Base_T_seq;
  typedef _CORBA_WChar T;

  inline _CORBA_Sequence_WChar() {}

  inline _CORBA_Sequence_WChar(_CORBA_ULong max, _CORBA_Boolean bounded=0) :
    Base_T_seq(max,bounded) {}

  inline _CORBA_Sequence_WChar(const T_seq& s) : Base_T_seq(s) {}

  inline _CORBA_Sequence_WChar(_CORBA_ULong max,
			       _CORBA_ULong len,
			       T           *value,
			       _CORBA_Boolean release_ = 0,
			       _CORBA_Boolean bounded = 0
			       )  : 
    Base_T_seq(max,len,value,release_,bounded) {}

  inline T_seq &operator= (const T_seq &s) {
    Base_T_seq::operator= (s);
    return *this;
  }

  inline ~_CORBA_Sequence_WChar() {}

public:
  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);
};

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Unbounded_Sequence_Char  //////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_Unbounded_Sequence_Char : public _CORBA_Sequence_Char {
public:
  typedef _CORBA_Unbounded_Sequence_Char T_seq;
  typedef _CORBA_Sequence_Char Base_T_seq;
  typedef _CORBA_Char T;

  inline _CORBA_Unbounded_Sequence_Char() {}

  inline _CORBA_Unbounded_Sequence_Char(_CORBA_ULong max) : Base_T_seq(max) {}

  inline _CORBA_Unbounded_Sequence_Char(_CORBA_ULong max,
					 _CORBA_ULong len,
					 T   *value,
					 _CORBA_Boolean release_ = 0) : 
    Base_T_seq(max,len,value,release_) {}

  inline _CORBA_Unbounded_Sequence_Char (const T_seq& s) : Base_T_seq(s) {}

  inline T_seq& operator= (const T_seq& s) {
    Base_T_seq::operator=(s);
    return *this;
  }

  inline ~_CORBA_Unbounded_Sequence_Char() {}

  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T* data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
///////////////// _CORBA_Unbounded_Sequence_Boolean /////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_Unbounded_Sequence_Boolean : public _CORBA_Sequence_Boolean {
public:
  typedef _CORBA_Unbounded_Sequence_Boolean T_seq;
  typedef _CORBA_Sequence_Boolean            Base_T_seq;
  typedef _CORBA_Boolean T;

  inline _CORBA_Unbounded_Sequence_Boolean() {}

  inline _CORBA_Unbounded_Sequence_Boolean(_CORBA_ULong max) : 
    Base_T_seq(max) {}

  inline _CORBA_Unbounded_Sequence_Boolean(_CORBA_ULong max,
					    _CORBA_ULong len,
					    T   *value,
					    _CORBA_Boolean release_ = 0) : 
    Base_T_seq(max,len,value,release_) {}

  inline _CORBA_Unbounded_Sequence_Boolean(const T_seq& s) : Base_T_seq(s) {}

  inline T_seq& operator= (const T_seq& s) {
    Base_T_seq::operator=(s);
    return *this;
  }

  inline ~_CORBA_Unbounded_Sequence_Boolean() {}

  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T* data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Unbounded_Sequence_Octet //////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_Unbounded_Sequence_Octet : public _CORBA_Sequence_Octet {
public:
  typedef _CORBA_Unbounded_Sequence_Octet T_seq;
  typedef _CORBA_Sequence_Octet          Base_T_seq;
  typedef _CORBA_Octet T;

  inline _CORBA_Unbounded_Sequence_Octet() {}

  inline _CORBA_Unbounded_Sequence_Octet(_CORBA_ULong max) : 
    Base_T_seq(max) {}

  inline _CORBA_Unbounded_Sequence_Octet(_CORBA_ULong max,
					  _CORBA_ULong len,
					  T   *value,
					  _CORBA_Boolean release_ = 0) : 
    Base_T_seq(max,len,value,release_) {}

  inline _CORBA_Unbounded_Sequence_Octet (const T_seq& s) : Base_T_seq(s) {}

  inline T_seq& operator= (const T_seq& s) {
    Base_T_seq::operator=(s);
    return *this;
  }

  inline ~_CORBA_Unbounded_Sequence_Octet() {}

  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T* data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Unbounded_Sequence_WChar  //////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_Unbounded_Sequence_WChar : public _CORBA_Sequence_WChar {
public:
  typedef _CORBA_Unbounded_Sequence_WChar T_seq;
  typedef _CORBA_Sequence_WChar          Base_T_seq;
  typedef _CORBA_WChar T;

  inline _CORBA_Unbounded_Sequence_WChar() {}

  inline _CORBA_Unbounded_Sequence_WChar(_CORBA_ULong max) : 
    Base_T_seq(max) {}

  inline _CORBA_Unbounded_Sequence_WChar(_CORBA_ULong max,
					  _CORBA_ULong len,
					  T   *value,
					  _CORBA_Boolean release_ = 0) : 
    Base_T_seq(max,len,value,release_) {}

  inline _CORBA_Unbounded_Sequence_WChar (const T_seq& s) : Base_T_seq(s) {}

  inline T_seq& operator= (const T_seq& s) {
    Base_T_seq::operator=(s);
    return *this;
  }

  inline ~_CORBA_Unbounded_Sequence_WChar() {}

  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T* data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
/////////////////// _CORBA_Bounded_Sequence_Char  ///////////////////
//////////////////////////////////////////////////////////////////////

template <int max>
class _CORBA_Bounded_Sequence_Char : public _CORBA_Sequence_Char {
public:
  typedef _CORBA_Bounded_Sequence_Char<max> T_seq;
  typedef _CORBA_Sequence_Char               Base_T_seq;
  typedef _CORBA_Char T;

  inline _CORBA_Bounded_Sequence_Char() : Base_T_seq(max,1) {}

  inline _CORBA_Bounded_Sequence_Char(_CORBA_ULong len,
				       T    *value,
				       _CORBA_Boolean release_ = 0)
    : Base_T_seq(max,len,value,release_,1) {}

  inline _CORBA_Bounded_Sequence_Char(const T_seq& s) : Base_T_seq(s) {}

  inline T_seq& operator= (const T_seq& s) {
     Base_T_seq::operator= (s);
     return *this;
  }

  inline ~_CORBA_Bounded_Sequence_Char() {}

  inline void replace(_CORBA_ULong len, T* data,_CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Bounded_Sequence_Boolean //////////////////
//////////////////////////////////////////////////////////////////////

template <int max>
class _CORBA_Bounded_Sequence_Boolean : public _CORBA_Sequence_Boolean {
public:
  typedef _CORBA_Bounded_Sequence_Boolean<max> T_seq;
  typedef _CORBA_Sequence_Boolean               Base_T_seq;
  typedef _CORBA_Boolean T;

  inline _CORBA_Bounded_Sequence_Boolean() : Base_T_seq(max,1) {}

  inline _CORBA_Bounded_Sequence_Boolean(_CORBA_ULong len,
					  T    *value,
					  _CORBA_Boolean release_ = 0)
    : Base_T_seq(max,len,value,release_,1) {}

  inline _CORBA_Bounded_Sequence_Boolean(const T_seq& s) : Base_T_seq(s) {}

  inline T_seq& operator= (const T_seq& s) {
     Base_T_seq::operator= (s);
     return *this;
  }

  inline ~_CORBA_Bounded_Sequence_Boolean() {}

  inline void replace(_CORBA_ULong len, T* data,_CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
/////////////////// _CORBA_Bounded_Sequence_Octet ///////////////////
//////////////////////////////////////////////////////////////////////

template <int max>
class _CORBA_Bounded_Sequence_Octet : public _CORBA_Sequence_Octet {
public:
  typedef _CORBA_Bounded_Sequence_Octet<max> T_seq;
  typedef _CORBA_Sequence_Octet               Base_T_seq;
  typedef _CORBA_Octet T;

  inline _CORBA_Bounded_Sequence_Octet() : Base_T_seq(max,1) {}

  inline _CORBA_Bounded_Sequence_Octet(_CORBA_ULong len,
					T    *value,
					_CORBA_Boolean release_ = 0)
    : Base_T_seq(max,len,value,release_,1) {}

  inline _CORBA_Bounded_Sequence_Octet(const T_seq& s) : Base_T_seq(s) {}

  inline T_seq& operator= (const T_seq& s) {
     Base_T_seq::operator= (s);
     return *this;
  }

  inline ~_CORBA_Bounded_Sequence_Octet() {}

  inline void replace(_CORBA_ULong len, T* data,_CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
/////////////////// _CORBA_Bounded_Sequence_WChar ///////////////////
//////////////////////////////////////////////////////////////////////

template <int max>
class _CORBA_Bounded_Sequence_WChar : public _CORBA_Sequence_WChar {
public:
  typedef _CORBA_Bounded_Sequence_WChar<max> T_seq;
  typedef _CORBA_Sequence_WChar               Base_T_seq;
  typedef _CORBA_WChar T;

  inline _CORBA_Bounded_Sequence_WChar() : Base_T_seq(max,1) {}

  inline _CORBA_Bounded_Sequence_WChar(_CORBA_ULong len,
					T    *value,
					_CORBA_Boolean release_ = 0)
    : Base_T_seq(max,len,value,release_,1) {}

  inline _CORBA_Bounded_Sequence_WChar(const T_seq& s) : Base_T_seq(s) {}

  inline T_seq& operator= (const T_seq& s) {
     Base_T_seq::operator= (s);
     return *this;
  }

  inline ~_CORBA_Bounded_Sequence_WChar() {}

  inline void replace(_CORBA_ULong len, T* data,_CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
//////////////////////// _CORBA_Sequence_Array ///////////////////////
//////////////////////////////////////////////////////////////////////

template <class T,class T_slice,class T_elm,int dimension>
class _CORBA_Sequence_Array {
public:
  typedef _CORBA_Sequence_Array<T,T_slice,T_elm,dimension> T_seq;

  inline _CORBA_ULong maximum() const { return pd_max; }
  inline _CORBA_ULong length() const { return pd_len; }
  inline void length(_CORBA_ULong len)
  {
    if (len) {
      if (len > pd_max) {
	if (pd_bounded) {
	  _CORBA_bound_check_error();
	  // Never reach here.
	}
	// Grow exponentially
	_CORBA_ULong newmax = pd_max * 2;
	copybuffer((len > newmax) ? len : newmax);
      }
      else if (!pd_buf) {
	// Allocate initial buffer
	copybuffer(pd_max);
      }
    }
    else {
      // Zero length. Deallocate buffer if it is owned by us.
      if (pd_rel && pd_buf) {
	freebuf(pd_buf);
	pd_buf = 0;
	if (!pd_bounded)
	  pd_max = 0;
      }
    }
    pd_len = len;
  }
  // These have to return pointer to slice to support brain-dead compilers
  // (such as MSVC, which handles references to arrays poorly).
  inline T_slice* operator[] (_CORBA_ULong index_)
  {
    if (index_ >= pd_len) _CORBA_bound_check_error();
    return (T_slice*)(pd_buf[index_]);
  }
  inline const T_slice* operator[] (_CORBA_ULong index_) const
  {
    if (index_ >= pd_len) _CORBA_bound_check_error();
    return (const T_slice*)(pd_buf[index_]);
  }

  static inline T* allocbuf(_CORBA_ULong nelems)
  {
    if (nelems) return new T[nelems];
    else return 0;
  }
  static inline void freebuf(T * b)
  {
    if (b) delete [] b; 
    return;
  }

  // CORBA 2.3 additions
  inline _CORBA_Boolean release() const { return pd_rel; }
  
  inline T* get_buffer(_CORBA_Boolean orphan = 0) {
    if (pd_max && !pd_buf) {
      copybuffer(pd_max);
    }
    if (!orphan) {
      return pd_buf;
    }
    else {
      if (!pd_rel)
	return 0;
      else {
	T* tmp = pd_buf;
	pd_buf = 0;
	if (!pd_bounded) {
	  pd_max = 0;
	}
	pd_len = 0;
	pd_rel = 1;
	return tmp;
      }
    }
  }

  inline T* get_buffer() const {
    if (pd_max && !pd_buf) {
      T_seq* s = OMNI_CONST_CAST(T_seq*, this);
      s->copybuffer(pd_max);
    }
    return pd_buf;
  }

  inline ~_CORBA_Sequence_Array() {
    if (pd_rel && pd_buf) freebuf(pd_buf);
    pd_buf = 0;
    return;
  }

  // omniORB extensions
  inline T *NP_data() const { return pd_buf; }
  inline void NP_norelease() { pd_rel = 0; }

protected:
  inline _CORBA_Sequence_Array() : pd_max(0), pd_len(0), pd_rel(1),
                                   pd_bounded(0), pd_buf(0) { }

  inline _CORBA_Sequence_Array(_CORBA_ULong max, _CORBA_Boolean bounded=0) :
    pd_max(max), pd_len(0), pd_rel(1), pd_bounded(bounded), pd_buf(0) {}

  inline _CORBA_Sequence_Array(const T_seq& s)
    : pd_max(s.pd_max), 
      pd_len(0),
      pd_rel(1),
      pd_bounded(s.pd_bounded), pd_buf(0)
  {
    length(s.pd_len);
    for (_CORBA_ULong i=0; i < pd_len; i++) {
      for (_CORBA_ULong j=0; j < dimension; j++) {
	*((T_elm*)(pd_buf[i]) + j) = *((T_elm*)(s.pd_buf[i]) + j);
      }
    }
  }

  inline _CORBA_Sequence_Array(_CORBA_ULong max,
			       _CORBA_ULong len,
			       T           *value,
			       _CORBA_Boolean release_ = 0,
			       _CORBA_Boolean bounded = 0
			       ) 
      : pd_max(max), 
	pd_len(len), 
	pd_rel(release_),
	pd_bounded(bounded),
	pd_buf(value)
  {
    if (len > max || (len && !value)) {
      _CORBA_bound_check_error();
      // never reach here
    }
  }


  inline T_seq& operator= (const T_seq& s)
  {
    if (&s != this) {
      // Prevent copybuffer from copying any of the current sequence members 
      pd_len = 0;

      length(s.pd_len);
      for (unsigned long i=0; i < pd_len; i++) {
        for (_CORBA_ULong j=0; j < dimension; j++) {
          *((T_elm*)(pd_buf[i]) + j) = *((T_elm*)(s.pd_buf[i]) + j);
        }
      }
    }
    return *this;
  }

  // CORBA 2.3 additions
  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T* data,
		      _CORBA_Boolean release_ = 0) {
    if (len > max || (len && !data)) {
      _CORBA_bound_check_error();
      // never reach here
    }
    if (pd_rel && pd_buf) {
      freebuf(pd_buf);
    }
    pd_max = max;
    pd_len = len;
    pd_buf = data;
    pd_rel = release_;
  }


protected:

  void copybuffer(_CORBA_ULong newmax) {
    // replace pd_data with a new buffer of size newmax.
    // Invariant:  pd_len <= newmax
    //
    T* newbuf = allocbuf(newmax);
    if (!newbuf) {
      _CORBA_new_operator_return_null();
      // never reach here
    }
    for (unsigned long i=0; i < pd_len; i++) {
      for (_CORBA_ULong j=0; j < dimension; j++) {
	*((T_elm*)(newbuf[i]) + j) = *((T_elm*)(pd_buf[i]) + j);
      }
    }
    if (pd_rel && pd_buf) {
      freebuf(pd_buf);
    }
    else {
      pd_rel = 1;
    }
    pd_buf = newbuf;
    pd_max = newmax;
  }

  _CORBA_ULong    pd_max;
  _CORBA_ULong    pd_len;
  _CORBA_Boolean  pd_rel;
  _CORBA_Boolean  pd_bounded;
  T*              pd_buf;
};

//////////////////////////////////////////////////////////////////////
/////////////////// _CORBA_Unbounded_Sequence_Array //////////////////
//////////////////////////////////////////////////////////////////////

template <class T,class T_slice,class T_elm,int dimension>
class _CORBA_Unbounded_Sequence_Array : public _CORBA_Sequence_Array<T,T_slice,T_elm,dimension> {
public:
  typedef _CORBA_Unbounded_Sequence_Array T_seq;
  typedef  _CORBA_Sequence_Array<T,T_slice,T_elm,dimension> Base_T_seq ;

  inline _CORBA_Unbounded_Sequence_Array() {}
  inline _CORBA_Unbounded_Sequence_Array(_CORBA_ULong max) : Base_T_seq(max) {}
  inline _CORBA_Unbounded_Sequence_Array(const T_seq& s) : Base_T_seq(s) {}
  inline _CORBA_Unbounded_Sequence_Array(_CORBA_ULong max,
					 _CORBA_ULong len,
					 T           *value,
					 _CORBA_Boolean release_ = 0)
     : Base_T_seq(max,len,value,release_) {}
  inline T_seq &operator= (const T_seq& s) {
    Base_T_seq::operator= (s);
    return *this;
  }
  inline ~_CORBA_Unbounded_Sequence_Array() {}

  // CORBA 2.3 additions

  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T* data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }

  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);
};

//////////////////////////////////////////////////////////////////////
//////////////////// _CORBA_Bounded_Sequence_Array ///////////////////
//////////////////////////////////////////////////////////////////////

template <class T,class T_slice,class T_elm,int dimension,int max>
class _CORBA_Bounded_Sequence_Array : public _CORBA_Sequence_Array<T,T_slice,T_elm,dimension> {
public:
  typedef _CORBA_Bounded_Sequence_Array<T,T_slice,T_elm,dimension,max> T_seq;
  typedef _CORBA_Sequence_Array<T,T_slice,T_elm,dimension> Base_T_seq ;

  inline _CORBA_Bounded_Sequence_Array() : Base_T_seq(max,1) {}
  inline _CORBA_Bounded_Sequence_Array(_CORBA_ULong len,
				       T           *value,
				       _CORBA_Boolean release_ = 0)
    : Base_T_seq(max,len,value,release_,1) {}
  inline _CORBA_Bounded_Sequence_Array(const T_seq& s) : Base_T_seq(s) {}
  inline T_seq&operator= (const T_seq&s) {
    Base_T_seq::operator= (s);
    return *this;
  }

  inline ~_CORBA_Bounded_Sequence_Array() {}

  static inline T* allocbuf(_CORBA_ULong /*nelems*/) {
    return Base_T_seq::allocbuf(max);
  }

  static inline T* allocbuf() { return allocbuf(max); }

  // CORBA 2.3 additions

  inline void replace(_CORBA_ULong len, T* data,_CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }

  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);
};

//////////////////////////////////////////////////////////////////////
////////// _CORBA_Unbounded_Sequence_Array_w_FixSizeElement //////////
//////////////////////////////////////////////////////////////////////

template <class T,class T_slice, class T_elm,int dimension,int elmSize,int elmAlignment>
class _CORBA_Unbounded_Sequence_Array_w_FixSizeElement 
   : public _CORBA_Unbounded_Sequence_Array<T,T_slice,T_elm,dimension> 
{
public:
  typedef _CORBA_Unbounded_Sequence_Array_w_FixSizeElement<T,T_slice,T_elm,dimension,elmSize,elmAlignment> T_seq;
  typedef _CORBA_Unbounded_Sequence_Array<T,T_slice,T_elm,dimension> Base_T_seq;

  inline _CORBA_Unbounded_Sequence_Array_w_FixSizeElement() {}
  inline _CORBA_Unbounded_Sequence_Array_w_FixSizeElement(_CORBA_ULong max) : 
    Base_T_seq(max) {}

  inline _CORBA_Unbounded_Sequence_Array_w_FixSizeElement (const T_seq& s) : 
    Base_T_seq(s) {}

  inline _CORBA_Unbounded_Sequence_Array_w_FixSizeElement(_CORBA_ULong max,
							  _CORBA_ULong len,
							  T           *value,
					   _CORBA_Boolean release_ = 0)
    : Base_T_seq(max,len,value,release_) {}
  inline T_seq& operator= (const T_seq& s)
  {
    Base_T_seq::operator= (s);
    return *this;
  }
  inline ~_CORBA_Unbounded_Sequence_Array_w_FixSizeElement() {}

  // CORBA 2.3 additions

  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T* data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }

  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);
};


//////////////////////////////////////////////////////////////////////
/////////// _CORBA_Bounded_Sequence_Array_w_FixSizeElement ///////////
//////////////////////////////////////////////////////////////////////

template <class T,class T_slice,class T_elm,int dimension,int max,int elmSize, int elmAlignment>
class _CORBA_Bounded_Sequence_Array_w_FixSizeElement 
  : public _CORBA_Bounded_Sequence_Array<T,T_slice,T_elm,dimension,max> 
{
public:
  typedef _CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,T_elm,dimension,max,elmSize,elmAlignment> T_seq;
  typedef _CORBA_Bounded_Sequence_Array<T,T_slice,T_elm,dimension,max> Base_T_seq;

  inline _CORBA_Bounded_Sequence_Array_w_FixSizeElement() {}

  inline _CORBA_Bounded_Sequence_Array_w_FixSizeElement(_CORBA_ULong len,
							T* value,
							_CORBA_Boolean rel = 0)
    : Base_T_seq(len,value,rel) {}

  inline _CORBA_Bounded_Sequence_Array_w_FixSizeElement(const T_seq& s) : 
    Base_T_seq(s) {}

  inline T_seq& operator=(const T_seq& s) {
    Base_T_seq::operator= (s);
    return *this;
  }

  inline ~_CORBA_Bounded_Sequence_Array_w_FixSizeElement() {}

  // CORBA 2.3 additions

  inline void replace(_CORBA_ULong len, T* data,_CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(len,data,release_);
  }

  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);
};


//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Sequence_Array_Char  ///////////////////////
//////////////////////////////////////////////////////////////////////
template<class T, class T_slice, int dimension>
class _CORBA_Sequence_Array_Char : 
  public _CORBA_Sequence_Array<T,T_slice,_CORBA_Char,dimension> {
protected:
  typedef _CORBA_Sequence_Array_Char<T,T_slice,dimension> T_seq;
  typedef _CORBA_Sequence_Array<T,T_slice,_CORBA_Char,dimension> Base_T_seq;

  inline _CORBA_Sequence_Array_Char() {}

  inline _CORBA_Sequence_Array_Char(_CORBA_ULong max, 
				    _CORBA_Boolean bounded=0) :
    Base_T_seq(max,bounded) {}

  inline _CORBA_Sequence_Array_Char(const T_seq& s) : Base_T_seq(s) {}

  inline _CORBA_Sequence_Array_Char(_CORBA_ULong max,
				    _CORBA_ULong len,
				    T           *value,
				    _CORBA_Boolean release_ = 0,
				    _CORBA_Boolean bounded = 0
				    )  :
    Base_T_seq(max,len,value,release_,bounded) {}

  inline T_seq &operator= (const T_seq &s) {
    Base_T_seq::operator= (s);
    return *this;
  }

  inline ~_CORBA_Sequence_Array_Char() {}

public:
  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);
};

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Sequence_Array_Boolean  ////////////////////
//////////////////////////////////////////////////////////////////////
template<class T, class T_slice, int dimension>
class _CORBA_Sequence_Array_Boolean : 
  public _CORBA_Sequence_Array<T,T_slice,_CORBA_Boolean,dimension> {
protected:
  typedef _CORBA_Sequence_Array_Boolean<T,T_slice,dimension> T_seq;
  typedef _CORBA_Sequence_Array<T,T_slice,_CORBA_Boolean,dimension> Base_T_seq;

  inline _CORBA_Sequence_Array_Boolean() {}

  inline _CORBA_Sequence_Array_Boolean(_CORBA_ULong max, 
				    _CORBA_Boolean bounded=0) :
    Base_T_seq(max,bounded) {}

  inline _CORBA_Sequence_Array_Boolean(const T_seq& s) : Base_T_seq(s) {}

  inline _CORBA_Sequence_Array_Boolean(_CORBA_ULong max,
				    _CORBA_ULong len,
				    T           *value,
				    _CORBA_Boolean release_ = 0,
				    _CORBA_Boolean bounded = 0
				    )  :
    Base_T_seq(max,len,value,release_,bounded) {}

  inline T_seq &operator= (const T_seq &s) {
    Base_T_seq::operator= (s);
    return *this;
  }

  inline ~_CORBA_Sequence_Array_Boolean() {}

public:
  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);
};


//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Sequence_Array_Octet  //////////////////////
//////////////////////////////////////////////////////////////////////
template<class T, class T_slice, int dimension>
class _CORBA_Sequence_Array_Octet : 
  public _CORBA_Sequence_Array<T,T_slice,_CORBA_Octet,dimension> {
protected:
  typedef _CORBA_Sequence_Array_Octet<T,T_slice,dimension> T_seq;
  typedef _CORBA_Sequence_Array<T,T_slice,_CORBA_Octet,dimension> Base_T_seq;

  inline _CORBA_Sequence_Array_Octet() {}

  inline _CORBA_Sequence_Array_Octet(_CORBA_ULong max, 
				    _CORBA_Boolean bounded=0) :
    Base_T_seq(max,bounded) {}

  inline _CORBA_Sequence_Array_Octet(const T_seq& s) : Base_T_seq(s) {}

  inline _CORBA_Sequence_Array_Octet(_CORBA_ULong max,
				    _CORBA_ULong len,
				    T           *value,
				    _CORBA_Boolean release_ = 0,
				    _CORBA_Boolean bounded = 0
				    )  :
    Base_T_seq(max,len,value,release_,bounded) {}

  inline T_seq &operator= (const T_seq &s) {
    Base_T_seq::operator= (s);
    return *this;
  }

  inline ~_CORBA_Sequence_Array_Octet() {}

public:
  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);
};

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Sequence_Array_WChar  //////////////////////
//////////////////////////////////////////////////////////////////////
template<class T, class T_slice, int dimension>
class _CORBA_Sequence_Array_WChar : 
  public _CORBA_Sequence_Array<T,T_slice,_CORBA_WChar,dimension> {
protected:
  typedef _CORBA_Sequence_Array_WChar<T,T_slice,dimension> T_seq;
  typedef _CORBA_Sequence_Array<T,T_slice,_CORBA_WChar,dimension> Base_T_seq;

  inline _CORBA_Sequence_Array_WChar() {}

  inline _CORBA_Sequence_Array_WChar(_CORBA_ULong max, 
				    _CORBA_Boolean bounded=0) :
    Base_T_seq(max,bounded) {}

  inline _CORBA_Sequence_Array_WChar(const T_seq& s) : Base_T_seq(s) {}

  inline _CORBA_Sequence_Array_WChar(_CORBA_ULong max,
				    _CORBA_ULong len,
				    T           *value,
				    _CORBA_Boolean release_ = 0,
				    _CORBA_Boolean bounded = 0
				    )  :
    Base_T_seq(max,len,value,release_,bounded) {}

  inline T_seq &operator= (const T_seq &s) {
    Base_T_seq::operator= (s);
    return *this;
  }

  inline ~_CORBA_Sequence_Array_WChar() {}

public:
  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);
};

//////////////////////////////////////////////////////////////////////
/////////////// _CORBA_Unbounded_Sequence_Array_Char /////////////
//////////////////////////////////////////////////////////////////////

template<class T, class T_slice, int dimension>
class _CORBA_Unbounded_Sequence_Array_Char
   : public _CORBA_Sequence_Array_Char<T,T_slice,dimension>
{
public:
  typedef _CORBA_Unbounded_Sequence_Array_Char<T,T_slice,dimension> T_seq;
  typedef _CORBA_Sequence_Array_Char<T,T_slice,dimension> Base_T_seq;

  inline _CORBA_Unbounded_Sequence_Array_Char() {}

  inline _CORBA_Unbounded_Sequence_Array_Char(_CORBA_ULong max) : 
    Base_T_seq(max) {}

  inline _CORBA_Unbounded_Sequence_Array_Char(_CORBA_ULong max,
					       _CORBA_ULong len,
					       T   *value,
					       _CORBA_Boolean release_ = 0) : 
    Base_T_seq(max,len,value,release_) {}

  inline _CORBA_Unbounded_Sequence_Array_Char (const T_seq& s) : 
    Base_T_seq(s) {}

  inline T_seq& operator= (const T_seq& s) {
    Base_T_seq::operator=(s);
    return *this;
  }

  inline ~_CORBA_Unbounded_Sequence_Array_Char() {}


  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T* data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
/////////////// _CORBA_Unbounded_Sequence_Array_Boolean /////////////
//////////////////////////////////////////////////////////////////////

template<class T, class T_slice, int dimension>
class _CORBA_Unbounded_Sequence_Array_Boolean
   : public _CORBA_Sequence_Array_Boolean<T,T_slice,dimension>
{
public:
  typedef _CORBA_Unbounded_Sequence_Array_Boolean<T,T_slice,dimension> T_seq;
  typedef _CORBA_Sequence_Array_Boolean<T,T_slice,dimension> Base_T_seq;

  inline _CORBA_Unbounded_Sequence_Array_Boolean() {}

  inline _CORBA_Unbounded_Sequence_Array_Boolean(_CORBA_ULong max) : 
    Base_T_seq(max) {}

  inline _CORBA_Unbounded_Sequence_Array_Boolean(_CORBA_ULong max,
					       _CORBA_ULong len,
					       T   *value,
					       _CORBA_Boolean release_ = 0) : 
    Base_T_seq(max,len,value,release_) {}

  inline _CORBA_Unbounded_Sequence_Array_Boolean (const T_seq& s) : 
    Base_T_seq(s) {}

  inline T_seq& operator= (const T_seq& s) {
    Base_T_seq::operator=(s);
    return *this;
  }

  inline ~_CORBA_Unbounded_Sequence_Array_Boolean() {}

  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T* data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
//////////////// _CORBA_Unbounded_Sequence_Array_Octet //////////////
//////////////////////////////////////////////////////////////////////

template<class T, class T_slice, int dimension>
class _CORBA_Unbounded_Sequence_Array_Octet
   : public _CORBA_Sequence_Array_Octet<T,T_slice,dimension>
{
public:
  typedef _CORBA_Unbounded_Sequence_Array_Octet<T,T_slice,dimension> T_seq;
  typedef _CORBA_Sequence_Array_Octet<T,T_slice,dimension> Base_T_seq;

  inline _CORBA_Unbounded_Sequence_Array_Octet() {}

  inline _CORBA_Unbounded_Sequence_Array_Octet(_CORBA_ULong max) : 
    Base_T_seq(max) {}

  inline _CORBA_Unbounded_Sequence_Array_Octet(_CORBA_ULong max,
					       _CORBA_ULong len,
					       T   *value,
					       _CORBA_Boolean release_ = 0) : 
    Base_T_seq(max,len,value,release_) {}

  inline _CORBA_Unbounded_Sequence_Array_Octet (const T_seq& s) : 
    Base_T_seq(s) {}

  inline T_seq& operator= (const T_seq& s) {
    Base_T_seq::operator=(s);
    return *this;
  }

  inline ~_CORBA_Unbounded_Sequence_Array_Octet() {}

  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T* data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
//////////////// _CORBA_Unbounded_Sequence_Array_WChar //////////////
//////////////////////////////////////////////////////////////////////

template<class T, class T_slice, int dimension>
class _CORBA_Unbounded_Sequence_Array_WChar
   : public _CORBA_Sequence_Array_WChar<T,T_slice,dimension>
{
public:
  typedef _CORBA_Unbounded_Sequence_Array_WChar<T,T_slice,dimension> T_seq;
  typedef _CORBA_Sequence_Array_WChar<T,T_slice,dimension> Base_T_seq;

  inline _CORBA_Unbounded_Sequence_Array_WChar() {}

  inline _CORBA_Unbounded_Sequence_Array_WChar(_CORBA_ULong max) : 
    Base_T_seq(max) {}

  inline _CORBA_Unbounded_Sequence_Array_WChar(_CORBA_ULong max,
					       _CORBA_ULong len,
					       T   *value,
					       _CORBA_Boolean release_ = 0) : 
    Base_T_seq(max,len,value,release_) {}

  inline _CORBA_Unbounded_Sequence_Array_WChar (const T_seq& s) : 
    Base_T_seq(s) {}

  inline T_seq& operator= (const T_seq& s) {
    Base_T_seq::operator=(s);
    return *this;
  }

  inline ~_CORBA_Unbounded_Sequence_Array_WChar() {}

  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T* data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
//////////////// _CORBA_Bounded_Sequence_Array_Char    //////////////
//////////////////////////////////////////////////////////////////////

template<class T, class T_slice, int dimension, int max>
class _CORBA_Bounded_Sequence_Array_Char : 
  public _CORBA_Sequence_Array_Char<T,T_slice,dimension> {
public:
  typedef _CORBA_Bounded_Sequence_Array_Char<T,T_slice,dimension,max> T_seq;
  typedef _CORBA_Sequence_Array_Char<T,T_slice,dimension> Base_T_seq;

  inline _CORBA_Bounded_Sequence_Array_Char() : Base_T_seq(max,1) {}

  inline _CORBA_Bounded_Sequence_Array_Char(_CORBA_ULong len,
					     T    *value,
					     _CORBA_Boolean release_ = 0)
    : Base_T_seq(max,len,value,release_,1) {}

  inline _CORBA_Bounded_Sequence_Array_Char(const T_seq& s) : Base_T_seq(s) {}

  inline T_seq& operator= (const T_seq& s) {
     Base_T_seq::operator= (s);
     return *this;
  }

  inline ~_CORBA_Bounded_Sequence_Array_Char() {}

  inline void replace(_CORBA_ULong len, T* data,_CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};


//////////////////////////////////////////////////////////////////////
//////////////// _CORBA_Bounded_Sequence_Array_Boolean //////////////
//////////////////////////////////////////////////////////////////////

template<class T, class T_slice, int dimension, int max>
class _CORBA_Bounded_Sequence_Array_Boolean : 
  public _CORBA_Sequence_Array_Boolean<T,T_slice,dimension> {
public:
  typedef _CORBA_Bounded_Sequence_Array_Boolean<T,T_slice,dimension,max> T_seq;
  typedef _CORBA_Sequence_Array_Boolean<T,T_slice,dimension> Base_T_seq;

  inline _CORBA_Bounded_Sequence_Array_Boolean() : Base_T_seq(max,1) {}

  inline _CORBA_Bounded_Sequence_Array_Boolean(_CORBA_ULong len,
					     T    *value,
					     _CORBA_Boolean release_ = 0)
    : Base_T_seq(max,len,value,release_,1) {}

  inline _CORBA_Bounded_Sequence_Array_Boolean(const T_seq& s) : Base_T_seq(s) {}

  inline T_seq& operator= (const T_seq& s) {
     Base_T_seq::operator= (s);
     return *this;
  }

  inline ~_CORBA_Bounded_Sequence_Array_Boolean() {}

  inline void replace(_CORBA_ULong len, T* data,_CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
///////////////// _CORBA_Bounded_Sequence_Array_Octet ////////////////
//////////////////////////////////////////////////////////////////////

template<class T, class T_slice, int dimension, int max>
class _CORBA_Bounded_Sequence_Array_Octet : 
  public _CORBA_Sequence_Array_Octet<T,T_slice,dimension> {
public:
  typedef _CORBA_Bounded_Sequence_Array_Octet<T,T_slice,dimension,max> T_seq;
  typedef _CORBA_Sequence_Array_Octet<T,T_slice,dimension> Base_T_seq;

  inline _CORBA_Bounded_Sequence_Array_Octet() : Base_T_seq(max,1) {}

  inline _CORBA_Bounded_Sequence_Array_Octet(_CORBA_ULong len,
					     T    *value,
					     _CORBA_Boolean release_ = 0)
    : Base_T_seq(max,len,value,release_,1) {}

  inline _CORBA_Bounded_Sequence_Array_Octet(const T_seq& s) : Base_T_seq(s) {}

  inline T_seq& operator= (const T_seq& s) {
     Base_T_seq::operator= (s);
     return *this;
  }

  inline ~_CORBA_Bounded_Sequence_Array_Octet() {}

  inline void replace(_CORBA_ULong len, T* data,_CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
///////////////// _CORBA_Bounded_Sequence_Array_WChar ////////////////
//////////////////////////////////////////////////////////////////////

template<class T, class T_slice, int dimension, int max>
class _CORBA_Bounded_Sequence_Array_WChar : 
  public _CORBA_Sequence_Array_WChar<T,T_slice,dimension> {
public:
  typedef _CORBA_Bounded_Sequence_Array_WChar<T,T_slice,dimension,max> T_seq;
  typedef _CORBA_Sequence_Array_WChar<T,T_slice,dimension> Base_T_seq;

  inline _CORBA_Bounded_Sequence_Array_WChar() : Base_T_seq(max,1) {}

  inline _CORBA_Bounded_Sequence_Array_WChar(_CORBA_ULong len,
					     T    *value,
					     _CORBA_Boolean release_ = 0)
    : Base_T_seq(max,len,value,release_,1) {}

  inline _CORBA_Bounded_Sequence_Array_WChar(const T_seq& s) : Base_T_seq(s) {}

  inline T_seq& operator= (const T_seq& s) {
     Base_T_seq::operator= (s);
     return *this;
  }

  inline ~_CORBA_Bounded_Sequence_Array_WChar() {}

  inline void replace(_CORBA_ULong len, T* data,_CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
///////////////// _CORBA_Sequence_ObjRef              ////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_Elem, class T_Helper>
class _CORBA_Sequence_ObjRef {
public:
  typedef _CORBA_Sequence_ObjRef<T,T_Elem,T_Helper> T_seq;

  inline _CORBA_ULong maximum() const { return pd_max; }
  inline _CORBA_ULong length() const { return pd_len; }
  inline void length(_CORBA_ULong len)
  {
    if (len) {
      if (len > pd_max) {
	if (pd_bounded) {
	  _CORBA_bound_check_error();
	  // Never reach here.
	}
	// Grow exponentially
	_CORBA_ULong newmax = pd_max * 2;
	copybuffer((len > newmax) ? len : newmax);
      }
      else if (!pd_buf) {
	// Allocate initial buffer
	copybuffer(pd_max);
      }
      else {
	// We have shrunk. Clear the entries at the top
	T* nil_ = T_Helper::_nil();
	for (_CORBA_ULong i = len; i < pd_len; i++) {
	  T_Helper::release(pd_buf[i]);
	  pd_buf[i] = nil_;
	}
      }
    }
    else {
      // Zero length. Deallocate buffer if it is owned by us.
      if (pd_rel && pd_buf) {
	freebuf(pd_buf);
	pd_buf = 0;
	if (!pd_bounded)
	  pd_max = 0;
      }
    }
    pd_len = len;
  }

  inline T_Elem operator[] (_CORBA_ULong i) {
    if( i >= pd_len )  _CORBA_bound_check_error();
    return T_Elem(pd_buf[i],pd_rel);
  }
  inline T_Elem operator[] (_CORBA_ULong i) const {
    if( i >= pd_len )  _CORBA_bound_check_error();
    return T_Elem(pd_buf[i],pd_rel);
  }

  static inline T** allocbuf(_CORBA_ULong nelems) {
    if (!nelems) return 0;

    T** b = new T*[nelems+1];

    T* nil_ = T_Helper::_nil();
    for (_CORBA_ULong i_ = 1; i_ < nelems+1; i_++) {
      b[i_] = nil_;
    }
    b[0] = (T*) (omni_ptr_arith_t) nelems;
    return b+1;
  }

  static inline void freebuf(T** buf) { 
    if (!buf) return;

    T** b = buf-1;

    _CORBA_ULong l = (_CORBA_ULong)(omni_ptr_arith_t)b[0];
    for (_CORBA_ULong i = 0; i < l; i++) {
      T_Helper::release(buf[i]);
    }
    b[0] = (T*) 0;
    delete [] b;
  }

  // CORBA 2.3 additions
  inline _CORBA_Boolean release() const { return pd_rel; }

  inline T** get_buffer(_CORBA_Boolean orphan = 0) {
    if (pd_max && !pd_buf) {
      copybuffer(pd_max);
    }
    if (!orphan) {
      return pd_buf;
    }
    else {
      if (!pd_rel) {
	return 0;
      }
      else {
	T** tmp = pd_buf;
	pd_buf = 0;
	if (!pd_bounded) {
	  pd_max = 0;
	}
	pd_len = 0;
	pd_rel = 1;
	return tmp;
      }
    }
  }

  inline T*const * get_buffer() const { 
    if (pd_max && !pd_buf) {
      T_seq* s = OMNI_CONST_CAST(T_seq*, this);
      s->copybuffer(pd_max);
    }
    return pd_buf; 
  }

  inline ~_CORBA_Sequence_ObjRef() {
    if (pd_rel && pd_buf) freebuf(pd_buf);
    pd_buf = 0;
  }


  // omniORB extensions
  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);

protected:
  inline _CORBA_Sequence_ObjRef()
    : pd_max(0), pd_len(0), pd_rel(1), pd_bounded(0), pd_buf(0) {}

  inline _CORBA_Sequence_ObjRef(_CORBA_ULong max,
				_CORBA_Boolean bounded=0)
    : pd_max(max), pd_len(0), pd_rel(1), pd_bounded(bounded), pd_buf(0) {}

  inline _CORBA_Sequence_ObjRef(_CORBA_ULong max,
				_CORBA_ULong len,
				T** value, 
				_CORBA_Boolean release_ = 0,
				_CORBA_Boolean bounded = 0)
    : pd_max(max), pd_len(len), pd_rel(release_),
      pd_bounded(bounded), pd_buf(value)
  {
    if (len > max || (len && !value)) {
      _CORBA_bound_check_error();
      // never reach here
    }
  }

  inline _CORBA_Sequence_ObjRef(const T_seq& s)
    : pd_max(s.pd_max), pd_len(0), pd_rel(1),
      pd_bounded(s.pd_bounded), pd_buf(0) {
    length(s.pd_len);
    for (_CORBA_ULong i = 0; i < pd_len; i++) {
      pd_buf[i] = s.pd_buf[i];
      T_Helper::duplicate(pd_buf[i]);
    }
  }

  inline T_seq& operator= (const T_seq& s) {
    if (&s != this) {
      _CORBA_ULong i;

      if (pd_rel) {
        T* nil_ = T_Helper::_nil();
        for (i=0; i < pd_len; i++) {
          T_Helper::release(pd_buf[i]);
          pd_buf[i] = nil_;
        }
        pd_len = 0;
        length(s.pd_len);
        for (i = 0; i < pd_len; i++) {
          pd_buf[i] = s.pd_buf[i];
          T_Helper::duplicate(pd_buf[i]);
        }
      }
      else {
        pd_len = 0;
        length(s.pd_len);
        for (i = 0; i < pd_len; i++) {
          pd_buf[i] = s.pd_buf[i];
        }
      }
    }
    return *this;
  }

  // CORBA 2.3 additions
  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T** data,
		      _CORBA_Boolean release_ = 0) {
    if (len > max || (len && !data)) {
      _CORBA_bound_check_error();
      // never reach here
    }
    if (pd_rel && pd_buf) {
      freebuf(pd_buf);
    }
    pd_max = max;
    pd_len = len;
    pd_buf = data;
    pd_rel = release_;
  }


protected:

  void copybuffer(_CORBA_ULong newmax) {
    // replace pd_buf with a new buffer of size newmax.
    // Invariant:  pd_len <= newmax
    //
    T** newdata = allocbuf(newmax);
    if (!newdata) {
      _CORBA_new_operator_return_null();
      // never reach here
    }
    if (pd_rel) {
      T* nil_ = T_Helper::_nil();
      for (unsigned long i_=0; i_ < pd_len; i_++) {
	newdata[i_] = pd_buf[i_];
	pd_buf[i_]  = nil_;
      }
      if (pd_buf) {
	freebuf(pd_buf);
      }
    }
    else {
      for (unsigned long i_=0; i_ < pd_len; i_++) {
	T_Helper::duplicate(pd_buf[i_]);
	newdata[i_] = pd_buf[i_];
      }
      pd_rel = 1;
    }
    pd_buf = newdata;
    pd_max = newmax;
  }

  _CORBA_ULong    pd_max;
  _CORBA_ULong    pd_len;
  _CORBA_Boolean  pd_rel;
  _CORBA_Boolean  pd_bounded;
  T**             pd_buf;
};

//////////////////////////////////////////////////////////////////////
///////////////// _CORBA_Unbounded_Sequence_ObjRef    ////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_Elem, class T_Helper>
class _CORBA_Unbounded_Sequence_ObjRef : public _CORBA_Sequence_ObjRef<T,T_Elem,T_Helper> {
public:
  typedef _CORBA_Unbounded_Sequence_ObjRef<T,T_Elem,T_Helper> T_seq;
  typedef _CORBA_Sequence_ObjRef<T,T_Elem,T_Helper> Base_T_seq;

  inline _CORBA_Unbounded_Sequence_ObjRef() {}
  inline _CORBA_Unbounded_Sequence_ObjRef(_CORBA_ULong max) : 
    Base_T_seq(max) {}

  inline _CORBA_Unbounded_Sequence_ObjRef(_CORBA_ULong max,
					  _CORBA_ULong len,
					  T          **value,
					  _CORBA_Boolean rel = 0) : 
    Base_T_seq(max,len,value,rel) {}

  inline _CORBA_Unbounded_Sequence_ObjRef(const T_seq& s) : Base_T_seq(s) {}

  inline ~_CORBA_Unbounded_Sequence_ObjRef() {}

  inline T_seq &operator= (const T_seq& s) {
    Base_T_seq::operator= (s);
    return *this;
  };

  // CORBA 2.3 additions
  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T** data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
///////////////// _CORBA_Bounded_Sequence_ObjRef      ////////////////
//////////////////////////////////////////////////////////////////////

template<class T, class T_Elem,class T_Helper,int max>
class _CORBA_Bounded_Sequence_ObjRef
  : public _CORBA_Sequence_ObjRef<T,T_Elem,T_Helper>
{
public:
  typedef _CORBA_Bounded_Sequence_ObjRef<T,T_Elem,T_Helper,max> T_seq;
  typedef _CORBA_Sequence_ObjRef<T,T_Elem,T_Helper> Base_T_seq;

  inline _CORBA_Bounded_Sequence_ObjRef() : Base_T_seq(max,1){}
  inline _CORBA_Bounded_Sequence_ObjRef(_CORBA_ULong len,
					T          **value,
					_CORBA_Boolean rel = 0) : 
    Base_T_seq(max,len,value,rel,1) {}

  inline _CORBA_Bounded_Sequence_ObjRef(const T_seq& s) : Base_T_seq(s) {}

  inline ~_CORBA_Bounded_Sequence_ObjRef() {}

  inline T_seq& operator= (const T_seq&s) {
    Base_T_seq::operator= (s);
    return *this;
  }

  static inline T** allocbuf(_CORBA_ULong /*nelems*/) {
    return Base_T_seq::allocbuf(max);
  }

  static inline T** allocbuf() { return allocbuf(max); }

  // CORBA 2.3 additions
  inline void replace(_CORBA_ULong len, T** data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};


#endif // __SEQTEMPLATEDECLS_H__

