// -*- Mode: C++; -*-
//                            Package   : omniORB
// callHandle.cc              Created on: 16/05/2001
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2003-2012 Apasphere Ltd
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//
//   Call handle used during remote or in-process operation dispatch.

#include <omniORB4/CORBA.h>
#include <omniORB4/omniORB.h>
#include <omniORB4/callHandle.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/omniServant.h>
#include <omniORB4/IOP_C.h>
#include <poacurrentimpl.h>
#include <invoker.h>
#include <giopStream.h>
#include <giopStrand.h>
#include <giopRope.h>
#include <omniORB4/giopEndpoint.h>

OMNI_USING_NAMESPACE(omni)


static void
dealWithUserException(cdrMemoryStream& stream,
		      omniCallDescriptor* desc,
		      CORBA::UserException& ex);


#ifdef HAS_Cplusplus_Namespace
namespace {
#endif
  class PostInvoker {
  public:
    inline PostInvoker(omniCallHandle::PostInvokeHook* hook)
      : pd_hook(hook) {}
    inline ~PostInvoker() {
      if (pd_hook)
	pd_hook->postinvoke();
    }
  private:
    omniCallHandle::PostInvokeHook* pd_hook;
  };

  class MainThreadTask : public omniTask {
  public:
    inline MainThreadTask(omniServant* servant, omniCallDescriptor& desc,
			  omni_tracedmutex* mu, omni_tracedcondition* cond)
      : omniTask(omniTask::DedicatedThread),
	pd_servant(servant),
	pd_desc(desc),
	pd_mu(mu),
	pd_cond(cond),
	pd_except(0),
	pd_done(0)
    {
      if (omniORB::trace(25)) {
	omniORB::logger l;
	l << "Preparing to dispatch '" << desc.op() << "' to main thread\n";
      }
    }

    void execute();
    // Called by the async invoker. Performs the upcall. If an
    // exception occurs, places a copy in pd_except.

    void wait();
    // Wait for execute() to finish. Throws the exception in pd_except
    // if there is one.
    
  private:
    omniServant*          pd_servant;
    omniCallDescriptor&   pd_desc;
    omni_tracedmutex*     pd_mu;
    omni_tracedcondition* pd_cond;
    CORBA::Exception*     pd_except;
    int                   pd_done;
  };

#ifdef HAS_Cplusplus_Namespace
}
#endif


void
omniCallHandle::upcall(omniServant* servant, omniCallDescriptor& desc)
{
  OMNIORB_ASSERT(pd_localId);
  desc.poa(pd_poa);
  desc.localId(pd_localId);

  _OMNI_NS(poaCurrentStackInsert) insert(&desc, pd_self_thread);

  if (pd_iop_s) { // Remote call
    pd_iop_s->ReceiveRequest(desc);
    {
      PostInvoker postinvoker(pd_postinvoke_hook);

      if (!pd_mainthread_mu) {
	desc.doLocalCall(servant);
      }
      else {
	// Main thread dispatch
	MainThreadTask mtt(servant, desc,
			   pd_mainthread_mu, pd_mainthread_cond);
	int i = _OMNI_NS(orbAsyncInvoker)->insert(&mtt); OMNIORB_ASSERT(i);
	mtt.wait();
      }
    }
    pd_iop_s->SendReply();
  }
  else { // In process call

    if (pd_call_desc == &desc) {
      // Fast case -- call descriptor can invoke directly on the servant
      PostInvoker postinvoker(pd_postinvoke_hook);

      if (!pd_mainthread_mu) {
	desc.doLocalCall(servant);
      }
      else {
	// Main thread dispatch
	MainThreadTask mtt(servant, desc,
			   pd_mainthread_mu, pd_mainthread_cond);
	int i = _OMNI_NS(orbAsyncInvoker)->insert(&mtt); OMNIORB_ASSERT(i);
	mtt.wait();
      }
    }
    else {
      // Cannot call directly -- use a memory stream
      if (omniORB::traceInvocations) {
	omniORB::logger l;
	l << "In process indirect call '" << desc.op() << "'\n";
      }
      cdrMemoryStream stream;
      pd_call_desc->initialiseCall(stream);
      pd_call_desc->marshalArguments(stream);
      stream.clearValueTracker();

      if (omniORB::trace(30)) {
        omniORB::logs(30, "Indirect call buffer:");
        _OMNI_NS(giopStream)::dumpbuf((unsigned char*)stream.bufPtr(),
                                      stream.bufSize());
      }
      
      desc.unmarshalArguments(stream);
      stream.clearValueTracker();

      try {
	PostInvoker postinvoker(pd_postinvoke_hook);

	if (!pd_mainthread_mu) {
	  desc.doLocalCall(servant);
	}
	else {
	  // Main thread dispatch
	  MainThreadTask mtt(servant, desc,
			     pd_mainthread_mu, pd_mainthread_cond);
	  int i = _OMNI_NS(orbAsyncInvoker)->insert(&mtt); OMNIORB_ASSERT(i);
	  mtt.wait();
	}
	stream.rewindPtrs();

	desc.marshalReturnedValues(stream);
	stream.clearValueTracker();

	pd_call_desc->unmarshalReturnedValues(stream);
      }
#ifdef HAS_Cplusplus_catch_exception_by_base
      catch (CORBA::UserException& ex) {
	stream.rewindPtrs();
	stream.clearValueTracker();
	dealWithUserException(stream, pd_call_desc, ex);
      }
#else
      catch (omniORB::StubUserException& uex) {
	try {
	  CORBA::UserException& ex = *((CORBA::UserException*)uex.ex());
	  stream.rewindPtrs();
	  stream.clearValueTracker();
	  dealWithUserException(stream, pd_call_desc, ex);
	}
	catch (...) {
	  delete uex.ex();  // ?? Possible memory leak?
	  throw;
	}
	delete uex.ex();
      }
#endif
    }
  }
}


