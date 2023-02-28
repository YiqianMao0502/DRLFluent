// -*- Mode: C++; -*-
//                            Package   : omniORB
// cdrMemoryStream.cc         Created on: 13/1/99
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2012 Apasphere Ltd
//    Copyright (C) 1999 AT&T Laboratories Cambridge
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

#include <omniORB4/CORBA.h>
#include <orbParameters.h>

OMNI_USING_NAMESPACE(omni)

// We want to ensure that the stream always starts at 8 bytes aligned.
// Call this function with bufp would yield the correctly aligned starting
// point.
static inline void* ensure_align_8(void*p) {
  return (void*)omni::align_to((omni::ptr_arith_t)p,omni::ALIGN_8);
}

cdrMemoryStream::cdrMemoryStream(CORBA::ULong initialBufsize,
				 CORBA::Boolean clearMemory)
{
  pd_readonly_and_external_buffer = 0;
  pd_clear_memory = clearMemory;
  pd_bufp     = pd_inline_buffer;
  pd_bufp_8   = ensure_align_8(pd_bufp);
  pd_outb_end = (pd_inline_buffer + sizeof(pd_inline_buffer));
  rewindPtrs();
  if (initialBufsize > (CORBA::ULong)((omni::ptr_arith_t)pd_outb_end - 
				      (omni::ptr_arith_t)pd_outb_mkr))
    reserveOutputSpace(omni::ALIGN_8,initialBufsize);
  if (pd_clear_memory) memset(pd_bufp,0,
			      (omni::ptr_arith_t)pd_outb_end -
			      (omni::ptr_arith_t)pd_bufp);

  pd_tcs_c = orbParameters::anyCharCodeSet;
  pd_tcs_w = orbParameters::anyWCharCodeSet;
}

cdrMemoryStream::~cdrMemoryStream()
{
  if (!pd_readonly_and_external_buffer && pd_bufp != pd_inline_buffer)
    delete [] (char*)pd_bufp;
}

void*
cdrMemoryStream::ptrToClass(int* cptr)
{
  if (cptr == &cdrMemoryStream::_classid) return (cdrMemoryStream*)this;
  if (cptr == &cdrStream      ::_classid) return (cdrStream*)      this;
  return 0;
}

int cdrMemoryStream::_classid;


void
cdrMemoryStream::put_octet_array(const CORBA::Octet* b, int size,
				omni::alignment_t align)
{
  (void) reserveOutputSpace(align,size);
  omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr,align);
  memcpy((void*)p1,b,size);
  pd_outb_mkr = (void*)(p1+size);
}

void
cdrMemoryStream::get_octet_array(CORBA::Octet* b,int size,
				 omni::alignment_t align)
{
  fetchInputData(align,size);
  omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_inb_mkr,align);
  memcpy(b,(void*)p1,size);
  pd_inb_mkr = (void*)(p1+size);
}

void
cdrMemoryStream::skipInput(_CORBA_ULong size)
{
  fetchInputData(omni::ALIGN_1,size);
  pd_inb_mkr = (void*)((omni::ptr_arith_t)pd_inb_mkr + size);
}

CORBA::Boolean
cdrMemoryStream::checkInputOverrun(CORBA::ULong itemSize, 
				   CORBA::ULong nItems,
				   omni::alignment_t align)
{
  if (!pd_readonly_and_external_buffer) pd_inb_end = pd_outb_mkr;
  omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_inb_mkr,align);
  p1 += itemSize*nItems;
  return ((void*)p1 > pd_inb_end) ? 0 : 1;
}

CORBA::Boolean
cdrMemoryStream::checkOutputOverrun(CORBA::ULong,CORBA::ULong,
				    omni::alignment_t)
{
  return (pd_readonly_and_external_buffer) ? 0 : 1;
}

void
cdrMemoryStream::fetchInputData(omni::alignment_t align, size_t required)
{
  if (!pd_readonly_and_external_buffer) pd_inb_end = pd_outb_mkr;
  required += omni::align_to((omni::ptr_arith_t)pd_inb_mkr,align) -
              (omni::ptr_arith_t)pd_inb_mkr;

  size_t avail = (char*)pd_inb_end - (char*) pd_inb_mkr;
  if (avail < required)
    OMNIORB_THROW(MARSHAL,MARSHAL_PassEndOfMessage,
		  (CORBA::CompletionStatus)completion());
}

