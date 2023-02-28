// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslContext.cc              Created on: 29 May 2001
//                            Author    : Sai Lai Lo (sll)
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
//	*** PROPRIETARY INTERFACE ***
//

#include <omniORB4/CORBA.h>

#include <stdlib.h>
#ifndef __WIN32__
#include <unistd.h>
#else
#include <process.h>
#endif
#include <omniORB4/minorCode.h>
#include <omniORB4/sslContext.h>
#include <exceptiondefs.h>
#include <ssl/sslTransportImpl.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(sslContext);

OMNI_USING_NAMESPACE(omni)

static void report_error();

const char*    sslContext::certificate_authority_file = 0;
const char*    sslContext::certificate_authority_path = 0;
const char*    sslContext::key_file = 0;
const char*    sslContext::key_file_password = 0;
int            sslContext::verify_mode = (SSL_VERIFY_PEER |
                                          SSL_VERIFY_FAIL_IF_NO_PEER_CERT);
int          (*sslContext::verify_callback)(int,X509_STORE_CTX *) = 0;
void         (*sslContext::info_callback)(const SSL *s, int where, int ret) = 0;
CORBA::Boolean sslContext::full_peerdetails = 0;

sslContext*    sslContext::singleton = 0;


/////////////////////////////////////////////////////////////////////////
sslContext::sslContext(const char* cafile,
		       const char* keyfile,
		       const char* password) :
  pd_cafile(cafile), pd_keyfile(keyfile), pd_password(password), pd_ctx(0),
  pd_locks(0), pd_ssl_owner(0) {}


/////////////////////////////////////////////////////////////////////////
sslContext::sslContext() :
  pd_cafile(0), pd_keyfile(0), pd_password(0), pd_ctx(0),
  pd_locks(0), pd_ssl_owner(0) {
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::internal_initialise() {

  if (pd_ctx) return;

  // Assume we own the ssl if no locking callback yet.
  pd_ssl_owner = CRYPTO_get_locking_callback() == 0;

  if (pd_ssl_owner) {
    SSL_library_init();
    set_cipher();
    SSL_load_error_strings();
  }

  pd_ctx = SSL_CTX_new(set_method());
  if (!pd_ctx) {
    report_error();
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,
		  CORBA::COMPLETED_NO);
  }

  static const unsigned char session_id_context[] = "omniORB";
  size_t session_id_len =
    (sizeof(session_id_context) >= SSL_MAX_SSL_SESSION_ID_LENGTH ?
     SSL_MAX_SSL_SESSION_ID_LENGTH : sizeof(session_id_context));

  if (SSL_CTX_set_session_id_context(pd_ctx,
				     session_id_context, session_id_len) != 1) {
    report_error();
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,
		  CORBA::COMPLETED_NO);
  }

  set_supported_versions();
  seed_PRNG();
  set_certificate();
  set_privatekey();
  set_CA();
  set_DH();
  set_ephemeralRSA();

  // Allow the user to overwrite the SSL verification types.
  SSL_CTX_set_verify(pd_ctx, set_verify_mode(), verify_callback);
  SSL_CTX_set_info_callback(pd_ctx, info_callback);

  if (pd_ssl_owner)
    thread_setup();
}

/////////////////////////////////////////////////////////////////////////
sslContext::~sslContext() {
  if (pd_ctx) {
    SSL_CTX_free(pd_ctx);
  }
  if (pd_ssl_owner)
    thread_cleanup();
}

/////////////////////////////////////////////////////////////////////////
sslContext::PeerDetails::~PeerDetails() {
  if (pd_cert)
    X509_free(pd_cert);
}

