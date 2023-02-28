// -*- Mode: C++; -*-
//                            Package   : omniORB
// any.cc                     Created on: 31/07/97
//                            Author1   : Eoin Carroll (ewc)
//                            Author2   : James Weatherall (jnw)
//                            Author3   : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2004-2011 Apasphere Ltd.
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//      Implementation of type any

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <typecode.h>
#include <tcParser.h>
#include <orbParameters.h>
#include <omniORB4/linkHacks.h>
#include <omniORB4/anyStream.h>

OMNI_FORCE_LINK(dynamicLib);

OMNI_USING_NAMESPACE(omni)

// Mutex to protect Any pointers against modification by multiple threads.
static omni_tracedmutex anyLock("anyLock");


// This code is slightly unsafe by default, in that there is a small
// risk that compiler optimisations or CPU instruction re-ordering
// could lead to a race condition resulting in use of uninitialised
// memory if multiple threads try to read from a particular Any at the
// same time. To avoid that risk, but incur a small performance
// penalty, uncomment the SNAP_LOCK define below.

//#define SNAP_LOCK omni_tracedmutex_lock snap_lock(anyLock)
#define SNAP_LOCK do{}while(0)


// Extract possibly zero typecode.
static inline
CORBA::TypeCode_ptr
get(CORBA::TypeCode_ptr tc)
{
  return tc ? tc : CORBA::_tc_null;
}

// Set typecode, possibly releasing existing one. Takes ownership of new_tc.
static inline
void
grabTC(CORBA::TypeCode_ptr& tc, CORBA::TypeCode_ptr new_tc)
{
  if (tc)
    CORBA::release(tc);

  tc = new_tc;
}

// Set typecode, possibly releasing existing one. Duplicates new_tc.
static inline
void
dupTC(CORBA::TypeCode_ptr& tc, CORBA::TypeCode_ptr new_tc)
{
  if (tc)
    CORBA::release(tc);

  tc = new_tc ? CORBA::TypeCode::_duplicate(new_tc) : 0;
}


//////////////////////////////////////////////////////////////////////
////////////////// Constructors / destructor /////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Any::Any()
  : pd_tc(0), pd_mbuf(0), pd_data(0), pd_marshal(0), pd_destructor(0)
{
}


CORBA::Any::~Any()
{
  if (pd_mbuf)
    pd_mbuf->remove_ref();

  if (pd_data) {
    OMNIORB_ASSERT(pd_destructor);
    pd_destructor(pd_data);
  }

  if (pd_tc)
    CORBA::release(pd_tc);
}

void
CORBA::Any::PR_clearData()
{
  if (pd_mbuf)
    pd_mbuf->remove_ref();

  if (pd_data) {
    OMNIORB_ASSERT(pd_destructor);
    pd_destructor(pd_data);
  }
  pd_mbuf = 0;
  pd_data = 0;
  pd_marshal = 0;
  pd_destructor = 0;
}


CORBA::Any::Any(const Any& a) 
  : pd_tc(0), pd_data(0), pd_marshal(0), pd_destructor(0)
{
  dupTC(pd_tc, a.pd_tc);

  cdrAnyMemoryStream* snap_mbuf;
  void*               snap_data;
  pr_marshal_fn       snap_marshal;

  {
    SNAP_LOCK;
    snap_mbuf	 = a.pd_mbuf;
    snap_data	 = a.pd_data;
    snap_marshal = a.pd_marshal;
  }

  if (snap_mbuf) {
    pd_mbuf = snap_mbuf;
    pd_mbuf->add_ref();
  }
  else if (snap_data) {
    // Existing Any has data in its void* pointer. Rather than trying
    // to copy that (which would require a copy function to be
    // registered along with the marshal and destructor functions), we
    // marshal the data into a memory buffer.
    pd_mbuf = new cdrAnyMemoryStream;
    snap_marshal(*pd_mbuf, snap_data);
  }
  else {
    // The Any has just a TypeCode and no data yet.
    pd_mbuf = 0;
  }
}


CORBA::Any&
CORBA::Any::operator=(const CORBA::Any& a)
{
  if (&a != this) {
    PR_clearData();
    dupTC(pd_tc, a.pd_tc);

    cdrAnyMemoryStream* snap_mbuf;
    void*               snap_data;
    pr_marshal_fn       snap_marshal;

    {
      SNAP_LOCK;
      snap_mbuf	   = a.pd_mbuf;
      snap_data	   = a.pd_data;
      snap_marshal = a.pd_marshal;
    }

    if (snap_mbuf) {
      pd_mbuf = snap_mbuf;
      pd_mbuf->add_ref();
    }
    else if (snap_data) {
      pd_mbuf = new cdrAnyMemoryStream;
      snap_marshal(*pd_mbuf, snap_data);
    }
  }
  return *this;
}


//
// Nasty deprecated constructor and replace() taking a void* buffer
//

