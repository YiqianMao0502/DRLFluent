// -*- Mode: C++; -*-
//                            Package   : omniORB
// cs-8bit.cc                 Created on: 24/10/2000
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2003-2008 Apasphere Ltd
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
//    8 bit code sets

#include <omniORB4/CORBA.h>
#include <codeSetUtil.h>
#include <orbParameters.h>

OMNI_USING_NAMESPACE(omni)

//
// Native code set
//

void
omniCodeSet::NCS_C_8bit::marshalChar(cdrStream& stream,
				     omniCodeSet::TCS_C* tcs,
				     _CORBA_Char c)
{
  if (!tcs) OMNIORB_THROW(BAD_INV_ORDER, 
			  BAD_INV_ORDER_CodeSetNotKnownYet,
			  (CORBA::CompletionStatus)stream.completion());

  if (tcs->fastMarshalChar(stream, this, c)) return;

  omniCodeSet::UniChar uc = pd_toU[c];
  if (c && !uc) OMNIORB_THROW(DATA_CONVERSION, 
			      DATA_CONVERSION_CannotMapChar,
			      (CORBA::CompletionStatus)stream.completion());


  tcs->marshalChar(stream, uc);
}

void
omniCodeSet::NCS_C_8bit::marshalString(cdrStream&          stream,
				       omniCodeSet::TCS_C* tcs,
				       _CORBA_ULong        bound,
				       _CORBA_ULong        len,
				       const char*         s)
{
  if (!tcs) OMNIORB_THROW(BAD_INV_ORDER, 
			  BAD_INV_ORDER_CodeSetNotKnownYet,
			  (CORBA::CompletionStatus)stream.completion());

  if (tcs->fastMarshalString(stream, this, bound, len, s))
    return;

  if (len == 0)
    len = strlen(s);

  omniCodeSet::UniChar*    us = omniCodeSetUtil::allocU(len+1);
  omniCodeSetUtil::HolderU uh(us);
  omniCodeSet::UniChar     uc;

  for (_CORBA_ULong i=0; i<=len; i++) {
    uc = pd_toU[(_CORBA_Char)(s[i])];
    if (!uc && s[i])
      OMNIORB_THROW(DATA_CONVERSION, 
		    DATA_CONVERSION_CannotMapChar,
		    (CORBA::CompletionStatus)stream.completion());
    us[i] = uc;
  }
  tcs->marshalString(stream, bound, len, us);
}

_CORBA_Char
omniCodeSet::NCS_C_8bit::unmarshalChar(cdrStream& stream,
				       omniCodeSet::TCS_C* tcs)
{
  if (!tcs) OMNIORB_THROW(BAD_INV_ORDER, 
			  BAD_INV_ORDER_CodeSetNotKnownYet,
			  (CORBA::CompletionStatus)stream.completion());

  _CORBA_Char c;
  if (tcs->fastUnmarshalChar(stream, this, c)) return c;

  omniCodeSet::UniChar uc = tcs->unmarshalChar(stream);

  c = pd_fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
  if (uc && !c) OMNIORB_THROW(DATA_CONVERSION, 
			      DATA_CONVERSION_CannotMapChar,
			      (CORBA::CompletionStatus)stream.completion());

  return c;
}

_CORBA_ULong
omniCodeSet::NCS_C_8bit::unmarshalString(cdrStream& stream,
					 omniCodeSet::TCS_C* tcs,
					 _CORBA_ULong bound, char*& s)
{
  if (!tcs) OMNIORB_THROW(BAD_INV_ORDER, 
			  BAD_INV_ORDER_CodeSetNotKnownYet,
			  (CORBA::CompletionStatus)stream.completion());

  _CORBA_ULong len;
  if (tcs->fastUnmarshalString(stream, this, bound, len, s)) return len;

  omniCodeSet::UniChar* us;
  len = tcs->unmarshalString(stream, bound, us);
  OMNIORB_ASSERT(us);

  omniCodeSetUtil::HolderU uh(us);

  s = omniCodeSetUtil::allocC(len+1);
  omniCodeSetUtil::HolderC h(s);

  omniCodeSet::UniChar uc;
  _CORBA_Char          c;

  for (_CORBA_ULong i=0; i<=len; i++) {
    uc = us[i];
    c  = pd_fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
    if (uc && !c) OMNIORB_THROW(DATA_CONVERSION, 
				DATA_CONVERSION_CannotMapChar,
				(CORBA::CompletionStatus)stream.completion());
    s[i] = c;
  }
  h.drop();
  return len;
}


//
// Transmission code set
//

void
omniCodeSet::TCS_C_8bit::marshalChar(cdrStream& stream,
				     omniCodeSet::UniChar uc)
{
  _CORBA_Char c = pd_fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
  if (uc && !c) OMNIORB_THROW(DATA_CONVERSION, 
			      DATA_CONVERSION_CannotMapChar,
			      (CORBA::CompletionStatus)stream.completion());

  stream.marshalOctet(c);
}

void
omniCodeSet::TCS_C_8bit::marshalString(cdrStream& stream,
				       _CORBA_ULong bound,
				       _CORBA_ULong len,
				       const omniCodeSet::UniChar* us)
{
  if (bound && len > bound)
    OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, 
		  (CORBA::CompletionStatus)stream.completion());

  len++;
  stream.declareArrayLength(omni::ALIGN_4, len+4);
  len >>= stream;

  _CORBA_Char          c;
  omniCodeSet::UniChar uc;

  for (_CORBA_ULong i=0; i<len; i++) {
    uc = us[i];
    c = pd_fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
    if (uc && !c)  OMNIORB_THROW(DATA_CONVERSION, 
				DATA_CONVERSION_CannotMapChar,
				(CORBA::CompletionStatus)stream.completion());
    stream.marshalOctet(c);
  }
}