/////////////////////////////////////////////////////////////////////////
SSL_METHOD*
sslContext::set_method() {
  return OMNI_CONST_CAST(SSL_METHOD*, SSLv23_method());
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_supported_versions() {
  SSL_CTX_set_options(pd_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_CA() {

  if (!(SSL_CTX_load_verify_locations(pd_ctx, pd_cafile,
                                      certificate_authority_path))) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Failed to set CA";

      if (pd_cafile)
        log << " file '" << pd_cafile << "'";

      if (certificate_authority_path)
        log << " path '" << certificate_authority_path << "'";

      log << ".\n";
    }
      
    report_error();
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
  }

  if (pd_cafile) {
    // Set the client CA list
    SSL_CTX_set_client_CA_list(pd_ctx, SSL_load_client_CA_file(pd_cafile));
  }
  
  // We no longer set the verify depth to 1, to use the default of 9.
  //  SSL_CTX_set_verify_depth(pd_ctx,1);
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_certificate() {
  {
    if (!pd_keyfile) {
      if (omniORB::trace(5)) {
	omniORB::logger log;
	log << "sslContext certificate file is not set.\n";
      }
      return;
    }
  }

  if(!(SSL_CTX_use_certificate_chain_file(pd_ctx, pd_keyfile))) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Failed to use certificate file '" << pd_keyfile << "'.\n";
    }
    report_error();
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
  }
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_cipher() {
  OpenSSL_add_all_algorithms();
}

/////////////////////////////////////////////////////////////////////////
static const char* ssl_password = 0;

extern "C"
int sslContext_password_cb (char *buf,int num,int,void *) {
  int size = strlen(ssl_password);
  if (num < size+1) return 0;
  strcpy(buf,ssl_password);
  return size;
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_privatekey() {

  if (!pd_password) {
    if (omniORB::trace(5)) {
      omniORB::logger log;
      log << "sslContext private key is not set\n";
    }
    return;
  }

  ssl_password = pd_password;
  SSL_CTX_set_default_passwd_cb(pd_ctx,sslContext_password_cb);
  if(!(SSL_CTX_use_PrivateKey_file(pd_ctx,pd_keyfile,SSL_FILETYPE_PEM))) {
    report_error();
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
  }
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::seed_PRNG() {
  // Seed the PRNG if it has not been done
  if (!RAND_status()) {

    // This is not necessary on systems with /dev/urandom. Otherwise, the
    // application is strongly adviced to seed the PRNG using one of the
    // seeding functions: RAND_seed(), RAND_add(), RAND_event() or
    // RAND_screen().
    // What we do here is a last resort and does not necessarily give a very
    // good seed!

    int* data = new int[256];

#if ! defined(__WIN32__)
    srand(getuid() + getpid());
#else
    srand(_getpid());
#endif
    int i;
    for(i = 0 ; i < 128 ; ++i)
      data[i] = rand();

    unsigned long abs_sec, abs_nsec;
    omni_thread::get_time(&abs_sec,&abs_nsec);
    srand(abs_sec + abs_nsec);
    for(i = 128 ; i < 256 ; ++i)
      data[i] = rand();

    RAND_seed((unsigned char *)data, (256 * (sizeof(int))));

    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "SSL: the pseudo random number generator has not been seeded.\n"
	  << "A seed is generated but it is not consided to be of crypto strength.\n"
	  << "The application should call one of the OpenSSL seed functions,\n"
	  << "e.g. RAND_event() to initialise the PRNG before calling sslTransportImpl::initialise().\n";
    }
  }
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_DH() {

  DH* dh = DH_new();
  if(dh == 0) {
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
  }

  static unsigned char dh2048_p[]={
    0xD5,0xB3,0xFA,0xA4,0xD2,0x01,0xB3,0x20,0x56,0x8A,0x57,0xB6,
    0xFA,0xCF,0x5B,0xEA,0x44,0xD6,0xBD,0xB2,0x09,0x2A,0x82,0x81,
    0x6E,0x5E,0x60,0x7C,0xBB,0x20,0x2C,0xFE,0x2F,0x99,0x28,0x12,
    0xCC,0xAA,0x90,0xC5,0x76,0x53,0xFC,0xA4,0xC6,0x82,0x03,0xDE,
    0x32,0xB2,0xA2,0xD5,0xA8,0xBC,0xDE,0x0D,0xFF,0xA6,0xC7,0xA2,
    0xD3,0x40,0x56,0x2E,0x5C,0x30,0x91,0x1B,0xC3,0xA9,0x8D,0x3C,
    0xE5,0xAC,0x12,0xED,0xEF,0x50,0xD1,0xB7,0x13,0x4C,0x4B,0xA6,
    0x6D,0x99,0x3E,0x59,0x83,0xAD,0x5E,0x22,0xDF,0x8A,0xB3,0xFD,
    0x06,0x51,0xDC,0xAE,0xD7,0x18,0xBE,0x26,0x6C,0x91,0xF3,0x24,
    0x22,0x27,0x63,0x79,0x04,0x2F,0x44,0x85,0x30,0xF1,0x32,0x96,
    0x65,0x5F,0x43,0x00,0x7B,0x7E,0x72,0x3F,0xF8,0x8F,0x56,0xFA,
    0xFE,0xA2,0x2B,0xBA,0x93,0x13,0x4C,0x84,0xB4,0x0F,0x97,0x80,
    0xFE,0xBE,0xC8,0xE8,0xB0,0x0E,0x96,0xA8,0xEA,0x76,0xD8,0x38,
    0x01,0x49,0xF1,0x7B,0xAE,0xC4,0x05,0xF3,0xC2,0x70,0xBC,0x01,
    0x8C,0xD6,0x19,0x8F,0xF5,0x29,0x6A,0xE9,0x02,0x15,0x22,0x10,
    0x7E,0x46,0x9D,0xD2,0x6F,0xED,0xCE,0xDC,0xE1,0xA1,0x35,0x2A,
    0xD8,0x77,0x02,0x22,0xCC,0xA0,0x68,0x46,0x28,0xA5,0x3D,0xE6,
    0xF2,0x95,0x31,0xE6,0x27,0xB0,0xB2,0xEA,0x62,0x66,0xBB,0xCD,
    0xB7,0xAA,0xDE,0xB8,0x52,0x70,0x7A,0x74,0x0E,0x11,0x26,0x0A,
    0x27,0x07,0x49,0x8C,0xE5,0xF3,0xED,0xCD,0x98,0xB6,0xE2,0x96,
    0x8E,0xCF,0x96,0x4A,0x1D,0xB9,0x50,0x17,0x48,0x16,0x2D,0x53,
    0x3E,0x39,0xA4,0x53,
  };
  static unsigned char dh2048_g[]={
    0x02,
  };

  BIGNUM* p = BN_bin2bn(dh2048_p, sizeof(dh2048_p), 0);
  BIGNUM* g = BN_bin2bn(dh2048_g, sizeof(dh2048_g), 0);
  
  if (!p || !g) {
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
  }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
  dh->p = p;
  dh->g = g;
#else
  DH_set0_pqg(dh, p, 0, g);
#endif
  
  SSL_CTX_set_tmp_dh(pd_ctx, dh);
  DH_free(dh);
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_ephemeralRSA() {

  // Default implementation does nothing. To support low-grade
  // ephemeral RSA key exchange, use a subclass with code like the
  // following:

#if 0
  RSA *rsa;

  rsa = RSA_generate_key(512,RSA_F4,NULL,NULL);

  if (!SSL_CTX_set_tmp_rsa(pd_ctx,rsa)) {
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
  }
  RSA_free(rsa);
#endif
}


/////////////////////////////////////////////////////////////////////////
int
sslContext::set_verify_mode() {
  return sslContext::verify_mode;
}


/////////////////////////////////////////////////////////////////////////
static omni_tracedmutex *openssl_locks = 0;

extern "C" 
void sslContext_locking_callback(int mode, int type, const char *,int) { 
  
  if (mode & CRYPTO_LOCK) {
    openssl_locks[type].lock();
  }
  else {
    OMNIORB_ASSERT(mode & CRYPTO_UNLOCK);
    openssl_locks[type].unlock();
  }
}

/////////////////////////////////////////////////////////////////////////
#ifndef __WIN32__
extern "C"
unsigned long sslContext_thread_id(void) {
  unsigned long id = (unsigned long)pthread_self();
  return id;
}
#endif

/////////////////////////////////////////////////////////////////////////
void
sslContext::thread_setup() {
  pd_locks = new omni_tracedmutex[CRYPTO_num_locks()];
  openssl_locks = pd_locks;
  CRYPTO_set_locking_callback(sslContext_locking_callback);
#ifndef __WIN32__
  CRYPTO_set_id_callback(sslContext_thread_id);
#endif
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::thread_cleanup() {
  CRYPTO_set_locking_callback(NULL);
#ifndef __WIN32__
  CRYPTO_set_id_callback(NULL);
#endif
  if (pd_locks) {
    delete [] pd_locks;
    openssl_locks = 0;
  }
}

/////////////////////////////////////////////////////////////////////////
static void report_error() {

  if (omniORB::trace(1)) {
    char buf[128];
    ERR_error_string_n(ERR_get_error(),buf,128);
    omniORB::logger log;
    log << "OpenSSL: " << (const char*) buf << "\n";
  }
}