CORBA::
Any::Any(TypeCode_ptr tc, void* value, Boolean release)
  : pd_tc(0)
{
  dupTC(pd_tc, tc);

  if (value == 0) {
    // No value yet.
    pd_mbuf = 0;
    pd_data = 0;
    pd_marshal = 0;
    pd_destructor = 0;
  }
  else {
    // Create a cdrAnyMemoryStream referencing the data.
    pd_mbuf = new cdrAnyMemoryStream(value, release);
    pd_data = 0;
    pd_marshal = 0;
    pd_destructor = 0;
  }
}

void
CORBA::Any::replace(TypeCode_ptr tc, void* value, Boolean release)
{
  dupTC(pd_tc, tc);

  PR_clearData();

  if (value == 0) {
    // No value yet.
    pd_mbuf = 0;
    pd_data = 0;
    pd_marshal = 0;
    pd_destructor = 0;
  }
  else {
    // Create a cdrAnyMemoryStream referencing the data.
    pd_mbuf = new cdrAnyMemoryStream(value, release);
    pd_data = 0;
    pd_marshal = 0;
    pd_destructor = 0;
  }
}

//////////////////////////////////////////////////////////////////////
////////////////// Marshalling operators /////////////////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::Any::operator>>= (cdrStream& s) const
{
  if (orbParameters::tcAliasExpand) {
    CORBA::TypeCode_var tc = TypeCode_base::aliasExpand(ToTcBase(get(pd_tc)));
    CORBA::TypeCode::marshalTypeCode(tc, s);
  }
  else
    CORBA::TypeCode::marshalTypeCode(get(pd_tc), s);

  cdrAnyMemoryStream* snap_mbuf;
  void*               snap_data;
  pr_marshal_fn       snap_marshal;

  {
    SNAP_LOCK;
    snap_mbuf	 = pd_mbuf;
    snap_data	 = pd_data;
    snap_marshal = pd_marshal;
  }

  if (snap_data) {
    OMNIORB_ASSERT(snap_marshal);
    snap_marshal(s, snap_data);
  }
  else if (snap_mbuf) {
    tcParser::copyMemStreamToStream_rdonly(get(pd_tc), *snap_mbuf, s);
  }
  else {
    CORBA::TCKind kind = get(pd_tc)->kind();
    if (kind == CORBA::tk_objref ||
        kind == CORBA::tk_value ||
	kind == CORBA::tk_value_box ||
	kind == CORBA::tk_abstract_interface) {

      // Nil objref / value
      OMNIORB_ASSERT(snap_marshal);
      snap_marshal(s, snap_data);
    }
    else {
      // No data
      OMNIORB_ASSERT(kind == CORBA::tk_void || kind == CORBA::tk_null);
    }
  }
}

void
CORBA::Any::operator<<= (cdrStream& s)
{
  PR_clearData();

  grabTC(pd_tc, CORBA::TypeCode::unmarshalTypeCode(s));
  pd_mbuf = new cdrAnyMemoryStream;
  tcParser::copyStreamToStream(get(pd_tc), s, *pd_mbuf);
}

// omniORB data-only marshalling functions
void
CORBA::Any::NP_marshalDataOnly(cdrStream& s) const
{
  cdrAnyMemoryStream* snap_mbuf;
  void*               snap_data;
  pr_marshal_fn       snap_marshal;

  {
    SNAP_LOCK;
    snap_mbuf	 = pd_mbuf;
    snap_data	 = pd_data;
    snap_marshal = pd_marshal;
  }

  if (snap_data) {
    OMNIORB_ASSERT(snap_marshal);
    snap_marshal(s, snap_data);
  }
  else if (snap_mbuf) {
    tcParser::copyMemStreamToStream_rdonly(get(pd_tc), *snap_mbuf, s);
  }
  else {
    CORBA::TCKind kind = get(pd_tc)->kind();
    if (kind == CORBA::tk_objref ||
        kind == CORBA::tk_value ||
	kind == CORBA::tk_value_box ||
	kind == CORBA::tk_abstract_interface) {

      // Nil objref / value
      OMNIORB_ASSERT(snap_marshal);
      snap_marshal(s, snap_data);
    }
    else {
      // No data
      OMNIORB_ASSERT(kind == CORBA::tk_void || kind == CORBA::tk_null);
    }
  }
}

void
CORBA::Any::NP_unmarshalDataOnly(cdrStream& s)
{
  PR_clearData();
  pd_mbuf = new cdrAnyMemoryStream;
  tcParser::copyStreamToMemStream_flush(get(pd_tc), s, *pd_mbuf);
}


//////////////////////////////////////////////////////////////////////
////////////////// Insertion / extraction functions //////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::Any::
PR_insert(CORBA::TypeCode_ptr newtc, pr_marshal_fn marshal, void* data)
{
  PR_clearData();
  dupTC(pd_tc, newtc);
  pd_mbuf = new cdrAnyMemoryStream();
  marshal(*pd_mbuf, data);
}

