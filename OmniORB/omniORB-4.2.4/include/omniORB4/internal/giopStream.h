// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopStream.h                   Created on: 05/01/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2012 Apasphere Ltd
//    Copyright (C) 2001      AT&T Laboratories Cambridge
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

#ifndef __GIOPSTREAM_H__
#define __GIOPSTREAM_H__

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

OMNI_NAMESPACE_BEGIN(omni)

class giopStrand;
class giopStreamImpl;
class giopImpl10;
class giopImpl11;
class giopImpl12;

struct giopStream_Buffer {
  CORBA::ULong             start;   // offset to the beginning of data
  CORBA::ULong             end;     // offset to the end of buffer
  CORBA::ULong             last;    // offset to the last data byte
  CORBA::ULong             size;    // GIOP message size.
  giopStream_Buffer*       next;    // next Buffer in a chain
  // buffer data follows.

  void alignStart(omni::alignment_t);
  static void deleteBuffer(giopStream_Buffer*);
  static giopStream_Buffer* newBuffer(CORBA::ULong sz=0);

  inline omni::ptr_arith_t bufStart()
  {
    return (omni::ptr_arith_t)this + start;
  }

  inline omni::ptr_arith_t bufLast()
  {
    return (omni::ptr_arith_t)this + last;
  }

  inline omni::ptr_arith_t bufEnd()
  {
    return (omni::ptr_arith_t)this + end;
  }

  inline void setLast(void* mkr)
  {
    last = (CORBA::ULong)((omni::ptr_arith_t)mkr - (omni::ptr_arith_t)this);
  }

  inline CORBA::ULong dataSize()
  {
    return last - start;
  }
};

class giopStream;


//
// ZIOP support

class giopCompressor {
public:
  virtual ~giopCompressor() = 0;

  virtual giopStream_Buffer* compressBuffer(giopStream*        stream,
                                            giopStream_Buffer* buf) = 0;
  // Compress buf, which must contain a complete GIOP message.
  // Consumes buf, or returns it unchanged if the data was not
  // compressible.

  virtual giopStream_Buffer* decompressBuffer(giopStream*        stream,
                                              giopStream_Buffer* buf) = 0;
  // Decompress buf, reading more data from the stream if need be.
  // Consumes buf.
};

class giopCompressorFactory {
public:
  virtual ~giopCompressorFactory() = 0;
  virtual giopCompressor* newCompressor() = 0;
};


//
// GIOP stream

class giopStream : public cdrStream {
public:

  giopStream(giopStrand*);
  // No thread safety precondition

  ~giopStream();
  // No thread safety precondition

  virtual void* ptrToClass(int* cptr);
  static inline giopStream* downcast(cdrStream* s) {
    return (giopStream*)s->ptrToClass(&_classid);
  }
  static _core_attr int _classid;

  void reset();

  GIOP::Version version();
  // No thread safety precondition

  inline giopStrand& strand() { return *pd_strand; }
  // No thread safety precondition

  inline giopStreamImpl* impl() const { return pd_impl; }
  // No thread safety precondition

  inline void impl(giopStreamImpl* impl) { pd_impl = impl; }
  // No thread safety precondition

  inline const omni_time_t& getDeadline() const {
    return pd_deadline;
  }
  // No thread safety precondition

  inline void setDeadline(const omni_time_t& deadline) {
    pd_deadline = deadline;
  }
  // No thread safety precondition

  inline void clearDeadline() {
    pd_deadline.assign(0,0);
  }
  // No thread safety precondition
  
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  // Thread Safety preconditions:
  //   Caller of these strand locking functions must hold the
  //   omniTransportLock before calling.

  virtual void rdLock();
  // Acquire read lock on the strand.
  //
  // The function honours the deadline set on the the object. If the deadline
  // is reached, the function should give up waiting.
  //
  // Error conditions. If the deadline has expired, this call raises the
  // CommFailure exception.

  virtual void wrLock();
  // Acquire write lock on the strand.
  //
  // The function honours the deadline set on the the object. If the deadline
  // is reached, the function should give up waiting.
  //
  // Error conditions. If the deadline has expired, this call raises the
  // CommFailure exception.

  virtual void rdUnLock();// Release the read lock if this instance holds one
  virtual void wrUnLock();// Release the write lock if this instance holds one.

  virtual _CORBA_Boolean rdLockNonBlocking();
  // Acquire read lock but do not block if another thread is already
  // holding one. Return True if read lock is acquired.

  virtual void sleepOnRdLock();
  // Block until the read lock is available.
  //
  // The function honours the deadline set on the the object. If the deadline
  // is reached, the function should give up waiting.
  //
  // Error conditions. If the deadline has expired, this call raises the
  // CommFailure exception.