CORBA::Boolean
cdrMemoryStream::reserveOutputSpaceForPrimitiveType(omni::alignment_t align,
						    size_t required)
{
  return reserveOutputSpace(align,required);
}


CORBA::Boolean
cdrMemoryStream::maybeReserveOutputSpace(omni::alignment_t align,
					 size_t required)
{
  return reserveOutputSpace(align,required);
}

CORBA::Boolean
cdrMemoryStream::reserveOutputSpace(omni::alignment_t align,size_t required)
{
  if (pd_readonly_and_external_buffer)
    OMNIORB_THROW(MARSHAL,MARSHAL_AttemptToWriteToReadOnlyBuf,
		  (CORBA::CompletionStatus)completion());


  required += omni::align_to((omni::ptr_arith_t)pd_outb_mkr,align) -
              (omni::ptr_arith_t)pd_outb_mkr;

  size_t newsize = (char*)pd_outb_end - (char*)pd_outb_mkr;
  if (newsize > required)
    return 1;

  // Reach here only if we really need to expand the buffer
  size_t datasize = ((omni::ptr_arith_t)pd_outb_mkr -
		     (omni::ptr_arith_t)pd_bufp_8);

  // Grow the buffer exponentially, but not too fast
  newsize = datasize + required + (size_t) omni::ALIGN_8;

  if (newsize < 1024)
    newsize += datasize;
  else
    newsize += datasize / 2;

  void* oldbufp   = pd_bufp;
  void* oldbufp_8 = pd_bufp_8;

  pd_bufp   = new char[newsize];
  pd_bufp_8 = ensure_align_8(pd_bufp);

  if (pd_clear_memory) memset(pd_bufp,0,newsize);

  if (datasize)
    memcpy(pd_bufp_8,oldbufp_8,datasize);

  pd_outb_end = (void*)((omni::ptr_arith_t)pd_bufp + newsize);
  pd_outb_mkr = (void*)((omni::ptr_arith_t)pd_bufp_8 +
			((omni::ptr_arith_t)pd_outb_mkr -
			 (omni::ptr_arith_t)oldbufp_8));
  pd_inb_mkr  = (void*)((omni::ptr_arith_t)pd_bufp_8 +
			((omni::ptr_arith_t)pd_inb_mkr -
			 (omni::ptr_arith_t)oldbufp_8));
  pd_inb_end  = (void*)((omni::ptr_arith_t)pd_bufp_8 +
			((omni::ptr_arith_t)pd_inb_end -
			 (omni::ptr_arith_t)oldbufp_8));

  if (oldbufp != pd_inline_buffer)
    delete [] (char*) oldbufp;
  return 1;
}

void
cdrMemoryStream::copy_to(cdrStream& s, int size, omni::alignment_t align) {

  fetchInputData(align,size);
  omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_inb_mkr,align);
  s.put_octet_array((const CORBA::Octet*)p1,size,align);
  pd_inb_mkr = (void*)(p1+size);
}


void
cdrMemoryStream::setByteSwapFlag(CORBA::Boolean littleendian)
{
  pd_marshal_byte_swap = pd_unmarshal_byte_swap = (littleendian == ((CORBA::Boolean)omni::myByteOrder)) ? 0 : 1; 
}

CORBA::ULong
cdrMemoryStream::currentInputPtr() const
{
  return ((omni::ptr_arith_t)pd_inb_mkr - (omni::ptr_arith_t)pd_bufp_8);
}

CORBA::ULong
cdrMemoryStream::currentOutputPtr() const
{
  return ((omni::ptr_arith_t)pd_outb_mkr - (omni::ptr_arith_t)pd_bufp_8);
}