void
CORBA::Any::
PR_insert(CORBA::TypeCode_ptr newtc, pr_marshal_fn marshal,
	  pr_destructor_fn destructor, void* data)
{
  PR_clearData();
  dupTC(pd_tc, newtc);
  pd_data = data;
  pd_marshal = marshal;
  pd_destructor = destructor;
}


CORBA::Boolean
CORBA::Any::
PR_extract(CORBA::TypeCode_ptr tc,
	   pr_unmarshal_fn     unmarshal,
	   void*               data) const
{
  if (!tc->equivalent(get(pd_tc)))
    return 0;

  if (pd_mbuf) {
    // Make a temporary stream wrapper around memory buffer.
    cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
    
    // Extract the data
    unmarshal(tbuf, data);
    return 1;
  }
  else {
    OMNIORB_ASSERT(!pd_data);
    return 0;
  }
}


CORBA::Boolean
CORBA::Any::
PR_extract(CORBA::TypeCode_ptr     tc,
	   pr_unmarshal_fn  	   unmarshal,
	   pr_marshal_fn    	   marshal,
	   pr_destructor_fn 	   destructor,
	   void*&                  data) const
{
  if (!tc->equivalent(get(pd_tc)))
    return 0;

  void*               snap_data;
  cdrAnyMemoryStream* snap_mbuf;

  {
    SNAP_LOCK;
    snap_data = pd_data;
    snap_mbuf = pd_mbuf;
  }

  if (snap_data) {
    data = snap_data;
    return 1;
  }
  else if (snap_mbuf) {
    {
      // Make a temporary stream wrapper around memory buffer.
      cdrAnyMemoryStream tbuf(*snap_mbuf, 1);

      // Extract the data
      data = 0;
      unmarshal(tbuf, data);
    }

    // Now set the data pointer
    CORBA::Boolean race = 0;
    {
      omni_tracedmutex_lock l(anyLock);

      if (!pd_data) {
	CORBA::Any* me = OMNI_CONST_CAST(CORBA::Any*, this);

	me->pd_data = data;
	me->pd_marshal = marshal;
	me->pd_destructor = destructor;
      }
      else {
	// Another thread got there first. We destroy the data we just
	// extracted, and return what the other thread made.
	race = 1;
	snap_data = pd_data;
      }
    }
    if (race) {
      destructor(data);
      data = snap_data;
    }
    return 1;
  }
  else {
    return 0;
  }
}

cdrAnyMemoryStream&
CORBA::Any::PR_streamToRead() const
{
  cdrAnyMemoryStream* snap_mbuf;

  {
    SNAP_LOCK;
    snap_mbuf = pd_mbuf;
  }

  if (!snap_mbuf) {

    if (pd_marshal) {
      snap_mbuf = new cdrAnyMemoryStream;
      pd_marshal(*snap_mbuf, pd_data);
    }
    else {
      CORBA::TCKind kind = get(pd_tc)->kind();
      if (kind == CORBA::tk_void || kind == CORBA::tk_null) {
        snap_mbuf = cdrAnyMemoryStream::_empty;
        snap_mbuf->add_ref();
      }
      else {
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidAny, CORBA::COMPLETED_NO);
      }
    }

    {
      omni_tracedmutex_lock l(anyLock);

      if (pd_mbuf) {
	// Another thread beat us to it
        snap_mbuf->remove_ref();
	snap_mbuf = pd_mbuf;
      }
      else {
	CORBA::Any* me = OMNI_CONST_CAST(CORBA::Any*, this);
	me->pd_mbuf = snap_mbuf;
      }
    }
  }
  return *snap_mbuf;
}

cdrAnyMemoryStream&
CORBA::Any::PR_streamToWrite()
{
  PR_clearData();
  pd_mbuf = new cdrAnyMemoryStream;
  return *pd_mbuf;
}


//////////////////////////////////////////////////////////////////////
////////////////// Simple insertion operators ////////////////////////
//////////////////////////////////////////////////////////////////////

// Simple types always go in the memory buffer.

void
CORBA::Any::operator<<=(Short s)
{
  PR_clearData();
  dupTC(pd_tc, CORBA::_tc_short);
  pd_mbuf = new cdrAnyMemoryStream();
  s >>= *pd_mbuf;
}


void CORBA::Any::operator<<=(UShort u)
{
  PR_clearData();
  dupTC(pd_tc, CORBA::_tc_ushort);
  pd_mbuf = new cdrAnyMemoryStream();
  u >>= *pd_mbuf;
}


void
CORBA::Any::operator<<=(Long l)
{
  PR_clearData();
  dupTC(pd_tc, CORBA::_tc_long);
  pd_mbuf = new cdrAnyMemoryStream();
  l >>= *pd_mbuf;
}


void
CORBA::Any::operator<<=(ULong u)
{
  PR_clearData();
  dupTC(pd_tc, CORBA::_tc_ulong);
  pd_mbuf = new cdrAnyMemoryStream();
  u >>= *pd_mbuf;
}

#ifdef HAS_LongLong
void
CORBA::Any::operator<<=(LongLong l)
{
  PR_clearData();
  dupTC(pd_tc, CORBA::_tc_longlong);
  pd_mbuf = new cdrAnyMemoryStream();
  l >>= *pd_mbuf;
}