static void
dealWithUserException(cdrMemoryStream& stream,
		      omniCallDescriptor* desc,
		      CORBA::UserException& ex)
{
  int size;
  const char* repoId = ex._NP_repoId(&size);

  if (omniORB::trace(25)) {
    omniORB::logger l;
    l << "Handling in-process user exception '" << repoId << "'\n";
  }

  ex._NP_marshal(stream);
  stream.clearValueTracker();
  desc->userException(stream, 0, repoId);
}

void 
omniCallHandle::SkipRequestBody()
{
  if (pd_iop_s)
    pd_iop_s->SkipRequestBody();
}


giopConnection*
omniCallHandle::connection()
{
  if (pd_iop_s) {
    cdrStream&  stream  = pd_iop_s->getStream();
    giopStream* gstream = giopStream::downcast(&stream);
    if (gstream)
      return gstream->strand().connection;
  }
  return 0;
}


const char*
omniCallHandle::myaddress()
{
  giopConnection* conn = connection();
  return conn ? conn->myaddress() : 0;
}

const char*
omniCallHandle::peeraddress()
{
  giopConnection* conn = connection();
  return conn ? conn->peeraddress() : 0;
}

const char*
omniCallHandle::peeridentity()
{
  giopConnection* conn = connection();
  return conn ? conn->peeridentity() : 0;
}

void*
omniCallHandle::peerdetails()
{
  giopConnection* conn = connection();
  return conn ? conn->peerdetails() : 0;
}


omniCallHandle::PostInvokeHook::~PostInvokeHook() {}


void
MainThreadTask::execute()
{
  if (omniORB::traceInvocations) {
    omniORB::logger l;
    l << "Main thread dispatch '" << pd_desc.op() << "'\n";
  }

  try {
    _OMNI_NS(poaCurrentStackInsert) insert(&pd_desc);
    pd_desc.doLocalCall(pd_servant);
  }
#ifdef HAS_Cplusplus_catch_exception_by_base
  catch (CORBA::Exception& ex) {
    pd_except = CORBA::Exception::_duplicate(&ex);
  }
#else
#  define DUPLICATE_AND_STORE(name) \
  catch (CORBA::name& ex) { \
    pd_except = CORBA::Exception::_duplicate(&ex); \
  }

  OMNIORB_FOR_EACH_SYS_EXCEPTION(DUPLICATE_AND_STORE)
#  undef DUPLICATE_AND_STORE

  catch (omniORB::StubUserException& uex) {
    pd_except = CORBA::Exception::_duplicate(uex.ex());
  }
#endif
  catch (...) {
    CORBA::UNKNOWN ex;
    pd_except = CORBA::Exception::_duplicate(&ex);
  }

  {
    // Wake up the dispatch thread
    omni_tracedmutex_lock l(*pd_mu);
    pd_done = 1;
    pd_cond->broadcast();
  }
}

void
MainThreadTask::wait()
{
  {
    omni_tracedmutex_lock l(*pd_mu);
    while (!pd_done)
      pd_cond->wait();
  }
  if (pd_except) {
    // This interesting construction contrives to ask the
    // heap-allocated exception to throw a copy of itself, then
    // deletes it.
    try {
      pd_except->_raise();
    }
    catch (...) {
      delete pd_except;
      throw;
    }
  }
}