cdrMemoryStream::cdrMemoryStream(void* databuffer)
{
  pd_tcs_c = orbParameters::anyCharCodeSet;
  pd_tcs_w = orbParameters::anyWCharCodeSet;

  pd_readonly_and_external_buffer = 1;
  pd_clear_memory = 0;
  pd_bufp = databuffer;
  pd_bufp_8 = databuffer;

#if (SIZEOF_LONG == SIZEOF_PTR)
  pd_inb_end = (void *) ULONG_MAX;
#elif (SIZEOF_INT == SIZEOF_PTR)
  pd_inb_end = (void *) UINT_MAX;
#elif defined (_WIN64)
  pd_inb_end = (void *) _UI64_MAX;
#else
#error "No suitable integer type available to calculate maximum" \
  " pointer value from"
#endif
  rewindPtrs();
}

cdrMemoryStream::cdrMemoryStream(void* databuffer, size_t maxLen)
{
  pd_tcs_c = orbParameters::anyCharCodeSet;
  pd_tcs_w = orbParameters::anyWCharCodeSet;

  pd_readonly_and_external_buffer = 1;
  pd_clear_memory = 0;
  pd_bufp   = databuffer;
  pd_bufp_8 = databuffer;
  pd_inb_end = (void*)((omni::ptr_arith_t)pd_bufp + maxLen);
  rewindPtrs();
}

cdrMemoryStream::cdrMemoryStream(const cdrMemoryStream& s,
				 _CORBA_Boolean read_only)
{
  pd_tcs_c = s.pd_tcs_c;
  pd_tcs_w = s.pd_tcs_w;

  pd_readonly_and_external_buffer = (read_only ||
				     s.pd_readonly_and_external_buffer);
  pd_clear_memory = 0;

  pd_marshal_byte_swap = pd_unmarshal_byte_swap = s.pd_marshal_byte_swap;

  if (s.pd_readonly_and_external_buffer) {
    // For an external buffer the storage is managed elsewhere. We
    // assume that it will continue to exist for the lifetime of this
    // buffered stream, and just use the buffer directly.
    pd_bufp    = s.pd_bufp;
    pd_bufp_8  = s.pd_bufp;
    pd_inb_end = s.pd_inb_end;
    rewindPtrs();
  }
  else if (read_only) {
    // Original MemoryStream is read/write.  We assume that the
    // original stream will exist at least as long as us, intact, so
    // that we can safely refer to its buffer directly.
    pd_bufp   = s.bufPtr();
    pd_bufp_8 = pd_bufp;
    pd_inb_end = (void*)((omni::ptr_arith_t)pd_bufp + s.bufSize());
    rewindPtrs();
  }
  else {
    // Copy the contents of the original stream
    pd_bufp     = pd_inline_buffer;
    pd_bufp_8   = ensure_align_8(pd_inline_buffer);
    pd_outb_end = (pd_inline_buffer + sizeof(pd_inline_buffer));
    rewindPtrs();
    if (s.bufSize()) {
      reserveOutputSpace(omni::ALIGN_8,s.bufSize());
      memcpy(pd_outb_mkr,s.bufPtr(),s.bufSize());
      pd_outb_mkr = (void*)((omni::ptr_arith_t)pd_outb_mkr + s.bufSize());
    }
  }
}

cdrMemoryStream& 
cdrMemoryStream::operator=(const cdrMemoryStream& s)
{
  pd_tcs_c = s.pd_tcs_c;
  pd_tcs_w = s.pd_tcs_w;

  pd_marshal_byte_swap = pd_unmarshal_byte_swap = s.pd_marshal_byte_swap;

  if (!s.pd_readonly_and_external_buffer) {
    if (pd_readonly_and_external_buffer) {
      pd_readonly_and_external_buffer = 0;
      pd_bufp     = pd_inline_buffer;
      pd_bufp_8   = ensure_align_8(pd_inline_buffer);
      pd_outb_end = (pd_inline_buffer + sizeof(pd_inline_buffer));
    }
    rewindPtrs();
    if (s.bufSize()) {
      reserveOutputSpace(omni::ALIGN_8,s.bufSize());
      memcpy(pd_outb_mkr,s.bufPtr(),s.bufSize());
      pd_outb_mkr = (void*)((omni::ptr_arith_t)pd_outb_mkr + s.bufSize());
    }
  }
  else {
    // For an external buffer the storage is managed elsewhere. We assume
    // that it will continue to exist for the lifetime of this buffered
    // stream also - so just copy the pointer.
    
    if (!pd_readonly_and_external_buffer) {
      pd_readonly_and_external_buffer = 1;
      if (pd_bufp != pd_inline_buffer) {
	delete [] (char*)pd_bufp;
      }
    }
    pd_bufp     = s.pd_bufp;
    pd_bufp_8   = s.pd_bufp;
    pd_inb_end  = s.pd_inb_end;
    rewindPtrs();
  }
  return *this;
}


