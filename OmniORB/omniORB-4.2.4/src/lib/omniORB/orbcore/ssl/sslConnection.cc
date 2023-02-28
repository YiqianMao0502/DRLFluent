// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslConnection.cc           Created on: 19 Mar 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2015 Apasphere Ltd
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

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <orbParameters.h>
#include <SocketCollection.h>
#include <tcpSocket.h>
#include <omniORB4/sslContext.h>
#include <ssl/sslConnection.h>
#include <ssl/sslEndpoint.h>
#include <ssl/sslTransportImpl.h>
#include <openssl/err.h>
#include <stdio.h>
#include <giopStreamImpl.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(sslConnection);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
static inline CORBA::Boolean
handleErrorSyscall(int ret, int ssl_err, const char* peer, const char* kind)
{
  int errno_val = ERRNO;
  if (RC_TRY_AGAIN(errno_val))
    return 1;

  int err_err = ERR_get_error();
  if (err_err) {
    while (err_err) {
      if (omniORB::trace(10)) {
        char buf[128];
        ERR_error_string_n(err_err, buf, 128);

        omniORB::logger log;
        log << peer << " " << kind << " error: " << (const char*)buf << "\n";
      }
      err_err = ERR_get_error();
    }
  }
  else if (omniORB::trace(10)) {
    omniORB::logger log;
    log << peer << " " << kind;

    if (ssl_err == SSL_ERROR_ZERO_RETURN)
      log << " connection has been closed.\n";
    else if (ret == 0)
      log << " observed an EOF that violates the protocol.\n";
    else if (ret == -1)
      log << " received an I/O error (" << errno_val << ").\n";
    else
      log << " unexepctedly returned " << ret << ".\n";
  }
  return 0;
}


/////////////////////////////////////////////////////////////////////////
int
sslConnection::Send(void* buf, size_t sz,
		    const omni_time_t& deadline) {

  if (!pd_handshake_ok) {
    omniORB::logs(25, "Send failed because SSL handshake not yet completed.");
    return -1;
  }

  if (sz > orbParameters::maxSocketSend)
    sz = orbParameters::maxSocketSend;

  int tx;
  int ssl_err;

  do {
    struct timeval t;

    if (deadline) {
      if (tcpSocket::setTimeout(deadline, t)) {
	// Already timed out.
	return 0;
      }
      else {
        setNonBlocking();

        tx = tcpSocket::waitWrite(pd_socket, t);

	if (tx == 0) {
	  // Timed out
	  return 0;
	}
	else if (tx == RC_SOCKET_ERROR) {
	  if (ERRNO == RC_EINTR) {
	    continue;
          }
	  else {
	    return -1;
	  }
	}
      }
    }
    else {
      setBlocking();
    }

    // Reach here if we can write without blocking or we don't
    // care if we block here.
#if OPENSSL_VERSION_NUMBER >= 0x0090601fL
    tx = SSL_write(pd_ssl,buf,sz);
#else
    tx = SSL_write(pd_ssl,(char*)buf,sz);
#endif

    ssl_err = SSL_get_error(pd_ssl, tx);

    switch (ssl_err) {
    case SSL_ERROR_NONE:
      break;

    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      continue;

    case SSL_ERROR_SSL:
    case SSL_ERROR_ZERO_RETURN:
    case SSL_ERROR_SYSCALL:
      if (handleErrorSyscall(tx, ssl_err, pd_peeraddress, "send"))
        continue;
      else
        return -1;

    default:
      OMNIORB_ASSERT(0);
    }

    OMNIORB_ASSERT(tx != 0);

    break;

  } while(1);

  return tx;
}

/////////////////////////////////////////////////////////////////////////
int
sslConnection::Recv(void* buf, size_t sz,
		    const omni_time_t& deadline) {

  if (!pd_handshake_ok) {
    omniORB::logs(25, "Recv failed because SSL handshake not yet completed.");
    return -1;
  }

  if (sz > orbParameters::maxSocketRecv)
    sz = orbParameters::maxSocketRecv;

  int rx;
  int ssl_err;

  do {
    if (pd_shutdown)
      return -1;

    struct timeval t;

    if (tcpSocket::setAndCheckTimeout(deadline, t)) {
      // Already timed out
      return 0;
    }

    if (t.tv_sec || t.tv_usec) {
      setNonBlocking();
      rx = SSL_pending(pd_ssl) || tcpSocket::waitRead(pd_socket, t);

      if (rx == 0) {
	// Timed out
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
	continue;
#else
	return 0;
#endif
      }
      else if (rx == RC_SOCKET_ERROR) {
	if (ERRNO == RC_EINTR) {
	  continue;
        }
	else {
	  return -1;
	}
      }
    }
    else {
      setBlocking();
    }

    // Reach here if we can read without blocking or we don't
    // care if we block here.
#if OPENSSL_VERSION_NUMBER >= 0x0090601fL
    rx = SSL_read(pd_ssl,buf,sz);
#else
    rx = SSL_read(pd_ssl,(char*)buf,sz);
#endif

    ssl_err = SSL_get_error(pd_ssl, rx);

    switch (ssl_err) {
    case SSL_ERROR_NONE:
      break;

    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      continue;

    case SSL_ERROR_SSL:
    case SSL_ERROR_ZERO_RETURN:
    case SSL_ERROR_SYSCALL:
      if (handleErrorSyscall(rx, ssl_err, pd_peeraddress, "recv"))
        continue;
      else
        return -1;

    default:
      OMNIORB_ASSERT(0);
    }

    OMNIORB_ASSERT(rx != 0);

    break;

  } while(1);

  return rx;
}