omniCodeSet::UniChar
omniCodeSet::TCS_C_8bit::unmarshalChar(cdrStream& stream)
{
  _CORBA_Char c;
  c = stream.unmarshalOctet();

  omniCodeSet::UniChar uc = pd_toU[c];
  if (c && !uc)  OMNIORB_THROW(DATA_CONVERSION, 
			       DATA_CONVERSION_CannotMapChar,
			       (CORBA::CompletionStatus)stream.completion());
  return uc;
}

_CORBA_ULong
omniCodeSet::TCS_C_8bit::unmarshalString(cdrStream& stream,
					 _CORBA_ULong bound,
					 omniCodeSet::UniChar*& us)
{
  _CORBA_ULong mlen; mlen <<= stream;  // Includes terminating null

  if (mlen == 0) {
    if (orbParameters::strictIIOP) {
      omniORB::logs(1, "Error: received an invalid zero length string.");
      OMNIORB_THROW(MARSHAL, MARSHAL_StringNotEndWithNull, 
		    (CORBA::CompletionStatus)stream.completion());
    }
    else {
      omniORB::logs(1, "Warning: received an invalid zero length string. "
		    "Substituted with a proper empty string.");
      us = omniCodeSetUtil::allocU(1);
      us[0] = 0;
      return 0;
    }
  }

  if (bound && mlen-1 > bound)
    OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, 
		  (CORBA::CompletionStatus)stream.completion());

  if (!stream.checkInputOverrun(1, mlen))
    OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		  (CORBA::CompletionStatus)stream.completion());

  us = omniCodeSetUtil::allocU(mlen);
  omniCodeSetUtil::HolderU uh(us);

  _CORBA_Char          c;
  omniCodeSet::UniChar uc;

  for (_CORBA_ULong i=0; i < mlen; i++) {
    c = stream.unmarshalOctet();
    uc = pd_toU[c];
    if (c && !uc) OMNIORB_THROW(DATA_CONVERSION, 
				DATA_CONVERSION_CannotMapChar,
				(CORBA::CompletionStatus)stream.completion());
    us[i] = uc;
  }
  if (uc != 0) // String must end with null
    OMNIORB_THROW(MARSHAL, MARSHAL_StringNotEndWithNull, 
		  (CORBA::CompletionStatus)stream.completion());

  uh.drop();
  return mlen - 1; // Length without terminating null
}

_CORBA_Boolean
omniCodeSet::TCS_C_8bit::fastMarshalChar(cdrStream&          stream,
					 omniCodeSet::NCS_C* ncs,
					 _CORBA_Char         c)
{
  if (ncs->id() == id()) { // Null transformation
    stream.marshalOctet(c);
    return 1;
  }
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_C_8bit::fastMarshalString(cdrStream&          stream,
					   omniCodeSet::NCS_C* ncs,
					   _CORBA_ULong        bound,
					   _CORBA_ULong        len,
					   const char*         s)
{
  if (ncs->id() == id()) { // Null transformation
    if (len == 0) {
      len = stream.marshalRawString(s);

      if (bound && len-1 > bound)
	OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, 
		      (CORBA::CompletionStatus)stream.completion());
    }
    else {
      if (bound && len > bound)
	OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, 
		      (CORBA::CompletionStatus)stream.completion());
      len++;
      stream.declareArrayLength(omni::ALIGN_4, len + 4);
      len >>= stream;
      stream.put_octet_array((const _CORBA_Octet*)s, len);
    }
    return 1;
  }
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_C_8bit::fastUnmarshalChar(cdrStream&          stream,
					   omniCodeSet::NCS_C* ncs,
					   _CORBA_Char&        c)
{
  if (ncs->id() == id()) { // Null transformation
    c = stream.unmarshalOctet();
    return 1;
  }
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_C_8bit::fastUnmarshalString(cdrStream&          stream,
					     omniCodeSet::NCS_C* ncs,
					     _CORBA_ULong        bound,
					     _CORBA_ULong&       len,
					     char*&              s)
{
  if (ncs->id() == id()) { // Null transformation
    _CORBA_ULong mlen; mlen <<= stream;

    if (mlen == 0) {
      if (orbParameters::strictIIOP) {
	omniORB::logs(1, "Error: received an invalid zero length string.");
	OMNIORB_THROW(MARSHAL, MARSHAL_StringNotEndWithNull, 
		      (CORBA::CompletionStatus)stream.completion());
      }
      else {
	omniORB::logs(1, "Warning: received an invalid zero length string. "
		      "Substituted with a proper empty string.");
	s = omniCodeSetUtil::allocC(1);
	s[0] = '\0';
	return 0;
      }
    }

    if (bound && mlen-1 > bound)
      OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, 
		    (CORBA::CompletionStatus)stream.completion());

    if (!stream.checkInputOverrun(1, mlen))
      OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		    (CORBA::CompletionStatus)stream.completion());

    s = omniCodeSetUtil::allocC(mlen);
    omniCodeSetUtil::HolderC h(s);

    stream.get_octet_array((_CORBA_Octet*)s, mlen);
    if (s[mlen-1] != '\0') 
      OMNIORB_THROW(MARSHAL, MARSHAL_StringNotEndWithNull, 
		    (CORBA::CompletionStatus)stream.completion());


    h.drop();
    len = mlen - 1; // Return length without terminating null
    return 1;
  }
  return 0;
}

const _CORBA_Char omniCodeSet::empty8BitTable[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


