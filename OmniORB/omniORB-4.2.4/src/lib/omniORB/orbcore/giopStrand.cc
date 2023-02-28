// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopStrand.cc              Created on: 16/01/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2013 Apasphere Ltd
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

#include <omniORB4/CORBA.h>
#include <giopStream.h>
#include <giopStreamImpl.h>
#include <giopRope.h>
#include <giopStrand.h>
#include <giopStrandFlags.h>
#include <GIOP_S.h>
#include <GIOP_C.h>
#include <initialiser.h>
#include <invoker.h>
#include <orbOptions.h>
#include <orbParameters.h>

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////
CORBA::ULong orbParameters::scanGranularity = 5;
//  The granularity at which the ORB scan for idle connections.
//  This value determines the minimum value that inConScanPeriod or
//  outConScanPeriod can be implemented.
//
//  Valid values = (n >= 0 in seconds) 
//                  0 --> do not scan for idle connections.

CORBA::ULong orbParameters::outConScanPeriod = 120;
//  Idle connections shutdown. The ORB periodically scans all the
//  outgoing connections to detect if they are idle.
//  If no operation has passed through a connection for a scan period,
//  the ORB would treat this connection idle and shut it down.
//
//  Valid values = (n >= 0 in seconds) 
//                  0 --> do not close idle connections.

CORBA::ULong orbParameters::inConScanPeriod = 180;
//  Idle connections shutdown. The ORB periodically scans all the
//  incoming connections to detect if they are idle.
//  If no operation has passed through a connection for a scan period,
//  the ORB would treat this connection idle and shut it down.
//
//   Valid values = (n >= 0 in seconds) 
//                   0 --> do not close idle connections.


////////////////////////////////////////////////////////////////////////
class Scavenger : public omniTask {
public:
  Scavenger() : omniTask(omniTask::ImmediateDispatch) {}
  ~Scavenger() {}

  void execute();

  static void notify();

  static void terminate();

  static void initialise();

private:
  static CORBA::Boolean          shutdown;
  static omni_tracedmutex*       mutex;
  static omni_tracedcondition*	 cond;
  static Scavenger*              theTask;

  void removeIdle(StrandList& src,StrandList& dest, CORBA::Boolean client);
};

////////////////////////////////////////////////////////////////////////
static inline void
sendCloseConnection(giopStrand* s)
{
  // Send close connection message.
  char hdr[12];
  hdr[0] = 'G'; hdr[1] = 'I'; hdr[2] = 'O'; hdr[3] = 'P';
  hdr[4] = s->version.major;   hdr[5] = s->version.minor;
  hdr[6] = _OMNIORB_HOST_BYTE_ORDER_;
  hdr[7] = (char)GIOP::CloseConnection;
  hdr[8] = hdr[9] = hdr[10] = hdr[11] = 0;

  if (omniORB::trace(25)) {
    omniORB::logger log;
    log << "sendCloseConnection: to " << s->connection->peeraddress()
	<< " 12 bytes\n";
  }
  if (omniORB::trace(30))
    giopStream::dumpbuf((unsigned char*)hdr, 12);

  omni_time_t timeout(orbParameters::scanGranularity);
  omni_time_t deadline;

  if (timeout.s < 5)
    timeout.s = 5;

  omni_thread::get_time(deadline, timeout);

  int tx = s->connection->Send(hdr, 12, deadline);
  if (tx <= 0 && omniORB::trace(25)) {
    omniORB::logger log;
    const char* err = (tx == 0 ? "Timed out" : "Error");

    log << err << " sending CloseConnection to "
	<< s->connection->peeraddress() << "\n";
  }
}


////////////////////////////////////////////////////////////////////////
giopStrand::giopStrand(const giopAddress* addr) :
  pd_safelyDeleted(0),
  idlebeats(-1),
  address(addr), connection(0), server(0), flags(0),
  gatekeeper_checked(0), first_use(1), first_call(1),
  orderly_closed(0), biDir_initiated(0), biDir_has_callbacks(0),
  tcs_selected(0), tcs_c(0), tcs_w(0), giopImpl(0), compressor(0),
  rdcond(omniTransportLock, "giopStrand::rdcond"),
  rd_nwaiting(0), rd_n_justwaiting(0),
  wrcond(omniTransportLock, "giopStrand::wrcond"),
  wr_nwaiting(0), seqNumber(0), head(0), spare(0), pd_state(ACTIVE)
{
  version.major = version.minor = 0;
  Scavenger::notify();
  // Call scavenger::notify() to cause the scavenger thread to be
  // created if it hasn't been created already.
}