  virtual void sleepOnRdLockAlways();
  // Irrespective of the state of the read lock. Block the thread
  // on the read lock condition variable until it is signalled by another
  // thread or the deadline in real time has expired.
  //
  // The function honours the deadline set on the the object. If the deadline
  // is reached, the function should give up waiting.
  //
  // Error conditions. If the deadline has expired, this call raises the
  // CommFailure exception.


  static void wakeUpRdLock(giopStrand*);
  // Wakeup all those threads blocking to acquire a read lock.

  static _CORBA_Boolean noLockWaiting(giopStrand*);
  // Returns True (1) if no thread is waiting to acquire a read or write lock

  static _CORBA_Boolean rdLockNonBlocking(giopStrand*);
  // Acquire read lock but do not block if another thread is already
  // holding one. Return True if read lock is acquired. A variant of
  // the member func with the same name except that the caller do not
  // have a giopStream at hand.

  static void sleepOnRdLock(giopStrand*);
  // Block until the read lock is available. A variant of
  // the member func with the same name except that the caller do not
  // have a giopStream at hand.

  void markRdLock();
  // Caller must have called rdLockNonBlocking(giopStrand*) and succeeded.
  // Mark this instance as the one that have acquired the lock.

  static _CORBA_Boolean RdLockIsHeld(giopStrand*);
  // Return True (1) if the read lock is held by some giopStream.

  ////////////////////////////////////////////////////////////////////////
  virtual void notifyCommFailure(CORBA::Boolean heldlock,
				 CORBA::ULong& minor,
				 CORBA::Boolean& retry);
  // When the giopStream detects an error in sending or receiving data,
  // it raises the giopStream::CommFailure exception.
  // But before it does so, it calls this function to determine the 
  // the minor code of the exception. The function set the value.
  // Notice that this is a virtual function. The intention is for the
  // derived classes GIOP_S and GIOP_C to override this function in order
  // to intercept the callback. The override function can then look at its
  // internal state, e.g. the progress of a call, to decide what the
  // minor code should be.
  //
  // The function also set retry to TRUE(1) if the invocation should be
  // relaunched.
  //
  // Unless this function is overridden, the function returns minor = 0
  // retry = 0.
  //
  // Thread Safety preconditions:
  //    Internally, omniTransportLock is used for synchronisation, if
  //    <heldlock> is TRUE(1), the caller already hold the lock.

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  // CommFailure is the exception raised by giopStream when it details an
  // error in sending or receiving data in any of member functions derived
  // from cdrStream. (see below)
  class CommFailure {
  public:
    inline CommFailure(CORBA::ULong minor,
		       CORBA::CompletionStatus status,
		       CORBA::Boolean retry) :
      pd_minor(minor), pd_status(status), pd_retry(retry) {}

    // minor      - error no.
    // status     - completion status
    // retry      - TRUE(1) the invocation SHOULD BE relaunched
    //              FALSE(0) don't bother
    // filename   - source file
    // linenumber - line number where the exception is raised.
    // message    - reason for the exception
    // strand     - strand the exception occurred with

    inline ~CommFailure() {}

    inline CORBA::ULong minor() const { return pd_minor; }
    inline CORBA::CompletionStatus completed() const { return pd_status; }
    inline CORBA::Boolean retry() const { return pd_retry; }

    inline CommFailure(const CommFailure& e) :
      pd_minor(e.pd_minor), pd_status(e.pd_status), pd_retry(e.pd_retry) {}

    inline CommFailure& operator=(const CommFailure& e) {
      pd_minor = e.pd_minor;
      pd_status = e.pd_status;
      pd_retry = e.pd_retry;
      return *this;
    }

    static void _raise(CORBA::ULong            minor,
		       CORBA::CompletionStatus status,
		       CORBA::Boolean          retry,
		       const char*             filename,
		       CORBA::ULong            linenumber,
		       const char*             message,
		       giopStrand*             strand);

    static void _raise(CORBA::ULong            minor,
		       CORBA::CompletionStatus status,
		       CORBA::Boolean          retry,
		       const char*             filename,
		       CORBA::ULong            linenumber,
		       const char*             message,
		       const char*             endpoint);
    
  private:
    CORBA::ULong            pd_minor;
    CORBA::CompletionStatus pd_status;
    CORBA::Boolean          pd_retry;
  };
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  static void dumpbuf(unsigned char* buf, size_t sz);
  // Dump the buffer to the log. Logs just the first 128 bytes unless
  // omniORB::traceLevel >= 40.


protected:
  giopStrand*                pd_strand;
  CORBA::Boolean             pd_rdlocked;
  CORBA::Boolean             pd_wrlocked;
  giopStreamImpl*            pd_impl;
  omni_time_t                pd_deadline;

private:
  giopStream();
  giopStream(const giopStream&);
  giopStream& operator=(const giopStream&);

