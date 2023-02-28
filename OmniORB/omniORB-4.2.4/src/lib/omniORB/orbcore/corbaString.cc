// -*- Mode: C++; -*-
//                            Package   : omniORB
// corbaString.cc             Created on: 20/9/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2005-2006 Apasphere Ltd
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
//    Implementation of the String interface.
//	

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <exceptiondefs.h>

const char*const _CORBA_String_helper::empty_string = "";

OMNI_USING_NAMESPACE(omni)

char*
CORBA::string_alloc(CORBA::ULong len)
{
  // We initialise the string to zero length to help prevent errors
  // if this string is copied before it is initialised.  This is easy
  // to do when assigning the returned value to a CORBA::String_var.
  char* s = _CORBA_String_helper::alloc(len);
  if( s )  *s = '\0';
  return s;
}


void
CORBA::string_free(char* p)
{
  _CORBA_String_helper::dealloc(p);
}


char*
CORBA::string_dup(const char* p)
{
  if (p) return _CORBA_String_helper::dup(p);
  return 0;
}

//////////////////////////////////////////////////////////////////////
///////////////////////// cdrStream raw string ///////////////////////
//////////////////////////////////////////////////////////////////////

char*
cdrStream::unmarshalRawString() {
  _CORBA_ULong len; len <<= *this;

  if (len == 0)
    OMNIORB_THROW(MARSHAL,MARSHAL_StringNotEndWithNull,
		  (CORBA::CompletionStatus)completion());

  if (!checkInputOverrun(1, len))
    OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		  (CORBA::CompletionStatus)completion());

  char* s = _CORBA_String_helper::alloc(len - 1);
  get_octet_array((_CORBA_Octet*)s, len);

  if (s[len-1] != '\0')
    OMNIORB_THROW(MARSHAL,MARSHAL_StringNotEndWithNull,
		  (CORBA::CompletionStatus)completion());

  return s;
}

#ifndef Swap32
#define Swap32(l) ((((l) & 0xff000000) >> 24) | \
		   (((l) & 0x00ff0000) >> 8)  | \
		   (((l) & 0x0000ff00) << 8)  | \
		   (((l) & 0x000000ff) << 24))
#else
#error "Swap32 has already been defined"
#endif

_CORBA_ULong
cdrStream::marshalRawString(const char* s)
{
  // In the common case that the string fits inside the stream's
  // buffer, we make a single pass over the string. If the string does
  // not fit inside the bufer, we have to find the length of the
  // remaining portion before marshalling it, meaning some of the
  // string is passed over twice.

  char* current = (char*)omni::align_to((omni::ptr_arith_t)pd_outb_mkr,
					omni::ALIGN_4);
  char* limit   = (char*)pd_outb_end;

  _CORBA_ULong* lenp = (_CORBA_ULong*)current;
  current += 4;
  _CORBA_ULong len;

  if (current >= limit) {
    // No characters from the string will fit in the current buffer.
    // The length might fit, but we can't put it in this buffer since
    // in a chunked stream it must stay with the string contents.
    len = (CORBA::ULong)strlen(s) + 1;
    declareArrayLength(omni::ALIGN_4, len+4);
    len >>= *this;
    put_octet_array((const _CORBA_Octet*)s, len);
    return len;
  }

  pd_outb_mkr = (void*)current;

  // At least some of the string fits in the current buffer.
  while(*s && current < limit) {
    *current++ = *s++;
  }
  if (current < limit) {
    // Good -- the whole string fit in the buffer.
    *current++ = *s;

    len = (_CORBA_ULong)((omni::ptr_arith_t)current -
                         (omni::ptr_arith_t)pd_outb_mkr);
    pd_outb_mkr = (void*)current;

    *lenp = pd_marshal_byte_swap ? Swap32(len) : len;
  }
  else {
    // Some of the string did not fit.
    len = (_CORBA_ULong)((omni::ptr_arith_t)current -
                         (omni::ptr_arith_t)pd_outb_mkr);
    pd_outb_mkr = (void*)current;
    _CORBA_ULong rest = (_CORBA_ULong)strlen(s) + 1;

    len += rest;
    if ((omni::ptr_arith_t)lenp < (omni::ptr_arith_t)pd_outb_end)
      *lenp = pd_marshal_byte_swap ? Swap32(len) : len;

    put_octet_array((const _CORBA_Octet*)s, rest);
  }
  return len;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////// String_member ///////////////////////////
//////////////////////////////////////////////////////////////////////

void
_CORBA_String_member::operator <<= (cdrStream& s)
{
  if( _ptr && _ptr != _CORBA_String_helper::empty_string )
    _CORBA_String_helper::dealloc(_ptr);
  _ptr = 0;

  _ptr = s.unmarshalString();
}

void
_CORBA_String_member::operator >>= (cdrStream& s) const { 
  s.marshalString(_ptr); 
}

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Unbounded_Sequence__String /////////////////
//////////////////////////////////////////////////////////////////////

void
_CORBA_Sequence_String::operator >>= (cdrStream& s) const
{
  pd_len >>= s;

  for( CORBA::ULong i = 0; i < pd_len; i++ ) {
    char* p = pd_data[i];
    s.marshalString(p);
  }
}


void
_CORBA_Sequence_String::operator <<= (cdrStream& s)
{
  _CORBA_ULong slen;
  slen <<= s;

  if (!s.checkInputOverrun(1,slen))
    OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		  (CORBA::CompletionStatus)s.completion());

  if (pd_bounded && slen > pd_max)
    OMNIORB_THROW(MARSHAL,MARSHAL_SequenceIsTooLong,
		  (CORBA::CompletionStatus)s.completion());

  if (!pd_rel && slen <= pd_max) {
    // obtain ownership of the array and its elements (note that this isn't
    // the most effecient solution, but neither is invoking length!)
    copybuffer(pd_len);
  }

  length(slen);

  for( _CORBA_ULong i = 0; i < slen; i++ ) {
    char*& p = (char*&) pd_data[i];

    if( p ) { _CORBA_String_helper::dealloc(p); p = 0; }

    p = s.unmarshalString();
  }
}