cdrEncapsulationStream::cdrEncapsulationStream(CORBA::ULong initialBufsize,
					       CORBA::Boolean clearMemory)
  : cdrMemoryStream(initialBufsize,clearMemory)
{
  marshalOctet(omni::myByteOrder);
}


cdrEncapsulationStream::cdrEncapsulationStream(const CORBA::Octet* databuffer,
					       CORBA::ULong bufsize,
					       CORBA::Boolean useAlign4) 
  : cdrMemoryStream((void*)databuffer, bufsize)
{
  if (bufsize < 1) 
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidInitialSize,
		  (CORBA::CompletionStatus)completion());

  // We have to check the alignment of start of the the octet sequence buffer.
  // It should be <initialAlign>. This is normally the case but
  // is dependent on the implementation of the new operator. The following
  // deal with both cases.
  omni::alignment_t initialAlign = (useAlign4) ? omni::ALIGN_4 : omni::ALIGN_8;

  if ((omni::ptr_arith_t)databuffer != 
      omni::align_to((omni::ptr_arith_t)databuffer,initialAlign)) 
    {
      // This is the rare case. The sequence buffer does not start with
      // initialAlign. Create a local copy.
      pd_readonly_and_external_buffer = 0;
      pd_clear_memory = 0;
      pd_bufp     = pd_inline_buffer;
      pd_bufp_8   = ensure_align_8(pd_inline_buffer);
      pd_outb_end = (pd_inline_buffer + sizeof(pd_inline_buffer));
      rewindPtrs();
      put_octet_array((const CORBA::Char*)databuffer,bufsize);
    }

  {
    CORBA::Boolean endian = unmarshalBoolean();
    setByteSwapFlag(endian);
  }
}

cdrEncapsulationStream::
cdrEncapsulationStream(const _CORBA_Unbounded_Sequence_Octet& seq,
                       _CORBA_Boolean useAlign4)
  : cdrMemoryStream((void*)seq.get_buffer(), seq.length())
{
  if (seq.length() < 1)
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidInitialSize,
		  (CORBA::CompletionStatus)completion());

  // We have to check the alignment of start of the the octet sequence buffer.
  // It should be <initialAlign>. This is normally the case but
  // is dependent on the implementation of the new operator. The following
  // deal with both cases.
  omni::alignment_t initialAlign = (useAlign4) ? omni::ALIGN_4 : omni::ALIGN_8;

  if ((omni::ptr_arith_t)pd_bufp != 
      omni::align_to((omni::ptr_arith_t)pd_bufp, initialAlign)) 
    {
      // This is the rare case. The sequence buffer does not start with
      // initialAlign. Create a local copy.
      pd_readonly_and_external_buffer = 0;
      pd_clear_memory = 0;
      pd_bufp     = pd_inline_buffer;
      pd_bufp_8   = ensure_align_8(pd_inline_buffer);
      pd_outb_end = (pd_inline_buffer + sizeof(pd_inline_buffer));
      rewindPtrs();
      put_octet_array((const CORBA::Char*)seq.get_buffer(), seq.length());
    }

  {
    CORBA::Boolean endian = unmarshalBoolean();
    setByteSwapFlag(endian);
  }
}