////////////////////////////////////////////////////////////////////////
giopStrand::giopStrand(giopConnection* conn, giopServer* serv) :
  pd_safelyDeleted(0),
  idlebeats(-1),
  address(0), connection(conn), server(serv), flags(0),
  gatekeeper_checked(0), first_use(0), first_call(0),
  orderly_closed(0), biDir_initiated(0), biDir_has_callbacks(0),
  tcs_selected(0), tcs_c(0), tcs_w(0), giopImpl(0), compressor(0),
  rdcond(omniTransportLock, "giopStrand::rdcond"),
  rd_nwaiting(0), rd_n_justwaiting(0),
  wrcond(omniTransportLock, "giopStrand::wrcond"),
  wr_nwaiting(0), seqNumber(1), head(0), spare(0), pd_state(ACTIVE)
{
  version.major = version.minor = 0;
  Scavenger::notify();
  // Call scavenger::notify() to cause the scavenger thread to be
  // created if it hasn't been created already.

  if (omniORB::trace(20)) {
    omniORB::logger log;
    log << "Server accepted connection from " << conn->peeraddress() << "\n";
  }
}

////////////////////////////////////////////////////////////////////////
giopStrand::~giopStrand()
{
  OMNIORB_ASSERT(pd_state == DYING);

  if (!giopStreamList::is_empty(servers)) {
    giopStreamList* gp = servers.next;
    while (gp != &servers) {
      GIOP_S* g = (GIOP_S*)gp;
      OMNIORB_ASSERT(g->state() == IOP_S::UnUsed);
      gp = gp->next;
      g->giopStreamList::remove();
      delete g;
    }
  }

  if (!giopStreamList::is_empty(clients)) {
    giopStreamList* gp = clients.next;
    while (gp != &clients) {
      GIOP_C* g = (GIOP_C*)gp;
      OMNIORB_ASSERT(g->state() == IOP_C::UnUsed);
      gp = gp->next;
      g->giopStreamList::remove();
      delete g;
    }
  }

  giopStream_Buffer* p = head;
  while (p) {
    giopStream_Buffer* q = p->next;
    giopStream_Buffer::deleteBuffer(p);
    p = q;
  }
  head = 0;
  p = spare;
  while (p) {
    giopStream_Buffer* q = p->next;
    giopStream_Buffer::deleteBuffer(p);
    p = q;
  }
  spare = 0;

  if (compressor)
    delete compressor;
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopStrand::safeDelete(CORBA::Boolean forced)
{
  CORBA::Boolean deleted = 1;

  if (!forced) {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

    deleted = pd_safelyDeleted = 1;

    if (giopStreamList::is_empty(clients) &&
	giopStreamList::is_empty(servers) &&
	giopStream::noLockWaiting(this)) {

      // No other threads should be waiting for a read or write lock
      // on the strand. Otherwise, the GIOP_C or GIOP_S lists would not
      // be empty.
      StrandList::remove();
      RopeLink::remove();
      deleteStrandAndConnection();
    }
  }
  else {
    deleteStrandAndConnection(1);
  }

  return deleted;
}

////////////////////////////////////////////////////////////////////////
void
giopStrand::deleteStrandAndConnection(CORBA::Boolean forced)
{
  // Delete this strand only when connection's reference count goes to 0.
  CORBA::String_var peeraddr;

  if (connection) {

    peeraddr = connection->peeraddress();

    int count;
    if (!forced) {
      count = connection->decrRefCount();
      if (omniORB::trace(25)) {
	omniORB::logger log;
	log << (isClient() ? "Client" : "Server")
	    << " connection " << peeraddr << " refcount = " << count << "\n"; 
      }
      OMNIORB_ASSERT(count >= 0);
      if (count != 0)
	return;
    }
    else {
      count = connection->decrRefCount(1);
      if (omniORB::trace(25)) {
	omniORB::logger log;
	log << (isClient() ? "Client" : "Server")
	    << " connection " << peeraddr
	    << " refcount (forced) = " << count << "\n"; 
      }
      if (count) {
	// The only condition when this happen is when the connection
	// is bidirectional. giopServer still holds a refcount on this
	// connection.
	OMNIORB_ASSERT(isBiDir());
	connection->Shutdown(); // This would cause the giopServer to
	                        // remove this connection as well.
	return;                 // Do not delete the strand. Do so
                                // when the giopServer call this function.
      }
    }
  }
  if (omniORB::trace(20) && connection) {
    omniORB::logger log;
    log << (isClient() ? "Client" : "Server")
	<< " close connection" 
	<< (isClient() ? " to " : " from ")
	<< (const char*)peeraddr << "\n";
  }
  pd_state = DYING;     // satisfy the invariant in the dtor.
  delete this;
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopStrand::deletePending()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);
  return pd_safelyDeleted;
}


