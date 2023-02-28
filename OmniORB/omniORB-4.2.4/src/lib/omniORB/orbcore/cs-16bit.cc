// -*- Mode: C++; -*-
//                            Package   : omniORB
// cs-16bit.cc                Created on: 24/10/2000
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
//    16 bit code sets

#include <omniORB4/CORBA.h>
#include <codeSetUtil.h>

OMNI_USING_NAMESPACE(omni)

OMNI_NAMESPACE_BEGIN(omni)

//
// Native code set
//

void
omniCodeSet::NCS_W_16bit::marshalWChar(cdrStream& stream,
				       omniCodeSet::TCS_W* tcs,
				       _CORBA_WChar wc)
{
  OMNIORB_CHECK_TCS_W_FOR_MARSHAL(tcs, stream);

  if (tcs->fastMarshalWChar(stream, this, wc)) return;

#if (SIZEOF_WCHAR == 4)
  if (wc > 0xffff)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WCharOutOfRange, 
		  (CORBA::CompletionStatus)stream.completion());
#endif

  omniCodeSet::UniChar uc = pd_toU[(wc & 0xff00) >> 8][wc & 0x00ff];
  if (wc && !uc) OMNIORB_THROW(DATA_CONVERSION, 
			       DATA_CONVERSION_CannotMapChar,
			       (CORBA::CompletionStatus)stream.completion());

  tcs->marshalWChar(stream, uc);
}

void
omniCodeSet::NCS_W_16bit::marshalWString(cdrStream&          stream,
					 omniCodeSet::TCS_W* tcs,
					 _CORBA_ULong        bound,
					 _CORBA_ULong        len,
					 const _CORBA_WChar* ws)
{
  OMNIORB_CHECK_TCS_W_FOR_MARSHAL(tcs, stream);

  if (tcs->fastMarshalWString(stream, this, bound, len, ws)) return;

  if (bound && len > bound)
    OMNIORB_THROW(MARSHAL, MARSHAL_WStringIsTooLong, 
		  (CORBA::CompletionStatus)stream.completion());

  omniCodeSet::UniChar*    us = omniCodeSetUtil::allocU(len+1);
  omniCodeSetUtil::HolderU uh(us);
  omniCodeSet::UniChar     uc;
  _CORBA_WChar             wc;

  for (_CORBA_ULong i=0; i<=len; i++) {
    wc = ws[i];
#if (SIZEOF_WCHAR == 4)
    if (wc > 0xffff) 
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WCharOutOfRange, 
		    (CORBA::CompletionStatus)stream.completion());
#endif
    uc = pd_toU[(wc & 0xff00) >> 8][wc & 0x00ff];
    if (wc && !uc) OMNIORB_THROW(DATA_CONVERSION, 
				 DATA_CONVERSION_CannotMapChar,
				 (CORBA::CompletionStatus)stream.completion());
    us[i] = uc;
  }
  tcs->marshalWString(stream, bound, len, us);
}

_CORBA_WChar
omniCodeSet::NCS_W_16bit::unmarshalWChar(cdrStream& stream,
					 omniCodeSet::TCS_W* tcs)
{
  OMNIORB_CHECK_TCS_W_FOR_UNMARSHAL(tcs, stream);

  _CORBA_WChar wc;
  if (tcs->fastUnmarshalWChar(stream, this, wc)) return wc;

  omniCodeSet::UniChar uc = tcs->unmarshalWChar(stream);

  wc = pd_fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
  if (uc && !wc) OMNIORB_THROW(DATA_CONVERSION, 
			       DATA_CONVERSION_CannotMapChar,
			       (CORBA::CompletionStatus)stream.completion());

  return wc;
}

_CORBA_ULong
omniCodeSet::NCS_W_16bit::unmarshalWString(cdrStream& stream,
					   omniCodeSet::TCS_W* tcs,
					   _CORBA_ULong bound,
					   _CORBA_WChar*& ws)
{
  OMNIORB_CHECK_TCS_W_FOR_UNMARSHAL(tcs, stream);

  _CORBA_ULong len;
  if (tcs->fastUnmarshalWString(stream, this, bound, len, ws)) return len;

  omniCodeSet::UniChar* us;
  len = tcs->unmarshalWString(stream, bound, us);
  OMNIORB_ASSERT(us);

  omniCodeSetUtil::HolderU uh(us);

  ws = omniCodeSetUtil::allocW(len+1);
  omniCodeSetUtil::HolderW wh(ws);

  omniCodeSet::UniChar uc;
  _CORBA_WChar         wc;

  for (_CORBA_ULong i=0; i<=len; i++) {
    uc = us[i];
    wc  = pd_fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
    if (uc && !wc) OMNIORB_THROW(DATA_CONVERSION, 
				 DATA_CONVERSION_CannotMapChar,
				 (CORBA::CompletionStatus)stream.completion());
    ws[i] = wc;
  }
  wh.drop();
  return len;
}


//
// Transmission code set
//