  operator giopStrand&();
  // Not implemented. Use the strand() member function instead.

public:
  // These tunable parameters are used to determine at what size an
  // octet array will be sent/received directly to/from the network
  // connection without staging through the internal buffer.
  static _core_attr CORBA::ULong directSendCutOff;
  static _core_attr CORBA::ULong directReceiveCutOff;

  // When performing a direct send, ensure that the previous chunk
  // contains at least this much data. Must be a multiple of 8.
  static _core_attr CORBA::ULong minChunkBeforeDirectSend;

  // Allocate this number of bytes for each giopStream_Buffer.
  static _core_attr CORBA::ULong bufferSize;

  static _core_attr giopCompressorFactory* compressorFactory;

public:
  // The following implement the abstract functions defined in cdrStream
  //
  // In addition to the CORBA SystemException, the member functions will
  // raise the giopStream::CommFailure instead of CORBA::COMM_FAILURE exception
  // to indicate an error in sending or receiving data. 

  void put_octet_array(const _CORBA_Octet* b, int size,
		       omni::alignment_t align=omni::ALIGN_1);

  void get_octet_array(_CORBA_Octet* b,int size,
		       omni::alignment_t align=omni::ALIGN_1);

  void skipInput(_CORBA_ULong);

  _CORBA_Boolean checkInputOverrun(_CORBA_ULong, _CORBA_ULong,
				   omni::alignment_t align=omni::ALIGN_1);

  _CORBA_Boolean checkOutputOverrun(_CORBA_ULong,_CORBA_ULong,
				    omni::alignment_t align=omni::ALIGN_1);

  void fetchInputData(omni::alignment_t,size_t);

  _CORBA_Boolean reserveOutputSpaceForPrimitiveType(omni::alignment_t,size_t);

  _CORBA_Boolean maybeReserveOutputSpace(omni::alignment_t,size_t);

  _CORBA_Boolean is_giopStream();

  _CORBA_ULong currentInputPtr() const;

  _CORBA_ULong currentOutputPtr() const;


  friend class giopImpl10;
  friend class giopImpl11;
  friend class giopImpl12;
  friend class giopCompressorImpl;

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  // Receiving GIOP messages: 
  // GIOP messages are read from the strand via the member function
  // inputMessage() and inputChunk(). Both functions return a
  // giopStream_Buffer. The buffer should be returned to the strand using
  // the releaseInputBuffer() function.

  giopStream_Buffer* newInputBuffer(CORBA::ULong sz=0);
  // Return a giopStream_Buffer. Should be deleted by calling its
  // deleteBuffer() method.

private:

  // The following variables must be initialised to 0 by reset().
  giopStream_Buffer*         pd_currentInputBuffer;
  giopStream_Buffer*         pd_input;
  CORBA::Boolean             pd_inputFullyBuffered;
  CORBA::Boolean             pd_inputMatchedId;

  // The following variables can be left uninitialised and will be
  // written with a sensible value when used.
  CORBA::Boolean             pd_inputExpectAnotherFragment;
  CORBA::ULong               pd_inputFragmentToCome;
  CORBA::ULong               pd_inputMessageSize;

  inline CORBA::Boolean inputFullyBuffered() const { 
    return pd_inputFullyBuffered;
  }

  inline void inputFullyBuffered(CORBA::Boolean b) {
    pd_inputFullyBuffered = b;
  }

  inline CORBA::Boolean inputExpectAnotherFragment() const {
    return pd_inputExpectAnotherFragment;
  }

  inline void inputExpectAnotherFragment(CORBA::Boolean yes) {
    pd_inputExpectAnotherFragment = yes;
  }

  inline CORBA::Boolean inputMatchedId() const {
    return pd_inputMatchedId;
  }

  inline void inputMatchedId(CORBA::Boolean yes) {
    pd_inputMatchedId = yes;
  }

  inline CORBA::ULong inputMessageSize() const {
    return pd_inputMessageSize;
  }

  inline void inputMessageSize(CORBA::ULong msgsz) {
    pd_inputMessageSize = msgsz;
  }

  inline CORBA::ULong inputFragmentToCome() const {
    return pd_inputFragmentToCome;
  }

  inline void inputFragmentToCome(CORBA::ULong fsz) {
    pd_inputFragmentToCome = fsz;
  }

  inline omni::ptr_arith_t inputBufferStart() {
    return pd_currentInputBuffer->bufStart();
  }