////////////////////////////////////////////////////////////////////////
GIOP_S*
giopStrand::acquireServer(giopWorker* w)
{
  // Theory of operation:
  //
  // One or more threads may serve the same strand. However, only
  // one thread works on the same GIOP_S instance. This invariant
  // is enforced by this function.
  //
  // There may be one or more GIOP_S instances linked to this
  // strand. They can be in one of the following states:
  //
  //  1. UnUsed*
  //        - the instance has not been claimed by any thread
  //  2. InputFullyBuffered*
  //        - the instance has not been claimed by
  //          any thread and a complete message has
  //          been received.
  //  3. InputPartiallyBuffered**
  //        - a message has been partially received. The thread
  //          that is currently holding the read lock on this
  //          strand may append additional data to this instance.
  //          If no thread is holding the read lock, this instance
  //          can be claimed by the calling thread.
  //  4. Other states***
  //        - the instance is currently in used by another thread.
  //
  //  Note:
  //    *   the instance can be claimed by the calling thread
  //    **  the instance cannot be claimed by the calling thread unless
  //        no other thread is holding the read lock.
  //    *** the instance cannot be claimed by the calling thread
  //
  // This function also acquires the Read lock on the strand if the returned
  // giopStream instance is not InputFullyBuffered. (If it is
  // InputFullyBuffered, there is no need to read from the strand, hence
  // no need for a Read lock.


  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);

  omni_tracedmutex_lock sync(*omniTransportLock);

  if (deletePending()) {
    // Check if safeDelete() has been called on the strand and has returned
    // true. If this is is the case, we should not proceed or else the
    // invarant
    //
    //    giopStreamList::is_empty(clients) &&
    //	  giopStreamList::is_empty(servers) &&
    //	  giopStream::noLockWaiting(this)
    //     
    // would be violated.
    //
    // Notice that giopServer (worker->server()) may schedule any number of
    // threads to serve this strand and each of these threads may call this
    // function at any time before or after safeDelete() is called and
    // returned true.  We use this check to stop all these threads right
    // here or else the invarant will be violated.
    return 0;
  }

 again:
  // Scan the list to identify the 1st occurrance of an instance in
  // one of these states: UnUsed, InputFullyBuffered, InputPartiallyBuffered.

  GIOP_S* up = 0; // 1st GIOP_S in unused state
  GIOP_S* fp = 0; // 1st GIOP_S in InputFullyBuffered state
  GIOP_S* pp = 0; // 1st GIOP_S in InputPartiallyBuffered state;

  GIOP_S* sp;

  giopStreamList* p = servers.next;
  while (p != &servers) {
    sp = (GIOP_S*)p;
    switch (sp->state()) {
    case IOP_S::UnUsed:
      if (!up) up = sp;
      break;
    case IOP_S::InputFullyBuffered:
      if (!fp) fp = sp;
      break;
    case IOP_S::InputPartiallyBuffered:
      if (!pp) pp = sp;
      break;
    default:
      break;
    }
    p = p->next;
  }
  if (fp) {
    // This is good, no need to acquire a Read Lock on the strand because
    // the whole request message is already in buffer.
    sp = fp;
  }
  else if (giopStream::rdLockNonBlocking(this)) {
    // Now we have got the Read Lock, this is *the* thread to read from
    // the strand until we release the lock.
    // Choose a GIOP_S instance in the following order:
    // 1. InputPartiallyBuffered
    // 2. UnUsed
    // 3. None of the above
    if (pp) {
      sp = pp;
    }
    else if (up) {
      sp = up;
      sp->impl(0);
    }
    else {
      sp = new GIOP_S(this);
      sp->giopStreamList::insert(servers);
    }
    sp->markRdLock();
  }
  else {
    // Another thread is already reading from the strand, we let it does
    // the work and let it wake us up if there is some work to do.
    giopStream::sleepOnRdLock(this);
    goto again;
  }

  sp->state(IOP_S::Idle);
  if (!sp->impl()) {
    giopStreamImpl *impl = giopStreamImpl::maxVersion();
    sp->impl(impl);
    if (version.major == 0)
	version = impl->version();
  }
  // the codeset convertors are filled in by the codeset interceptor
  // before a request is unmarshalled.
  sp->TCS_C(0);
  sp->TCS_W(0);
  sp->worker(w);
  sp->clearDeadline();
  return sp;
}

