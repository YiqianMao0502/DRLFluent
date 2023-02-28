// -*- Mode: C++; -*-
//                            Package   : omniORB
// logIOstream.cc             Created on: 31/3/1998
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2012 Apasphere Ltd
//    Copyright (C) 1998-1999 AT&T Laboratories Cambridge
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

// Macros to handle std namespace and streams header files

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <objectTable.h>
#include <remoteIdentity.h>
#include <inProcessIdentity.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////
/////////////////////////// omniORB::logger //////////////////////////
//////////////////////////////////////////////////////////////////////

#ifdef INIT_BUF_SIZE
#undef INIT_BUF_SIZE
#endif
#define INIT_BUF_SIZE  256

#define PREFIX "omniORB: "

static FILE*             logfile = stderr;
static CORBA::String_var logfilename;


OMNI_USING_NAMESPACE(omni)


static inline omniORB::logFunction& logfunc()
{
  static omniORB::logFunction f = 0;
  return f;
}

void
omniORB::setLogFunction(omniORB::logFunction f)
{
  logfunc() = f;
}

void
omniORB::setLogFilename(const char* n)
{
  const char* existing = (const char*)logfilename;
  if (existing && !strcmp(existing, n)) {
    // Already using this file
    return;
  }
  FILE* f = fopen(n, "a");
  if (!f) {
    // Output an error to the existing logger
    if (omniORB::trace(1)) {
      omniORB::logger l;
      l << "Unable to open log file '" << n << "'.\n";
    }
    OMNIORB_THROW(INITIALIZE, INITIALIZE_CannotOpenLogFile,
		  CORBA::COMPLETED_NO);
  }
  if ((const char*)logfilename) {
    // Close existing file
    fclose(logfile);
  }
  logfile = f;
  logfilename = n;
}

const char*
omniORB::getLogFilename()
{
  return (const char*)logfilename;
}
  

omniORB::logger::logger(const char* prefix)
  : pd_prefix(prefix), pd_buf(new char[INIT_BUF_SIZE])
{
  if( !pd_prefix )  pd_prefix = PREFIX;

  strcpy(pd_buf, pd_prefix);
  pd_p = pd_buf + strlen(pd_prefix);
  pd_end = pd_buf + INIT_BUF_SIZE;

  if (omniORB::traceThreadId) {
    omni_thread* self = omni_thread::self();
    if (self)
      *this << "(" << self->id() << ") ";
    else
      *this << "(?) ";
  }

#if defined(HAVE_STRFTIME) && defined(HAVE_LOCALTIME)
  if (omniORB::traceTime) {
    char tbuf[40];
    unsigned long s, ns;
    omni_thread::get_time(&s, &ns);
    time_t ts = s;
    strftime(tbuf, 39, "%Y-%m-%d %H:%M:%S", localtime(&ts));
    *this << tbuf;
    sprintf(tbuf, ".%06d: ", (int)ns / 1000);
    *this << tbuf;
  }
#endif
}


omniORB::logger::~logger()
{
  if( (size_t)(pd_p - pd_buf) != strlen(pd_prefix) ) {
    omniORB::logFunction f = logfunc();
    if (f)
      f(pd_buf);
    else {
      fputs(pd_buf, logfile ? logfile : stderr);
      if ((const char*)logfilename)
	fflush(logfile);
    }
  }
  delete[] pd_buf;
}