cdrEncapsulationStream::cdrEncapsulationStream(cdrStream& s,
					       CORBA::ULong fetchsize)
  : cdrMemoryStream(fetchsize)
{
  pd_tcs_c = s.TCS_C();
  pd_tcs_w = s.TCS_W();

  s.get_octet_array((CORBA::Octet*)pd_outb_mkr,(int)fetchsize);
  pd_outb_mkr = (void*)((omni::ptr_arith_t)pd_outb_mkr + fetchsize);
  rewindInputPtr();
  {
    CORBA::Boolean endian = unmarshalBoolean();
    setByteSwapFlag(endian);
  }
}

void*
cdrEncapsulationStream::ptrToClass(int* cptr)
{
  if (cptr == &cdrEncapsulationStream::_classid)
    return (cdrEncapsulationStream*)this;

  if (cptr == &cdrMemoryStream::_classid)
    return (cdrMemoryStream*)this;

  if (cptr == &cdrStream::_classid)
    return (cdrStream*)this;

  return 0;
}

int cdrEncapsulationStream::_classid;

void
cdrEncapsulationStream::getOctetStream(CORBA::Octet*& databuffer,
				       CORBA::ULong& max,
				       CORBA::ULong& len)
{
  if (pd_readonly_and_external_buffer) 
    OMNIORB_THROW(MARSHAL,MARSHAL_AttemptToWriteToReadOnlyBuf,
		  (CORBA::CompletionStatus)completion());


  void* begin = pd_bufp_8;
  
  max = ((omni::ptr_arith_t) pd_outb_end - (omni::ptr_arith_t) begin);
  len = ((omni::ptr_arith_t) pd_outb_mkr - (omni::ptr_arith_t) begin);
  if (begin == pd_bufp && pd_bufp != pd_inline_buffer) {
    databuffer = (CORBA::Octet*) pd_bufp;
    pd_readonly_and_external_buffer = 1;
  }
  else {
    databuffer = new CORBA::Octet[max];
    memcpy((void*)databuffer,(void*)begin,len);
  }
}

void
cdrEncapsulationStream::setOctetSeq(_CORBA_Unbounded_Sequence_Octet& seq)
{
  CORBA::Octet* databuffer;
  CORBA::ULong  max, len;
  getOctetStream(databuffer, max, len);

  seq.replace(max, len, databuffer, 1);
}


/////////////////////////////////////////////////////////////////////////////
void*
cdrCountingStream::ptrToClass(int* cptr)
{
  if (cptr == &cdrCountingStream::_classid) return (cdrCountingStream*)this;
  if (cptr == &cdrStream        ::_classid) return (cdrStream*)        this;
  return 0;
}

int cdrCountingStream::_classid;


void
cdrCountingStream::put_octet_array(const CORBA::Octet* b, int size,
				  omni::alignment_t align)
{
  omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_total,align);
  pd_total = p1 + size;
}

CORBA::Boolean
cdrCountingStream::maybeReserveOutputSpace(omni::alignment_t align,
					   size_t required)
{
  omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_total,align);
  pd_total = p1 + required;
  return 0;
}

CORBA::Boolean
cdrCountingStream::reserveOutputSpaceForPrimitiveType(omni::alignment_t align,
						      size_t required)
{
  return maybeReserveOutputSpace(align,required);
}

CORBA::Boolean
cdrCountingStream::checkOutputOverrun(CORBA::ULong,
				      CORBA::ULong,
				      omni::alignment_t)
{
  return 0;
}

void
cdrCountingStream::copy_to(cdrStream&, int, omni::alignment_t) {
}

void
cdrCountingStream::get_octet_array(CORBA::Octet*,int,omni::alignment_t)
{
}

void
cdrCountingStream::skipInput(CORBA::ULong) 
{
}

CORBA::Boolean
cdrCountingStream::checkInputOverrun(CORBA::ULong,CORBA::ULong,
				     omni::alignment_t)
{
  return 0;
}

void
cdrCountingStream::fetchInputData(omni::alignment_t,size_t) 
{
}

CORBA::ULong
cdrCountingStream::currentInputPtr() const 
{
  return (CORBA::ULong) pd_total;
}

CORBA::ULong
cdrCountingStream::currentOutputPtr() const 
{ 
  return 0;
}