/////////////////////////////////////////////////////////////////////////
void
sslConnection::Shutdown() {
  SSL_set_shutdown(pd_ssl, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);
  SSL_shutdown(pd_ssl);
  SHUTDOWNSOCKET(pd_socket);
  pd_shutdown = 1;
}

/////////////////////////////////////////////////////////////////////////
const char*
sslConnection::myaddress() {
  return (const char*)pd_myaddress;
}

/////////////////////////////////////////////////////////////////////////
const char*
sslConnection::peeraddress() {
  return (const char*)pd_peeraddress;
}

/////////////////////////////////////////////////////////////////////////
const char*
sslConnection::peeridentity() {
  return (const char *)pd_peeridentity;
}

/////////////////////////////////////////////////////////////////////////
void*
sslConnection::peerdetails() {

  if (sslContext::full_peerdetails) {
    return (void*)pd_peerdetails;
  }
  else if (pd_peerdetails && pd_peerdetails->verified()) {
    return (void*)pd_peerdetails->cert();
  }
  else {
    return 0;
  }
}

/////////////////////////////////////////////////////////////////////////
_CORBA_Boolean
sslConnection::gatekeeperCheckSpecific(giopStrand* strand)
{
  // Perform SSL accept

  if (omniORB::trace(25)) {
    omniORB::logger log;
    CORBA::String_var peer = tcpSocket::peerToURI(pd_socket, "giop:ssl");
    log << "Perform SSL accept for new incoming connection " << peer << "\n";
  }

  omni_time_t deadline;
  struct timeval tv;

  if (sslTransportImpl::sslAcceptTimeOut) {

    tcpSocket::setNonBlocking(pd_socket);
    omni_thread::get_time(deadline, sslTransportImpl::sslAcceptTimeOut);
  }

  int timeout = 0;
  int go = 1;

  while (go && !pd_shutdown) {
    if (tcpSocket::setAndCheckTimeout(deadline, tv)) {
      // Timed out
      timeout = 1;
      break;
    }

    int result = SSL_accept(pd_ssl);
    int code   = SSL_get_error(pd_ssl, result);

    switch(code) {
    case SSL_ERROR_NONE:
      tcpSocket::setBlocking(pd_socket);
      pd_handshake_ok = 1;
      setPeerDetails();
      return 1;

    case SSL_ERROR_WANT_READ:
      if (tcpSocket::waitRead(pd_socket, tv) == 0) {
	timeout = 1;
	go = 0;
      }
      continue;

    case SSL_ERROR_WANT_WRITE:
      if (tcpSocket::waitWrite(pd_socket, tv) == 0) {
	timeout = 1;
	go = 0;
      }
      continue;

    case SSL_ERROR_SYSCALL:
      {
	if (ERRNO == RC_EINTR)
	  continue;
      }
      // otherwise falls through
    case SSL_ERROR_SSL:
    case SSL_ERROR_ZERO_RETURN:
      {
	if (omniORB::trace(10)) {
	  omniORB::logger log;
	  char buf[128];
	  ERR_error_string_n(ERR_get_error(), buf, 128);
	  CORBA::String_var peer = tcpSocket::peerToURI(pd_socket, "giop:ssl");
	  log << "OpenSSL error detected in SSL accept from "
	      << peer << " : " << (const char*) buf << "\n";
	}
	go = 0;
      }
    }
  }
  if (timeout && omniORB::trace(10)) {
    omniORB::logger log;
    CORBA::String_var peer = tcpSocket::peerToURI(pd_socket, "giop:ssl");
    log << "Timeout in SSL accept from " << peer << "\n";
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////
sslConnection::sslConnection(SocketHandle_t sock,::SSL* ssl, 
			     SocketCollection* belong_to) : 
  SocketHolder(sock), pd_ssl(ssl), pd_handshake_ok(0), pd_peerdetails(0)
{
  OMNI_SOCKADDR_STORAGE addr;
  SOCKNAME_SIZE_T l;

  l = sizeof(OMNI_SOCKADDR_STORAGE);
  if (getsockname(pd_socket,
		  (struct sockaddr *)&addr,&l) == RC_SOCKET_ERROR) {
    pd_myaddress = (const char*)"giop:ssl:255.255.255.255:65535";
  }
  else {
    pd_myaddress = tcpSocket::addrToURI((sockaddr*)&addr, "giop:ssl");
  }

  l = sizeof(OMNI_SOCKADDR_STORAGE);
  if (getpeername(pd_socket,
		  (struct sockaddr *)&addr,&l) == RC_SOCKET_ERROR) {
    pd_peeraddress = (const char*)"giop:ssl:255.255.255.255:65535";
  }
  else {
    pd_peeraddress = tcpSocket::addrToURI((sockaddr*)&addr, "giop:ssl");
  }
  tcpSocket::setCloseOnExec(sock);

  belong_to->addSocket(this);
  setPeerDetails();
}

/////////////////////////////////////////////////////////////////////////
sslConnection::~sslConnection() {

  clearSelectable();
  pd_belong_to->removeSocket(this);

  if (pd_peerdetails) {
    delete pd_peerdetails;
    pd_peerdetails = 0;
  }

  if (pd_ssl != 0) {
    if (SSL_get_shutdown(pd_ssl) == 0) {
      SSL_set_shutdown(pd_ssl, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);
      SSL_shutdown(pd_ssl);
    }
    SSL_free(pd_ssl);
    pd_ssl = 0;
  }

  CLOSESOCKET(pd_socket);
}


/////////////////////////////////////////////////////////////////////////
void
sslConnection::setPeerDetails() {

  // Determine our peer identity, if there is one

  if (pd_peerdetails)
    return;

  X509           *peer_cert = SSL_get_peer_certificate(pd_ssl);
  CORBA::Boolean  verified  = 0;

  if (peer_cert) {
    verified       = SSL_get_verify_result(pd_ssl) == X509_V_OK;
    pd_peerdetails = new sslContext::PeerDetails(pd_ssl, peer_cert, verified);

    int lastpos = -1;

    X509_NAME* name = X509_get_subject_name(peer_cert);
    lastpos = X509_NAME_get_index_by_NID(name, NID_commonName, lastpos);

    if (lastpos == -1)
      return;

    X509_NAME_ENTRY* ne       = X509_NAME_get_entry(name, lastpos);
    ASN1_STRING*     asn1_str = X509_NAME_ENTRY_get_data(ne);

    // Convert to native code set
    cdrMemoryStream stream;
    GIOP::Version ver = giopStreamImpl::maxVersion()->version();
    stream.TCS_C(omniCodeSet::getTCS_C(omniCodeSet::ID_UTF_8, ver));

    if (ASN1_STRING_type(asn1_str) != V_ASN1_UTF8STRING) {
      unsigned char* s = 0;
      int len = ASN1_STRING_to_UTF8(&s, asn1_str);
      if (len == -1)
        return;

      CORBA::ULong(len+1) >>= stream;
      stream.put_octet_array(s, len);
      stream.marshalOctet(0);
      OPENSSL_free(s);
    }
    else {
      int len = ASN1_STRING_length(asn1_str);
      CORBA::ULong(len+1) >>= stream;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
      stream.put_octet_array(ASN1_STRING_data(asn1_str), len);
#else
      stream.put_octet_array(ASN1_STRING_get0_data(asn1_str), len);
#endif
      stream.marshalOctet(0);
    }

    try {
      pd_peeridentity = stream.unmarshalString();
    }
    catch (CORBA::SystemException &ex) {
      if (omniORB::trace(2)) {
	omniORB::logger log;
	log << "Failed to convert SSL peer identity to native code set ("
	    << ex._name() << ")\n";
      }
    }
  }
}


/////////////////////////////////////////////////////////////////////////
void
sslConnection::setSelectable(int now,
			     CORBA::Boolean data_in_buffer) {

  if (SSL_pending(ssl_handle())) data_in_buffer = 1;

  SocketHolder::setSelectable(now,data_in_buffer);
}


/////////////////////////////////////////////////////////////////////////
void
sslConnection::clearSelectable() {

  SocketHolder::clearSelectable();
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
sslConnection::isSelectable() {
  return pd_belong_to->isSelectable(pd_socket);
}


/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
sslConnection::Peek() {

  if (SSL_pending(ssl_handle())) {
    return 1;
  }
  return SocketHolder::Peek();
}


OMNI_NAMESPACE_END(omni)