////////////////////////////////////////////////////////////////////////
void
giopStrand::releaseServer(IOP_S* iop_s)
{
  omni_tracedmutex_lock sync(*omniTransportLock);

  GIOP_S* giop_s = (GIOP_S*) iop_s;

  giop_s->rdUnLock();
  giop_s->wrUnLock();
  giop_s->giopStreamList::remove();

  CORBA::Boolean remove = 0;
  CORBA::Boolean restart_idle = 1;

  if ( state() == giopStrand::DYING ) {
    remove = 1;
    restart_idle = 0;

    // We have to go through the GIOP_S list and delete any ones
    // that are in the Unused and InputFullyBuffered states.
    // We can also delete the ones in InputPartiallyBuffered state
    // if no one is holding the Read lock. See the "theory of operation"
    // in acquireServer() for more info.
    CORBA::Boolean remove_partial = !giopStream::RdLockIsHeld(this);
    giopStreamList* p = servers.next;
    while (p != &servers) {
      GIOP_S* sp = (GIOP_S*)p;
      switch (sp->state()) {
      case IOP_S::InputPartiallyBuffered:
	if (! remove_partial) break;
	// else falls through
      case IOP_S::UnUsed:
      case IOP_S::InputFullyBuffered:
	p = p->next;
	sp->giopStreamList::remove();
	delete sp;
	continue;
      default:
	break;
      }
      p = p->next;
    }

    safeDelete();
  }
  else if ( !giopStreamList::is_empty(servers) ) {
    remove = 1;
    giopStreamList* p = servers.next;
    while (p != &servers) {
      GIOP_S* sp = (GIOP_S*)p;
      if (sp->state() > IOP_S::WaitForRequestHeader) {
	restart_idle = 0;
      }
      p = p->next;
    }
  }
  else {
    giop_s->state(IOP_S::UnUsed);
    giop_s->giopStreamList::insert(servers);
  }

  if (remove) {
    if (giop_s->state() != IOP_S::WaitingForReply)
      delete giop_s;
    else
      restart_idle = 0;
  }

  if (restart_idle && !isBiDir()) {
    CORBA::Boolean success = startIdleCounter();
    if (!success) {
      if (omniORB::trace(1)) {
	omniORB::logger log;
	log << "Error: strand idle counter already running for strand "
	    << this << ".\n";
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
CORBA::ULong
giopStrand::newSeqNumber() {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);
  seqNumber += 2;
  return seqNumber;
}

////////////////////////////////////////////////////////////////////////
StrandList giopStrand::active;
StrandList giopStrand::active_timedout;
StrandList giopStrand::passive;
// Throughout the lifetime of a strand, it is a member of one and only one
// of the lists:
//   active           - the ORB uses this connection in the role of a client
//                      it is 'active' in the sense that the connection was
//                      initiated by this ORB
//   active_timedout  - the connection was previously active and has been
//                      idled for some time. It will be deleted soon.
//   passive          - the ORB uses this connection in the role of a server
//                      it is 'passive' because the connection was initiated
//                      by the remote party.
//

CORBA::ULong giopStrand::idleIncomingBeats = 36;
CORBA::ULong giopStrand::idleOutgoingBeats = 24;

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopStrand::startIdleCounter() {
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  if (idlebeats >= 0) {
    // The idle counter is already active or has already expired.
    return 0;
  }
  if (flags & GIOPSTRAND_HOLD_OPEN) {
    // Flags say we should hold the connection open.
    idlebeats = -1;
    return 1;
  }
  if (isClient()) {
    idlebeats = (idleOutgoingBeats) ? (CORBA::Long)idleOutgoingBeats : -1;
  }
  else {
    idlebeats = (idleIncomingBeats) ? (CORBA::Long)idleIncomingBeats : -1;
  }
  return 1;
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopStrand::stopIdleCounter() {
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  if (idlebeats == 0) {
    // The idle counter has already expired.
    return 0;
  }
  idlebeats = -1;
  return 1;
}


////////////////////////////////////////////////////////////////////////
void
Scavenger::removeIdle(StrandList& src,StrandList& dest,
		      CORBA::Boolean client)
{
  StrandList* p = src.next;
  while (p != &src) {
    giopStrand* s = (giopStrand*)p;

    if ( s->idlebeats >= 0 ) {

      if (omniORB::trace(30)) {
	omniORB::logger log;
	log << "Scavenger reduce idle count for strand "
	    << (void*)s << " to " << (s->idlebeats - 1) << "\n";
      }
      if (--(s->idlebeats) <= 0) {
	p = p->next;
	s->StrandList::remove();
	s->RopeLink::remove();
	s->StrandList::insert(dest);

	if (omniORB::trace(25)) {
	  omniORB::logger log;
	  log << "Scavenger close ";
	  if (client) {
	    log << (s->connection ? "connection" : "unconnected strand")
		<< " to " << s->address->address();
	  }
	  else {
	    log << "connection from " << s->connection->peeraddress();
	  }
	  log << "\n";
	}
	continue;
      }
    }

    p = p->next;
  }
}

////////////////////////////////////////////////////////////////////////
void
Scavenger::execute()
{
  omniORB::logs(25, "Scavenger task execute.");

  unsigned long abs_sec,abs_nsec;
  omni_thread::get_time(&abs_sec,&abs_nsec);

  while (1) {
    {
      omni_tracedmutex_lock sync(*mutex);
      if ( shutdown || !orbParameters::scanGranularity ) {
	goto died;
      }
      omni_thread::get_time(&abs_sec,&abs_nsec,orbParameters::scanGranularity);
      cond->timedwait(abs_sec,abs_nsec);
    }

    if (omniORB::trace(30)) {
      omniORB::logger log;
      log << "Scan for idle connections ("
	  << abs_sec << "," << abs_nsec << ")\n";
    }

    StrandList client_shutdown_list;
    StrandList server_shutdown_list;
    {
      // Collect all the strands that should be removed
      // We want to minimise the time we hold the omniTransportLock.
      // Therefore we remove the strands and keep it in the private
      // list for the time being.

      omni_tracedmutex_lock sync(*omniTransportLock);

      // Notice that only those strands that have no other threads touching
      // them will be scanned and may be removed.
      removeIdle(giopStrand::active_timedout,client_shutdown_list,1);
      removeIdle(giopStrand::active,client_shutdown_list,1);
      removeIdle(giopStrand::passive,server_shutdown_list,0);
    }

    // Now go through the list to delete them all
    CORBA::Boolean scavenged_client_strands = 0;
    {
      StrandList* p = client_shutdown_list.next;
      while ( p != &client_shutdown_list ) {

        scavenged_client_strands = 1;

	giopStrand* s = (giopStrand*)p;
	p = p->next;
	s->StrandList::remove();
	s->state(giopStrand::DYING);
        if ( s->version.minor >= 2 && s->connection ) {
          // GIOP 1.2 or above requires the client send a CloseConnection
          // message.
          sendCloseConnection(s);
        }
	{
	  omni_optional_lock sync(*omniTransportLock,
                                  !s->isBiDir(), !s->isBiDir());
	  s->safeDelete(1);
	}
      }
    }

    if (scavenged_client_strands) {
      // We have scavenged at least one client strand, so we reset any
      // idle giopRope address orders.
      giopRope::resetIdleRopeAddresses();
    }
    
    {
      // We have to hold <omniTransportLock> while disposing of the
      // server strands, since other threads may be dealing with them.
      omni_tracedmutex_lock sync(*omniTransportLock);

      StrandList* p = server_shutdown_list.next;
      while ( p != &server_shutdown_list ) {
	giopStrand* s = (giopStrand*)p;
	p = p->next;
	s->StrandList::remove();
	s->state(giopStrand::DYING);
	sendCloseConnection(s);
	s->connection->Shutdown();
      }
    }

    if (omniORB::trace(30)) {
      omniORB::logger log;
      log << "Scan for idle connections done ("
	  << abs_sec << "," << abs_nsec << ").\n";
    }
  }

 died:
  {
    mutex->lock();
    theTask = 0;
    if (shutdown) {
      mutex->unlock();
      delete cond;
      delete mutex;
    }
    else {
      mutex->unlock();
    }
    delete this;
  }
}

////////////////////////////////////////////////////////////////////////
void
Scavenger::notify()
{
  omni_tracedmutex_lock sync(*mutex);
  if ( !shutdown && orbParameters::scanGranularity &&!theTask ) {
    theTask = new Scavenger();
    orbAsyncInvoker->insert(theTask);
  }
}

////////////////////////////////////////////////////////////////////////
void
Scavenger::terminate()
{
  mutex->lock();
  if (theTask) {
    shutdown = 1;
    cond->signal();
    mutex->unlock();
  }
  else {
    mutex->unlock();
    delete cond; cond = 0;
    delete mutex; mutex = 0;
  }
}

void
Scavenger::initialise()
{
  Scavenger::shutdown = 0;
  Scavenger::mutex = new omni_tracedmutex("Scavenger::mutex");
  Scavenger::cond  = new omni_tracedcondition(Scavenger::mutex,
					      "Scavenger::cond");
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean        Scavenger::shutdown = 0;
omni_tracedmutex*     Scavenger::mutex = 0;
omni_tracedcondition* Scavenger::cond = 0;
Scavenger*            Scavenger::theTask = 0;

/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class scanGranularityHandler : public orbOptions::Handler {
public:

  scanGranularityHandler() : 
    orbOptions::Handler("scanGranularity",
			"scanGranularity = n >= 0 sec",
			1,
			"-ORBscanGranularity < n >= 0 sec >") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::ULong v;
    if (!orbOptions::getULong(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_ulong_msg);
    }
    orbParameters::scanGranularity = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::scanGranularity,
			   result);
  }
};

static scanGranularityHandler scanGranularityHandler_;

/////////////////////////////////////////////////////////////////////////////
class outConScanPeriodHandler : public orbOptions::Handler {
public:

  outConScanPeriodHandler() : 
    orbOptions::Handler("outConScanPeriod",
			"outConScanPeriod = n >= 0 sec",
			1,
			"-ORBoutConScanPeriod < n >= 0 sec >") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::ULong v;
    if (!orbOptions::getULong(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_ulong_msg);
    }
    orbParameters::outConScanPeriod = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::outConScanPeriod,
			   result);
  }
};

static outConScanPeriodHandler outConScanPeriodHandler_;

/////////////////////////////////////////////////////////////////////////////
class inConScanPeriodHandler : public orbOptions::Handler {
public:

  inConScanPeriodHandler() : 
    orbOptions::Handler("inConScanPeriod",
			"inConScanPeriod = n >= 0 sec",
			1,
			"-ORBinConScanPeriod < n >= 0 sec >") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::ULong v;
    if (!orbOptions::getULong(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_ulong_msg);
    }
    orbParameters::inConScanPeriod = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::inConScanPeriod,
			   result);
  }
};