omniORB::logger& 
omniORB::logger::operator<<(char c)
{
  reserve(1);
  *pd_p++ = c;
  *pd_p = '\0';
  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(const char *s)
{
  if (!s) s = "(null)";
  size_t len = strlen(s);
  reserve(len);
  strcpy(pd_p, s);
  pd_p += len;
  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(const void *p)
{
  reserve(30); // guess!
  sprintf(pd_p, "%p", p);
  pd_p += strlen(pd_p);
  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(int n)
{
  reserve(20);
  sprintf(pd_p, "%d", n);
  pd_p += strlen(pd_p);
  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(unsigned int n)
{
  reserve(20);
  sprintf(pd_p, "%u", n);
  pd_p += strlen(pd_p);
  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(long n)
{
  reserve(30);
  sprintf(pd_p, "%ld", n);
  pd_p += strlen(pd_p);
  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(unsigned long n)
{
  reserve(30);
  sprintf(pd_p, "%lu", n);
  pd_p += strlen(pd_p);
  return *this;
}


#ifndef NO_FLOAT
omniORB::logger&
omniORB::logger::operator<<(double n)
{
  reserve(30);
  sprintf(pd_p, "%g", n);
  pd_p += strlen(pd_p);
  return *this;
}
#endif


static void pp_key(omniORB::logger& l, const CORBA::Octet*, int);


omniORB::logger&
omniORB::logger::operator<<(const omniLocalIdentity* id)
{
  OMNIORB_ASSERT(id);

  pp_key(*this, id->key(), id->keysize());

  omniObjTableEntry* entry=omniObjTableEntry::downcast((omniLocalIdentity*)id);
  if (entry) {
    switch (entry->state()) {
    case omniObjTableEntry::ACTIVATING:    *this << " (activating)";     break;
    case omniObjTableEntry::ACTIVE:        *this << " (active)";         break;
    case omniObjTableEntry::DEACTIVATING:  *this << " (deactivating)";   break;
    case omniObjTableEntry::DEACTIVATING_OA:
                                           *this << " (deactivating OA)";break;
    case omniObjTableEntry::ETHEREALISING: *this << " (etherealising)";  break;
    case omniObjTableEntry::DEAD:          *this << " (dead)";           break;
    default:                               *this << " (???" ")";
    }
  }
  else
    *this << " (temp)";

  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(const omniIdentity* id)
{
  OMNIORB_ASSERT(id);

  pp_key(*this, id->key(), id->keysize());

  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(omniObjKey& k)
{
  pp_key(*this, k.key(), k.size());

  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(const omniORB::logger::exceptionStatus& ex)
{
  switch (ex.status) {
  case CORBA::COMPLETED_YES:
    *this << "YES,";
    break;
  case CORBA::COMPLETED_NO:
    *this << "NO,";
    break;
  case CORBA::COMPLETED_MAYBE:
    *this << "MAYBE,";
    break;
  }
  if (ex.minor_string) {
    *this << ex.minor_string;
  }
  else {
    reserve(30);
    sprintf(pd_p, "0x%08x", (int)ex.minor);
    pd_p += strlen(pd_p);
  }
  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(const CORBA::SystemException& ex)
{
  *this << ex._name() << "(";
  switch (ex.completed()) {
  case CORBA::COMPLETED_YES:
    *this << "YES,";
    break;
  case CORBA::COMPLETED_NO:
    *this << "NO,";
    break;
  case CORBA::COMPLETED_MAYBE:
    *this << "MAYBE,";
    break;
  }
  const char* minor = ex.NP_minorString();
  if (minor) {
    *this << minor;
  }
  else {
    reserve(30);
    sprintf(pd_p, "0x%08x", (int)ex.minor());
    pd_p += strlen(pd_p);
  }
  *this << ")";

  return *this;
}

void
omniORB::logger::flush()
{
  if( (size_t)(pd_p - pd_buf) != strlen(pd_prefix) ) {
    omniORB::logFunction f = logfunc();
    if (f)
      f(pd_buf);
    else
      fprintf(logfile ? logfile : stderr, "%s", pd_buf);
  }
  pd_p = pd_buf + strlen(pd_prefix);
  *pd_p = '\0';
}


void
omniORB::logger::more(int n)
{
  int used = pd_p - pd_buf + 1;
  int size = pd_end - pd_buf;

  while( size - used < n )  size *= 2;

  char* newbuf = new char[size];
  strcpy(newbuf, pd_buf);
  char* newp = newbuf + (used - 1);
  delete[] pd_buf;
  pd_buf = newbuf;
  pd_p = newp;
  pd_end = pd_buf + size;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#ifdef INLINE_BUF_SIZE
#undef INLINE_BUF_SIZE
#endif
#define INLINE_BUF_SIZE  256


void
omniORB::do_logs(const char* mesg)
{
  char inlinebuf[INLINE_BUF_SIZE];
  char* buf = inlinebuf;
  size_t fmtlen = strlen(mesg) + sizeof(PREFIX) + 15;

#if defined(HAVE_STRFTIME) && defined(HAVE_LOCALTIME)
  if (traceTime)
    fmtlen += 30;
#endif

  if (fmtlen > INLINE_BUF_SIZE)
    buf = new char[fmtlen];

  strcpy(buf, PREFIX);
  char* cbuf = buf + sizeof(PREFIX) - 1;

  if (traceThreadId) {
    omni_thread* self = omni_thread::self();
    if (self)
      cbuf += sprintf(cbuf, "(%d) ", self->id());
    else
      cbuf += sprintf(cbuf, "(?) ");
  }

#if defined(HAVE_STRFTIME) && defined(HAVE_LOCALTIME)
  if (traceTime) {
    unsigned long s, ns;
    omni_thread::get_time(&s, &ns);
    time_t ts = s;
    cbuf += strftime(cbuf, fmtlen - (cbuf-buf),
		     "%Y-%m-%d %H:%M:%S", localtime(&ts));
    cbuf += sprintf(cbuf, ".%06d: ", (int)ns / 1000);
  }
#endif

  sprintf(cbuf, "%s\n", mesg);

  omniORB::logFunction f = logfunc();
  if (f) {
    f(buf);
  }
  else {
    fputs(buf, logfile ? logfile : stderr);
    if ((const char*)logfilename)
      fflush(logfile);
  }
  if (buf != inlinebuf) delete[] buf;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// These should of course not be here ...
#define POA_NAME_SEP            '\xff'
#define POA_NAME_SEP_STR        "\xff"
#define TRANSIENT_SUFFIX_SEP    '\xfe'
#define TRANSIENT_SUFFIX_SIZE   8

static char cm[] = { '0', '1', '2', '3', '4', '5', '6', '7',
		     '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };


static int is_poa_key(const CORBA::Octet* key, int keysize)
{
  const char* k = (const char*) key;
  const char* kend = k + keysize;

  if( *k != TRANSIENT_SUFFIX_SEP && *k != POA_NAME_SEP )  return 0;

  while( k < kend && *k == POA_NAME_SEP ) {
    k++;
    while( k < kend && *k && *k != POA_NAME_SEP && *k != TRANSIENT_SUFFIX_SEP )
      k++;
  }

  if( k == kend )  return 0;

  if( *k == TRANSIENT_SUFFIX_SEP )
    k += TRANSIENT_SUFFIX_SIZE + 1;
  if( k >= kend || *k )  return 0;

  return 1;
}


static int is_boa_key(const CORBA::Octet* key, int keysize)
{
  return keysize == sizeof(omniOrbBoaKey);
}


static char* pp_poa_key(const CORBA::Octet* key, int keysize)
{
  // output: root/poa/name<key>

  const char* k = (const char*) key;
  const char* kend = k + keysize;

  // We play safe with the size.  It can be slightly bigger than
  // the key because we prefix 'root', and the object id may be
  // pretty printed larger than its octet representation.
  char* ret = new char[keysize * 2 + 20];
  char* s = ret;

  strcpy(s, "root");  s += 4;

  while( k < kend && *k == POA_NAME_SEP ) {
    *s++ = '/';
    k++;

    while( *k && *k != POA_NAME_SEP && *k != TRANSIENT_SUFFIX_SEP )
      *s++ = *k++;
  }

  if( *k == TRANSIENT_SUFFIX_SEP )
    k += TRANSIENT_SUFFIX_SIZE + 1;

  k++;
  *s++ = '<';
  CORBA::ULong idsize = kend - k;
  if (idsize == 12) {
    // Persistent POA key (we hope)

    while (idsize > 4) {
      *s++ = cm[(unsigned char)*k >> 4];
      *s++ = cm[(unsigned char)*k & 0xf];
      k++;
      idsize--;
    }
    *s++ = '/';
  }
  if( idsize == 4 ) {
    CORBA::ULong val = 0;
    while (idsize--)
      val += ((CORBA::ULong)((unsigned char)*k++)) << (idsize * 8);

    sprintf(s, "%lu", (unsigned long) val);
    s += strlen(s);
  }
  else {
    while( idsize-- )  { *s++ = isalnum((unsigned char)*k) ? *k : '.'; k++; }
  }

  *s++ = '>';
  *s++ = '\0';

  return ret;
}



static char* pp_boa_key(const CORBA::Octet* key, int keysize)
{
  // output: boa<key-in-hex>

  int size = 8 + keysize * 2;
  char* ret = new char[size];
  char* s = ret;
  strcpy(s, "boa<0x");
  s += strlen(s);

  const unsigned char* k = (const unsigned char*) key;

  for( int i = 0; i < keysize; i++, k++ ) {
    *s++ = cm[*k >> 4];
    *s++ = cm[*k & 0xf];
  }
  *s++ = '>';
  *s++ = '\0';

  return ret;
}


static char* pp_key(const CORBA::Octet* key, int keysize)
{
  // output: key<keystring>

  int size = 8 + keysize;
  char* ret = new char[size];
  char* s = ret;
  strcpy(s, "key<");
  s += strlen(s);

  const char* k = (const char*) key;

  for( int i = 0; i < keysize; i++, k++ ) {
    *s++ = isalnum((unsigned char)*k) ? *k : '.';
  }
  *s++ = '>';
  *s++ = '\0';

  return ret;
}


static void pp_key(omniORB::logger& l, const CORBA::Octet* key, int keysize)
{
  char* p;

  if( is_poa_key(key, keysize) )
    p = pp_poa_key(key, keysize);
  else if( is_boa_key(key, keysize) )
    p = pp_boa_key(key, keysize);
  else
    p = pp_key(key, keysize);

  l << p;
  delete[] p;
}