  inline omni::ptr_arith_t inputBufferLast() {
    return pd_currentInputBuffer->bufLast();
  }

  inline CORBA::ULong inputBufferMsgSize() {
    return pd_currentInputBuffer->size;
  }

  inline CORBA::ULong inputBufferDataSize() {
    return pd_currentInputBuffer->dataSize();
  }

  giopStream_Buffer* inputMessage();
  // Read the next GIOP message from the connection. This function does the
  // basic sanity check on the message header. 
  //
  // The function honours the deadline set on the the object. If the deadline
  // is reached, the function should give up waiting.
  //
  // Error conditions. If the data do not look like a GIOP message or
  // the connection reports an error in reading data or the deadline has
  // expired, this call raises the CommFailure exception. The strand
  // will be marked as DYING.
  // 
  // The size of the GIOP message is reported in <size>.
  // The returned buffer may or may not contain the complete message, i.e.
  // <last> - <start> may or may not equal to <size>. If it is the latter,
  // the caller should use inputChunk() to get the rest of the message.
  //
  // Thread Safety preconditions:
  //   Caller must have acquired the read lock on the strand.
  
  giopStream_Buffer* inputChunk(CORBA::ULong maxsize);
  // Same as inputMessage except that no sanity check is done on the
  // data received. The buffer returned contains up to maxsize bytes of
  // data. The <size> field is not applicable and is set to 0.
  //
  // Thread Safety preconditions:
  //   Caller must have acquired the read lock on the strand.

  void inputCopyChunk(void* dest,CORBA::ULong size);
  // Same as inputChunk except that data is copied directly into the
  // destination buffer.
  //
  // Thread Safety preconditions:
  //   Caller must have acquired the read lock on the strand.

  void releaseInputBuffer(giopStream_Buffer*);
  // Return the buffer given out by inputMessage and inputChunk.
  //
  // Thread Safety preconditions:
  //   None.

  void errorOnReceive(int,const char*,CORBA::ULong,giopStream_Buffer*,
		      CORBA::Boolean,const char*);
  // internal helper function, do not use outside this class

  CORBA::ULong ensureSaneHeader(const char*,CORBA::ULong,
				giopStream_Buffer*,CORBA::ULong);
  // internal helper function, do not use outside this class


  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  // Sending GIOP messages:

  // The following variables must be initialised to 0 at ctor.
  giopStream_Buffer*         pd_currentOutputBuffer;

  // The following variables can be left uninitialised and will be
  // written with a sensible value when used.
  CORBA::ULong               pd_outputFragmentSize;
  CORBA::ULong               pd_outputMessageSize;
  CORBA::ULong               pd_request_id;

  inline CORBA::ULong outputMessageSize() const {
    return pd_outputMessageSize;
  }

  inline void outputMessageSize(CORBA::ULong msgsz) {
    pd_outputMessageSize = msgsz;
  }

  inline CORBA::ULong outputFragmentSize() const {
    return pd_outputFragmentSize;
  }

  inline void outputFragmentSize(CORBA::ULong fsz) {
    pd_outputFragmentSize = fsz;
  }

  inline omni::ptr_arith_t outputBufferStart() {
    return pd_currentOutputBuffer->bufStart();
  }

  inline omni::ptr_arith_t outputBufferEnd() {
    return pd_currentOutputBuffer->bufEnd();
  }

  inline CORBA::ULong bufferedOutputSize() {
    return (CORBA::ULong)((omni::ptr_arith_t)pd_outb_mkr -
                          pd_currentOutputBuffer->bufStart());
  }

  inline void setOutputLastOffset() {
    pd_currentOutputBuffer->setLast(pd_outb_mkr);
  }


  // GIOP messages are sent via these member functions

  void sendChunk(giopStream_Buffer*);
  // Send the buffer to the strand.
  //
  // The function honours the deadline set on the the object. If the deadline
  // is reached, the function should give up waiting.
  //
  // Thread Safety preconditions:
  //   Caller must have acquired the write lock on the strand.

  void sendCopyChunk(void*, CORBA::ULong size);
  // Same as sendChunk() except that the data is copied directly from
  // the application buffer.
  //
  // Thread Safety preconditions:
  //   Caller must have acquired the write lock on the strand.

  void errorOnSend(int,const char*,CORBA::ULong,CORBA::Boolean,const char*);
  // internal helper function, do not use outside this class

public:
  giopActiveConnection* openConnection();
  // For a client-side stream, open the connection.

protected:
  //////////////////////////////////////////////////////////////////
  inline CORBA::ULong  requestId() const { return pd_request_id; }
  inline void requestId(CORBA::ULong v) { pd_request_id = v; }

};


OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __GIOPSTREAM_H__