static inConScanPeriodHandler inConScanPeriodHandler_;

////////////////////////////////////////////////////////////////////////
// Module initialiser
////////////////////////////////////////////////////////////////////////

class omni_giopStrand_initialiser : public omniInitialiser {
public:
  omni_giopStrand_initialiser() {
    orbOptions::singleton().registerHandler(scanGranularityHandler_);
    orbOptions::singleton().registerHandler(outConScanPeriodHandler_);
    orbOptions::singleton().registerHandler(inConScanPeriodHandler_);
  }

  void attach() {

    if (orbParameters::outConScanPeriod && orbParameters::scanGranularity) {
      if (orbParameters::outConScanPeriod <= orbParameters::scanGranularity) {
	giopStrand::idleOutgoingBeats = 1;
	omniORB::logs(2, "Warning: outConScanPeriod <= scanGranularity means "
		      "connections can be closed while still in use.");
      }
      else {
	giopStrand::idleOutgoingBeats = orbParameters::outConScanPeriod /
      	                                orbParameters::scanGranularity;
      }
    }
    else {
      giopStrand::idleOutgoingBeats = 0;
    }
    if (orbParameters::inConScanPeriod && orbParameters::scanGranularity) {
      if (orbParameters::inConScanPeriod <= orbParameters::scanGranularity) {
	giopStrand::idleIncomingBeats = 1;
	omniORB::logs(2, "Warning: inConScanPeriod <= scanGranularity means "
		      "connections can be closed while still in use.");
      }
      else {
	giopStrand::idleIncomingBeats = orbParameters::inConScanPeriod /
      	                                orbParameters::scanGranularity;
      }
    }
    else {
      giopStrand::idleIncomingBeats = 0;
    }

    Scavenger::initialise();
  }
  void detach() {
    omniORB::logs(25, "Terminate strand scavenger.");
    Scavenger::terminate();

    omni_tracedmutex_lock sync(*omniTransportLock);

    omniORB::logs(25, "Close remaining strands.");

    // Close client strands
    {
      StrandList* p = giopStrand::active_timedout.next;
      while ( p != &giopStrand::active_timedout ) {
	giopStrand* s = (giopStrand*)p;
	p = p->next;
	s->StrandList::remove();
	s->RopeLink::remove();
	s->state(giopStrand::DYING);
	if (omniORB::trace(25)) {
	  omniORB::logger log;
	  log << "Shutdown close "
              << (s->connection ? "connection" : "unconnected strand")
              << " to "
	      << s->address->address() << "\n";
	}
        if ( s->version.minor >= 2 && s->connection ) {
          // GIOP 1.2 or above requires the client send a closeconnection
          // message.
          sendCloseConnection(s);
        }
	s->safeDelete(1);
      }
    }
    {
      StrandList* p = giopStrand::active.next;
      while ( p != &giopStrand::active ) {
	giopStrand* s = (giopStrand*)p;
	p = p->next;
	s->StrandList::remove();
	s->RopeLink::remove();
	s->state(giopStrand::DYING);
	if (omniORB::trace(25)) {
	  omniORB::logger log;
	  log << "Shutdown close connection to "
	      << s->address->address() << "\n";
	}
	if ( s->version.minor >= 2 ) {
	  // GIOP 1.2 or above requires the client send a closeconnection
	  // message.
	  sendCloseConnection(s);
	}
	s->safeDelete(1);
      }
    }
    // Close server strands
    {
      StrandList* p = giopStrand::passive.next;
      while ( p != &giopStrand::passive ) {
	giopStrand* s = (giopStrand*)p;
	p = p->next;
	s->StrandList::remove();
	s->RopeLink::remove();
	s->state(giopStrand::DYING);
	if (omniORB::trace(25)) {
	  omniORB::logger log;
	  log << "Shutdown close connection from " 
	      << s->connection->peeraddress() << "\n";
	}	
	sendCloseConnection(s);
	s->connection->Shutdown();
      }
    }
  }
};

static omni_giopStrand_initialiser initialiser;

omniInitialiser& omni_giopStrand_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)