void
CORBA::Any::operator<<=(ULongLong u)
{
  PR_clearData();
  dupTC(pd_tc, CORBA::_tc_ulonglong);
  pd_mbuf = new cdrAnyMemoryStream();
  u >>= *pd_mbuf;
}
#endif


#if !defined(NO_FLOAT)
void
CORBA::Any::operator<<=(Float f)
{
  PR_clearData();
  dupTC(pd_tc, CORBA::_tc_float);
  pd_mbuf = new cdrAnyMemoryStream();
  f >>= *pd_mbuf;
}

void
CORBA::Any::operator<<=(Double d)
{
  PR_clearData();
  dupTC(pd_tc, CORBA::_tc_double);
  pd_mbuf = new cdrAnyMemoryStream();
  d >>= *pd_mbuf;
}

#ifdef HAS_LongDouble
void
CORBA::Any::operator<<=(LongDouble d)
{
  PR_clearData();
  dupTC(pd_tc, CORBA::_tc_longdouble);
  pd_mbuf = new cdrAnyMemoryStream();
  d >>= *pd_mbuf;
}
#endif

#endif


void
CORBA::Any::operator<<=(from_boolean b)
{
  PR_clearData();
  dupTC(pd_tc, CORBA::_tc_boolean);
  pd_mbuf = new cdrAnyMemoryStream();
  pd_mbuf->marshalBoolean(b.val);
}

void
CORBA::Any::operator<<=(from_char c)
{
  PR_clearData();
  dupTC(pd_tc, CORBA::_tc_char);
  pd_mbuf = new cdrAnyMemoryStream();
  pd_mbuf->marshalChar(c.val);
}

void
CORBA::Any::operator<<=(from_wchar c)
{
  PR_clearData();
  dupTC(pd_tc, CORBA::_tc_wchar);
  pd_mbuf = new cdrAnyMemoryStream();
  pd_mbuf->marshalWChar(c.val);
}

void 
CORBA::Any::operator<<=(from_octet o)
{
  PR_clearData();
  dupTC(pd_tc, CORBA::_tc_octet);
  pd_mbuf = new cdrAnyMemoryStream();
  pd_mbuf->marshalOctet(o.val);
}

void
CORBA::Any::operator<<=(from_fixed f)
{
  PR_clearData();
  CORBA::Fixed g(f.val);
  g.PR_setLimits(f.digits, f.scale);
  pd_tc = CORBA::TypeCode::NP_fixed_tc(f.digits,f.scale);
  pd_mbuf = new cdrAnyMemoryStream();
  g >>= *pd_mbuf;
}


//////////////////////////////////////////////////////////////////////
////////////////// Simple extraction operators ///////////////////////
//////////////////////////////////////////////////////////////////////