void
omniCodeSet::TCS_W_16bit::marshalWChar(cdrStream& stream,
				       omniCodeSet::UniChar uc)
{
  _CORBA_UShort tc = pd_fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
  if (uc && !tc) OMNIORB_THROW(DATA_CONVERSION, 
			       DATA_CONVERSION_CannotMapChar,
			       (CORBA::CompletionStatus)stream.completion());

  // In GIOP 1.2, wchar is encoded as an octet containing a length,
  // followed by that number of octets representing the wchar. The
  // CORBA 2.3 spec is silent on endianness issues, and whether there
  // should be any padding. The 2.4 spec says that if TCS-W is UTF-16,
  // the wchar is marshalled big-endian, unless there is a Unicode
  // byte order mark telling us otherwise. That doesn't help us here,
  // since we're not transmitting UTF-16. We assume here that there is
  // no padding, and we use the stream's endianness.

  stream.declareArrayLength(omni::ALIGN_1, 3);
  stream.marshalOctet(2);

  _CORBA_Octet* p = (_CORBA_Octet*)&tc;
  _CORBA_Octet  o;

  if (stream.marshal_byte_swap()) {
    o = p[1]; stream.marshalOctet(o);
    o = p[0]; stream.marshalOctet(o);
  }
  else {
    o = p[0]; stream.marshalOctet(o);
    o = p[1]; stream.marshalOctet(o);
  }
}

void
omniCodeSet::TCS_W_16bit::marshalWString(cdrStream& stream,
					 _CORBA_ULong bound,
					 _CORBA_ULong len,
					 const omniCodeSet::UniChar* us)
{
  // Just to be different, wstring is marshalled without a terminating
  // null. Length is in octets.
  _CORBA_ULong mlen = len * 2;
  stream.declareArrayLength(omni::ALIGN_4, mlen + 4);
  mlen >>= stream;

  _CORBA_UShort        tc;
  omniCodeSet::UniChar uc;
  
  for (_CORBA_ULong i=0; i<len; i++) {
    uc = us[i];
    tc = pd_fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
    if (uc && !tc) OMNIORB_THROW(DATA_CONVERSION, 
				 DATA_CONVERSION_CannotMapChar,
				 (CORBA::CompletionStatus)stream.completion());

    tc >>= stream;
  }
}

omniCodeSet::UniChar
omniCodeSet::TCS_W_16bit::unmarshalWChar(cdrStream& stream)
{
  _CORBA_UShort tc;
  _CORBA_Octet  len = stream.unmarshalOctet();
  _CORBA_Octet  o;

  switch (len) {
  case 0:
    tc = 0; // Evil but it might happen, I suppose
    break;
  case 1:
    o  = stream.unmarshalOctet();
    tc = o;
    break;
  case 2:
    {
      _CORBA_Octet* p = (_CORBA_Octet*)&tc;
      if (stream.unmarshal_byte_swap()) {
	o = stream.unmarshalOctet(); p[1] = o;
	o = stream.unmarshalOctet(); p[0] = o;
      }
      else {
	o = stream.unmarshalOctet(); p[0] = o;
	o = stream.unmarshalOctet(); p[1] = o;
      }
    }
    break;
  default:
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidWCharSize,
		  (CORBA::CompletionStatus)stream.completion());
  }
  omniCodeSet::UniChar uc = pd_toU[(tc & 0xff00) >> 8][tc & 0x00ff];
  if (tc && !uc) OMNIORB_THROW(DATA_CONVERSION, 
			       DATA_CONVERSION_CannotMapChar,
			       (CORBA::CompletionStatus)stream.completion());

  return uc;
}

_CORBA_ULong
omniCodeSet::TCS_W_16bit::unmarshalWString(cdrStream& stream,
					   _CORBA_ULong bound,
					   omniCodeSet::UniChar*& us)
{
  _CORBA_ULong mlen; mlen <<= stream;

  if (mlen % 2)
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidWCharSize,
		  (CORBA::CompletionStatus)stream.completion());

  _CORBA_ULong len = mlen / 2; // Note no terminating null in marshalled form

  if (bound && len > bound)
    OMNIORB_THROW(MARSHAL, MARSHAL_WStringIsTooLong, 
		  (CORBA::CompletionStatus)stream.completion());
		  

  if (!stream.checkInputOverrun(1, mlen))
    OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		  (CORBA::CompletionStatus)stream.completion());

  us = omniCodeSetUtil::allocU(len + 1);
  omniCodeSetUtil::HolderU uh(us);

  _CORBA_UShort        tc;
  omniCodeSet::UniChar uc;
  _CORBA_ULong         i;

  for (i=0; i < len; i++) {
    tc <<= stream;
    uc = pd_toU[(tc & 0xff00) >> 8][tc & 0x00ff];
    if (tc && !uc) OMNIORB_THROW(DATA_CONVERSION, 
				 DATA_CONVERSION_CannotMapChar,
				 (CORBA::CompletionStatus)stream.completion());
    us[i] = uc;
  }
  us[i] = 0; // Null terminator

  uh.drop();
  return len;
}


