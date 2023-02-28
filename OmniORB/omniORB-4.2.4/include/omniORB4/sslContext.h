// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslContext.h               Created on: 29 May 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2005-2012 Apasphere Ltd
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

#ifndef __SSLCONTEXT_H__
#define __SSLCONTEXT_H__

#include <omniORB4/linkHacks.h>

OMNI_FORCE_LINK(omnisslTP);


#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_SSL_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

#define crypt _openssl_broken_crypt
#include <openssl/ssl.h>
#undef crypt

OMNI_NAMESPACE_BEGIN(omni)
  class omni_sslTransport_initialiser;
OMNI_NAMESPACE_END(omni)

class sslContext {
 public:
  sslContext(const char* cafile, const char* keyfile, const char* password);

  SSL_CTX* get_SSL_CTX() const { return pd_ctx; }
  
  // These parameters must be set or else the default way to
  // initialise a sslContext singleton will not be used.
  static _core_attr const char* certificate_authority_file; // In PEM format
  static _core_attr const char* certificate_authority_path; // Path
  static _core_attr const char* key_file;                   // In PEM format
  static _core_attr const char* key_file_password;

  // These parameters can be overriden to adjust the verify mode and
  // verify callback passed to SSL_CTX_set_verify and the info
  // callback passed to SSL_CTX_set_info_callback.
  static _core_attr int         verify_mode;
  static _core_attr int       (*verify_callback)(int, X509_STORE_CTX*);

  static _core_attr void      (*info_callback)(const SSL *s,
					       int where, int ret);

  // If this parameter is false (the default), interceptor
  // peerdetails() calls treturn an X509*. If set true, the calls
  // return a pointer to an sslContext::PeerDetails object.
  static _core_attr CORBA::Boolean full_peerdetails;

  class PeerDetails {
  public:
    inline PeerDetails(SSL* s, X509* c, CORBA::Boolean v)
      : pd_ssl(s), pd_cert(c), pd_verified(v) {}

    ~PeerDetails();

    inline SSL*           ssl()      { return pd_ssl; }
    inline X509*          cert()     { return pd_cert; }
    inline CORBA::Boolean verified() { return pd_verified; }

  private:
    SSL*           pd_ssl;
    X509*          pd_cert;
    CORBA::Boolean pd_verified;
  };

  // sslContext singleton object.
  static _core_attr sslContext* singleton;

  virtual ~sslContext();

 protected:
  virtual SSL_METHOD* set_method(); 
  // Default to return SSLv23_method().

  virtual void set_supported_versions(); 
  // Default to SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_SSLv2|SSL_OP_NO_SSLv3);
  // That is only accept TLS.

  virtual void set_CA();
  // Default to read the certificates of the Certificate Authorities in the 
  // file named by the static member certificate_authority_file.

  virtual void set_certificate();
  // Default to read the certificate of this server from the file named
  // by the static member key_file. 

  virtual void set_cipher();
  // Default to call OpenSSL_add_all_algorithms().

  virtual void set_privatekey();
  // Default to read the private key of this server from the file named
  // by the static member key_file. Notice that this file also contains
  // the server's certificate.

  virtual void seed_PRNG();
  // On systems that does not provide a /dev/urandom, default to provide
  // a seed for the PRNG using process ID and time of date. This is not
  // a very good seed cryptographically. Secure applications should definitely
  // override this method to provide a better seed.

  virtual void set_DH();

  virtual void set_ephemeralRSA();

  virtual int set_verify_mode();
  // Set the SSL verify mode.
  // Defaults to return SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT.

  sslContext();

  friend class _OMNI_NS(omni_sslTransport_initialiser);
 private:

  void thread_setup();
  void thread_cleanup();

  virtual void internal_initialise();

  const char* 	    pd_cafile;
  const char* 	    pd_keyfile;
  const char* 	    pd_password;
  SSL_CTX*    	    pd_ctx;
  omni_tracedmutex* pd_locks;
  CORBA::Boolean    pd_ssl_owner;
};

#undef _core_attr

#endif // __SSLCONTEXT_H__