CORBA::Boolean 
CORBA::Any::operator>>=(Short& s) const
{
  if (!get(pd_tc)->equivalent(CORBA::_tc_short)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  s <<= tbuf;
  return 1;
}

CORBA::Boolean
CORBA::Any::operator>>=(UShort& u) const
{
  if (!get(pd_tc)->equivalent(CORBA::_tc_ushort)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  u <<= tbuf;
  return 1;
}

CORBA::Boolean
CORBA::Any::operator>>=(Long& l) const
{
  if (!get(pd_tc)->equivalent(CORBA::_tc_long)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  l <<= tbuf;
  return 1;
}


CORBA::Boolean
CORBA::Any::operator>>=(ULong& u) const
{
  if (!get(pd_tc)->equivalent(CORBA::_tc_ulong)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  u <<= tbuf;
  return 1;
}


#ifdef HAS_LongLong
CORBA::Boolean
CORBA::Any::operator>>=(LongLong& l) const
{
  if (!get(pd_tc)->equivalent(CORBA::_tc_longlong)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  l <<= tbuf;
  return 1;
}

  
CORBA::Boolean
CORBA::Any::operator>>=(ULongLong& u) const
{
  if (!get(pd_tc)->equivalent(CORBA::_tc_ulonglong)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  u <<= tbuf;
  return 1;
}
#endif


#if !defined(NO_FLOAT)
CORBA::Boolean
CORBA::Any::operator>>=(Float& f) const
{
  if (!get(pd_tc)->equivalent(CORBA::_tc_float)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  f <<= tbuf;
  return 1;
}


CORBA::Boolean
CORBA::Any::operator>>=(Double& d) const
{
  if (!get(pd_tc)->equivalent(CORBA::_tc_double)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  d <<= tbuf;
  return 1;
}

#ifdef HAS_LongDouble
CORBA::Boolean
CORBA::Any::operator>>=(LongDouble& d) const
{
  if (!get(pd_tc)->equivalent(CORBA::_tc_longdouble)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  d <<= tbuf;
  return 1;
}
#endif

#endif


CORBA::Boolean
CORBA::Any::operator>>=(to_boolean b) const
{
  if (!get(pd_tc)->equivalent(CORBA::_tc_boolean)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  b.ref = tbuf.unmarshalBoolean();
  return 1;
}


CORBA::Boolean
CORBA::Any::operator>>=(to_char c) const
{
  if (!get(pd_tc)->equivalent(CORBA::_tc_char)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  c.ref = tbuf.unmarshalChar();
  return 1;
}

    
CORBA::Boolean
CORBA::Any::operator>>=(to_wchar c) const
{
  if (!get(pd_tc)->equivalent(CORBA::_tc_wchar)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  c.ref = tbuf.unmarshalWChar();
  return 1;
}

    
CORBA::Boolean
CORBA::Any::operator>>=(to_octet o) const
{
  if (!get(pd_tc)->equivalent(CORBA::_tc_octet)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  o.ref = tbuf.unmarshalOctet();
  return 1;
}


CORBA::Boolean
CORBA::Any::operator>>=(to_fixed f) const
{
  CORBA::TypeCode_var tc = CORBA::TypeCode::NP_fixed_tc(f.digits,f.scale);

  if (!get(pd_tc)->equivalent(tc)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);

  CORBA::Fixed g;
  g.PR_setLimits(f.digits, f.scale);
  g <<= tbuf;

  f.val = g;
  return 1;
}



//////////////////////////////////////////////////////////////////////
/////////////// Complex insertion/extraction operators ///////////////
//////////////////////////////////////////////////////////////////////

// Any

static void marshalAny_fn(cdrStream& s, void* d)
{
  CORBA::Any* a = (CORBA::Any*)d;
  *a >>= s;
}
static void unmarshalAny_fn(cdrStream& s, void*& d)
{
  CORBA::Any* a = new CORBA::Any;
  *a <<= s;
  d = a;
}
static void deleteAny_fn(void* d)
{
  CORBA::Any* a = (CORBA::Any*)d;
  delete a;
}

void
CORBA::Any::operator<<=(const Any& a)
{
  CORBA::Any* na = new CORBA::Any(a);
  PR_insert(CORBA::_tc_any, marshalAny_fn, deleteAny_fn, na);
}

void
CORBA::Any::operator<<=(Any* a)
{
  PR_insert(CORBA::_tc_any, marshalAny_fn, deleteAny_fn, a);
}

CORBA::Boolean CORBA::Any::operator>>=(const CORBA::Any*& a) const
{
  void* v;

  if (PR_extract(CORBA::_tc_any,
		 unmarshalAny_fn, marshalAny_fn, deleteAny_fn, v)) {

    a = (const CORBA::Any*)v;
    return 1;
  }
  return 0;
}

// Deprecated non-const version.
CORBA::Boolean CORBA::Any::operator>>=(CORBA::Any*& a) const
{
  return this->operator>>=((const CORBA::Any*&) a);
}

// Obsolete pre-CORBA 2.3 operator.
CORBA::Boolean CORBA::Any::operator>>=(Any& a) const
{
  const CORBA::Any* ap;
  if (*this >>= ap) {
    a = *ap;
    return 1;
  }
  return 0;
}


// TypeCode

static void marshalTypeCode_fn(cdrStream& s, void* d)
{
  CORBA::TypeCode_ptr t = (CORBA::TypeCode_ptr)d;
  CORBA::TypeCode::marshalTypeCode(t, s);
}
static void unmarshalTypeCode_fn(cdrStream& s, void*& d)
{
  CORBA::TypeCode_ptr t = CORBA::TypeCode::unmarshalTypeCode(s);
  d = t;
}
static void deleteTypeCode_fn(void* d)
{
  CORBA::TypeCode_ptr t = (CORBA::TypeCode_ptr)d;
  CORBA::release(t);
}

void
CORBA::Any::operator<<=(TypeCode_ptr tc)
{
  if (!CORBA::TypeCode::PR_is_valid(tc)) {
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidTypeCode,CORBA::COMPLETED_NO);
  }
  CORBA::TypeCode_ptr ntc = CORBA::TypeCode::_duplicate(tc);
  PR_insert(CORBA::_tc_TypeCode, marshalTypeCode_fn, deleteTypeCode_fn, ntc);
}
void
CORBA::Any::operator<<=(TypeCode_ptr* tcp)
{
  if (!CORBA::TypeCode::PR_is_valid(*tcp)) {
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidTypeCode,CORBA::COMPLETED_NO);
  }
  PR_insert(CORBA::_tc_TypeCode, marshalTypeCode_fn, deleteTypeCode_fn, *tcp);
  *tcp = CORBA::TypeCode::_nil();
}
CORBA::Boolean
CORBA::Any::operator>>=(CORBA::TypeCode_ptr& tc) const
{
  void* v;
  if (PR_extract(CORBA::_tc_TypeCode,
		 unmarshalTypeCode_fn, marshalTypeCode_fn, deleteTypeCode_fn,
		 v)) {
    tc = (CORBA::TypeCode_ptr)v;
    return 1;
  }
  return 0;
}


// Object

static void marshalObject_fn(cdrStream& s, void* v)
{
  omniObjRef* o = (omniObjRef*)v;
  omniObjRef::_marshal(o, s);
}
static void unmarshalObject_fn(cdrStream& s, void*& v)
{
  omniObjRef* o = omniObjRef::_unMarshal(CORBA::Object::_PD_repoId, s);
  v = o;
}
static void deleteObject_fn(void* v)
{
  omniObjRef* o = (omniObjRef*)v;
  if (o)
    omni::releaseObjRef(o);
}

void
CORBA::Any::operator<<=(Object_ptr obj)
{
  if (!CORBA::Object::_PR_is_valid(obj)) {
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidObjectRef,CORBA::COMPLETED_NO);
  }
#if 1
  // Use the most derived interface for the TypeCode, not base CORBA::Object
  // *** Is this right?  The C++ mapping is silent on the issue.
  const char* repoid = CORBA::Object::_PD_repoId;
  const char* name   = "";
  if (!CORBA::is_nil(obj))
    repoid = obj->_PR_getobj()->_mostDerivedRepoId();
  CORBA::TypeCode_var tc = CORBA::TypeCode::NP_interface_tc(repoid,name);
#else
  CORBA::TypeCode_ptr tc = CORBA::_tc_Object;
#endif

  CORBA::Object_ptr no = CORBA::Object::_duplicate(obj);
  PR_insert(tc, marshalObject_fn, deleteObject_fn, no->_PR_getobj());
}

void
CORBA::Any::operator<<=(Object_ptr* objp)
{
  if (!CORBA::Object::_PR_is_valid(*objp)) {
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidObjectRef,CORBA::COMPLETED_NO);
  }
#if 1
  const char* repoid = CORBA::Object::_PD_repoId;
  const char* name   = "";
  if (!CORBA::is_nil(*objp))
    repoid = (*objp)->_PR_getobj()->_mostDerivedRepoId();
  CORBA::TypeCode_var tc = CORBA::TypeCode::NP_interface_tc(repoid,name);
#else
  CORBA::TypeCode_ptr tc = CORBA::_tc_Object;
#endif

  PR_insert(tc, marshalObject_fn, deleteObject_fn, (*objp)->_PR_getobj());
  *objp = CORBA::Object::_nil();
}

CORBA::Boolean
CORBA::Any::operator>>=(CORBA::Object_ptr& obj) const
{
  void* v;
  if (PR_extract(CORBA::_tc_Object,
		 unmarshalObject_fn, marshalObject_fn, deleteObject_fn,
		 v)) {
    omniObjRef* r = (omniObjRef*)v;
    if (r)
      obj = (CORBA::Object_ptr)r->_ptrToObjRef(CORBA::Object::_PD_repoId);
    else
      obj = CORBA::Object::_nil();
    return 1;
  }
  return 0;
}


// Abstract Interface

static void marshalAbstractInterface_fn(cdrStream& s, void* v)
{
  CORBA::AbstractBase* a = (CORBA::AbstractBase*)v;
  if (v) {
    CORBA::ValueBase* b = a->_NP_to_value();
    if (b) {
      s.marshalBoolean(0);
      CORBA::ValueBase::_NP_marshal(b,s);
      return;
    }
    CORBA::Object_ptr o = a->_NP_to_object();
    if (o) {
      s.marshalBoolean(1);
      omniObjRef::_marshal(o->_PR_getobj(),s);
      return;
    }
  }
  s.marshalBoolean(0);
  CORBA::ValueBase::_NP_marshal(0,s);
}
static void unmarshalAbstractInterface_fn(cdrStream& s, void*& v)
{
  CORBA::AbstractBase* a;
  CORBA::Boolean c = s.unmarshalBoolean();
  if (c) {
    omniObjRef* o = omniObjRef::_unMarshal(CORBA::Object::_PD_repoId,s);
    if (o) {
      a = (CORBA::AbstractBase*)o->_ptrToObjRef(CORBA::AbstractBase::_PD_repoId);
      if (!a)
	OMNIORB_THROW(BAD_PARAM,
		      BAD_PARAM_IncorrectAbstractIntfType,
		      (CORBA::CompletionStatus)s.completion());
    }
    else
      a = 0;
  }
  else {
    CORBA::ValueBase* b = CORBA::ValueBase::_NP_unmarshal(s);
    if (b) {
      a = (CORBA::AbstractBase*)b->_ptrToValue(CORBA::AbstractBase::_PD_repoId);
      if (!a)
	OMNIORB_THROW(BAD_PARAM,
		      BAD_PARAM_IncorrectAbstractIntfType,
		      (CORBA::CompletionStatus)s.completion());
    }
    else
      a = 0;
  }
  v = a;
}
static void deleteAbstractInterface_fn(void* v)
{
  CORBA::AbstractBase_ptr a = (CORBA::AbstractBase_ptr)v;
  if (a)
    CORBA::release(a);
}


// to_object, to_abstract_base, to_value

CORBA::Boolean
CORBA::Any::operator>>=(to_object o) const
{
  void* v;

  CORBA::TCKind kind = get(pd_tc)->kind();
  
  if (kind == CORBA::tk_objref) {
    // We call PR_extract giving it our own TypeCode, so its type check
    // always succeeds, whatever specific object reference type we
    // contain.
    //
    // Unlike other extraction operators, the caller takes ownership
    // of the returned reference here.

    if (PR_extract(get(pd_tc),
		   unmarshalObject_fn, marshalObject_fn, deleteObject_fn,
		   v)) {

      omniObjRef* r = (omniObjRef*)v;
      if (r)
	o.ref = CORBA::Object::_duplicate(
	        (CORBA::Object_ptr)r->_ptrToObjRef(CORBA::Object::_PD_repoId));
      else
	o.ref = CORBA::Object::_nil();

      return 1;
    }
  }
  else if (kind == CORBA::tk_abstract_interface) {
    if (PR_extract(get(pd_tc),
		   unmarshalAbstractInterface_fn,
		   marshalAbstractInterface_fn,
		   deleteAbstractInterface_fn,
		   v)) {
      CORBA::AbstractBase* a = (CORBA::AbstractBase*)v;
      if (a) {
	if (a->_NP_to_value())
	  return 0;
	o.ref = a->_to_object();
      }
      else
	o.ref = CORBA::Object::_nil();
      return 1;
    }
  }
  return 0;
}

CORBA::Boolean
CORBA::Any::operator>>=(to_abstract_base a) const
{
  void* v;

  if (get(pd_tc)->kind() != CORBA::tk_abstract_interface)
    return 0;

  if (PR_extract(get(pd_tc),
		 unmarshalAbstractInterface_fn,
		 marshalAbstractInterface_fn,
		 deleteAbstractInterface_fn,
		 v)) {
    a.ref = CORBA::AbstractBase::_duplicate((CORBA::AbstractBase*)v);
    return 1;
  }
  return 0;
}

CORBA::Boolean
CORBA::Any::operator>>=(to_value o) const
{
  void* v;

  CORBA::TCKind kind = get(pd_tc)->kind();
  
  if (kind == CORBA::tk_value || kind == CORBA::tk_value_box) {
    // When values are stored by pointer in pd_data, they are stored
    // as the most derived type. That means we can't use the pointer
    // as ValueBase here. Instead, we use a temporary memory buffer.
    // That's not as inefficient as it might seem, because the
    // cdrAnyMemoryStream stores valuetypes by pointer anyway.
    if (pd_mbuf) {
      o.ref = CORBA::ValueBase::_NP_unmarshal(*pd_mbuf);
      return 1;
    }
    else {
      OMNIORB_ASSERT(pd_data);
      OMNIORB_ASSERT(pd_marshal);
      cdrAnyMemoryStream tmp;
      pd_marshal(tmp, pd_data);
      o.ref = CORBA::ValueBase::_NP_unmarshal(tmp);
      return 1;
    }
  }
  else if (kind == CORBA::tk_abstract_interface) {
    if (PR_extract(get(pd_tc),
		   unmarshalAbstractInterface_fn,
		   marshalAbstractInterface_fn,
		   deleteAbstractInterface_fn,
		   v)) {
      CORBA::AbstractBase* a = (CORBA::AbstractBase*)v;
      if (a) {
	o.ref = a->_to_value();
	if (o.ref == 0)
	  return 0;
      }
      else
	o.ref = 0;
      return 1;
    }
  }
  return 0;
}


// String

static void marshalString_fn(cdrStream& s, void* d)
{
  s.marshalString((const char*)d);
}
static void unmarshalString_fn(cdrStream& s, void*& d)
{
  char* c = s.unmarshalString();
  d = c;
}
static void deleteString_fn(void* d)
{
  CORBA::string_free((char*)d);
}

void
CORBA::Any::operator<<=(const char* s)
{
  PR_clearData();
  dupTC(pd_tc, CORBA::_tc_string);

  char* ns = CORBA::string_dup(s);
  PR_insert(CORBA::_tc_string, marshalString_fn, deleteString_fn, ns);
}

void 
CORBA::Any::operator<<=(from_string s)
{
  CORBA::TypeCode_ptr tc;

  if (s.bound)
    tc = CORBA::TypeCode::NP_string_tc(s.bound);
  else
    tc = CORBA::TypeCode::_duplicate(CORBA::_tc_string);

  PR_clearData();
  grabTC(pd_tc, tc);

  pd_data       = s.nc ? s.val : CORBA::string_dup(s.val);
  pd_marshal    = marshalString_fn;
  pd_destructor = deleteString_fn;
}

CORBA::Boolean
CORBA::Any::operator>>=(const char*& s) const
{
  void* v;
  if (PR_extract(CORBA::_tc_string,
		 unmarshalString_fn, marshalString_fn, deleteString_fn,
		 v)) {
    s = (const char*)v;
    return 1;
  }
  return 0;
}

CORBA::Boolean
CORBA::Any::operator>>=(to_string s) const
{
  CORBA::TypeCode_var newtc = CORBA::TypeCode::NP_string_tc(s.bound);

  void* v;
  if (PR_extract(newtc,
		 unmarshalString_fn, marshalString_fn, deleteString_fn,
		 v)) {
    s.val = (char*)v;
    return 1;
  }
  return 0;
}


// Wstring

static void marshalWString_fn(cdrStream& s, void* d)
{
  s.marshalWString((const CORBA::WChar*)d);
}
static void unmarshalWString_fn(cdrStream& s, void*& d)
{
  CORBA::WChar* c = s.unmarshalWString();
  d = c;
}
static void deleteWString_fn(void* d)
{
  CORBA::wstring_free((CORBA::WChar*)d);
}

void
CORBA::Any::operator<<=(const CORBA::WChar* s)
{
  PR_clearData();
  dupTC(pd_tc, CORBA::_tc_string);

  CORBA::WChar* ns = CORBA::wstring_dup(s);
  PR_insert(CORBA::_tc_wstring, marshalWString_fn, deleteWString_fn, ns);
}  

void 
CORBA::Any::operator<<=(from_wstring s)
{
  CORBA::TypeCode_ptr tc;

  if (s.bound)
    tc = CORBA::TypeCode::NP_wstring_tc(s.bound);
  else
    tc = CORBA::TypeCode::_duplicate(CORBA::_tc_wstring);

  PR_clearData();
  grabTC(pd_tc, tc);

  pd_data       = s.nc ? s.val : CORBA::wstring_dup(s.val);
  pd_marshal    = marshalWString_fn;
  pd_destructor = deleteWString_fn;
}

CORBA::Boolean
CORBA::Any::operator>>=(const CORBA::WChar*& s) const
{
  void* v;
  if (PR_extract(CORBA::_tc_wstring,
		 unmarshalWString_fn, marshalWString_fn, deleteWString_fn,
		 v)) {
    s = (const CORBA::WChar*)v;
    return 1;
  }
  return 0;
}

CORBA::Boolean
CORBA::Any::operator>>=(to_wstring s) const
{
  CORBA::TypeCode_var newtc = CORBA::TypeCode::NP_wstring_tc(s.bound);

  void* v;
  if (PR_extract(newtc,
		 unmarshalWString_fn, marshalWString_fn, deleteWString_fn,
		 v)) {
    s.val = (CORBA::WChar*)v;
    return 1;
  }
  return 0;
}


CORBA::Boolean
CORBA::Any::operator>>=(const CORBA::SystemException*& e) const
{
  CORBA::Boolean r;
#define EXTRACT_IF_MATCH(name) \
  if (get(pd_tc)->equivalent(CORBA::_tc_##name)) {	\
    const CORBA::name* ex; \
    r = *this >>= ex; \
    e = ex; \
    return r; \
  }
  OMNIORB_FOR_EACH_SYS_EXCEPTION(EXTRACT_IF_MATCH)
#undef EXTRACT_IF_MATCH

  return 0;
}


CORBA::TypeCode_ptr
CORBA::Any::type() const
{
  return CORBA::TypeCode::_duplicate(get(pd_tc));
}

CORBA::TypeCode_ptr
CORBA::Any::NP_type() const
{
  return get(pd_tc);
}

void
CORBA::Any::type(CORBA::TypeCode_ptr tc)
{
  if (!get(pd_tc)->equivalent(tc))
    OMNIORB_THROW(BAD_TYPECODE,
		  BAD_TYPECODE_NotEquivalent,
		  CORBA::COMPLETED_NO);

  dupTC(pd_tc, tc);
}

const void*
CORBA::Any::value() const
{
  if (get(pd_tc)->kind() == CORBA::tk_null ||
      get(pd_tc)->kind() == CORBA::tk_void)
    return 0;

  cdrAnyMemoryStream* snap_mbuf;

  {
    SNAP_LOCK;
    snap_mbuf = pd_mbuf;
  }

  if (!snap_mbuf) {
    OMNIORB_ASSERT(pd_marshal);

    // We create a memory buffer and marshal our value into it. Note
    // that this will result in invalid data if we contain a
    // valuetype, since valuetypes do not get marshalled into the
    // memory buffer like other types. This value() method was
    // deprecated before valuetypes were specifified, so we consider
    // this an acceptable limitation.
    cdrAnyMemoryStream* mbuf = new cdrAnyMemoryStream;

    pd_marshal(*mbuf, pd_data);

    {
      omni_tracedmutex_lock l(anyLock);

      if (pd_mbuf) {
	// Another thread beat us to it
	delete mbuf;
	snap_mbuf = pd_mbuf;
      }
      else {
	CORBA::Any* me = OMNI_CONST_CAST(CORBA::Any*, this);
	snap_mbuf = me->pd_mbuf = mbuf;
      }
    }
  }
  return snap_mbuf->bufPtr();
}