_CORBA_Boolean
omniCodeSet::TCS_W_16bit::fastMarshalWChar(cdrStream&          stream,
					   omniCodeSet::NCS_W* ncs,
					   _CORBA_WChar        wc)
{
  if (ncs->id() == id()) { // Null transformation
#if (SIZEOF_WCHAR == 4)
    if (wc > 0xffff)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WCharOutOfRange, 
		    (CORBA::CompletionStatus)stream.completion());
#endif

    stream.declareArrayLength(omni::ALIGN_1, 3);
    stream.marshalOctet(2);

    _CORBA_UShort tc = wc;
    _CORBA_Octet* p  = (_CORBA_Octet*)&tc;
    _CORBA_Octet  o;

    if (stream.marshal_byte_swap()) {
      o = p[1]; stream.marshalOctet(o);
      o = p[0]; stream.marshalOctet(o);
    }
    else {
      o = p[0]; stream.marshalOctet(o);
      o = p[1]; stream.marshalOctet(o);
    }
    return 1;
  }
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_W_16bit::fastMarshalWString(cdrStream&          stream,
					     omniCodeSet::NCS_W* ncs,
					     _CORBA_ULong        bound,
					     _CORBA_ULong        len,
					     const _CORBA_WChar* ws)
{
  if (ncs->id() == id()) { // Null transformation

    if (bound && len > bound)
      OMNIORB_THROW(MARSHAL, MARSHAL_WStringIsTooLong, 
		    (CORBA::CompletionStatus)stream.completion());

    _CORBA_ULong mlen = len * 2;
    stream.declareArrayLength(omni::ALIGN_4, mlen+4);
    mlen >>= stream;

#if (SIZEOF_WCHAR == 2)
    if (stream.marshal_byte_swap()) {
      _CORBA_UShort tc;
      for (_CORBA_ULong i=0; i<len; i++) {
	tc = ws[i]; tc >>= stream;
      }
    }
    else {
      stream.put_octet_array((const _CORBA_Char*)ws, mlen, omni::ALIGN_2);
    }
#else
    _CORBA_UShort tc;
    for (_CORBA_ULong i=0; i<len; i++) {
      tc = ws[i]; tc >>= stream;
    }
#endif
    return 1;
  }
  return 0;
}


_CORBA_Boolean
omniCodeSet::TCS_W_16bit::fastUnmarshalWChar(cdrStream&          stream,
					     omniCodeSet::NCS_W* ncs,
					     _CORBA_WChar&       wc)
{
  if (ncs->id() == id()) { // Null transformation
    _CORBA_Octet len = stream.unmarshalOctet();
    _CORBA_Octet o;

    switch (len) {
    case 0:
      wc = 0; // Evil but it might happen, I suppose
      break;
    case 1:
      o = stream.unmarshalOctet();
      wc = o;
      break;
    case 2:
      {
	_CORBA_UShort tc;
	_CORBA_Octet* p = (_CORBA_Octet*)&tc;
	if (stream.unmarshal_byte_swap()) {
	  o = stream.unmarshalOctet(); p[1] = o;
	  o = stream.unmarshalOctet(); p[0] = o;
	}
	else {
	  o = stream.unmarshalOctet(); p[0] = o;
	  o = stream.unmarshalOctet(); p[1] = o;
	}
	wc = tc;
      }
      break;
    default:
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidWCharSize, CORBA::COMPLETED_NO);
    }
    return 1;
  }
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_W_16bit::fastUnmarshalWString(cdrStream&          stream,
					       omniCodeSet::NCS_W* ncs,
					       _CORBA_ULong        bound,
					       _CORBA_ULong&       len,
					       _CORBA_WChar*&      ws)
{
  if (ncs->id() == id()) { // Null transformation

    _CORBA_ULong mlen; mlen <<= stream;

    if (mlen % 2)
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidWCharSize,
		    (CORBA::CompletionStatus)stream.completion());

    len = mlen / 2; // Note no terminating null in marshalled form

    if (bound && len > bound)
      OMNIORB_THROW(MARSHAL, MARSHAL_WStringIsTooLong, 
		    (CORBA::CompletionStatus)stream.completion());

    if (!stream.checkInputOverrun(1, mlen))
      OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		    (CORBA::CompletionStatus)stream.completion());

    ws = omniCodeSetUtil::allocW(len + 1);
    omniCodeSetUtil::HolderW wh(ws);

#if (SIZEOF_WCHAR == 2)
    stream.unmarshalArrayUShort((_CORBA_UShort*)ws, len);
#else
    _CORBA_UShort tc;
    for (_CORBA_ULong i=0; i < len; i++) {
      tc <<= stream;
      ws[i] = tc;
    }
#endif
    ws[len] = 0; // Null terminator

    wh.drop();
    return 1;
  }
  return 0;
}


const _CORBA_UShort omniCodeSet::empty16BitTable[] = {
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

OMNI_NAMESPACE_END(omni)
