// -*- Mode: C++; -*-
//                          Package   : omniNames
// log.cc                   Author    : Tristan Richardson (tjr)
//
//    Copyright (C) 2002-2013 Apasphere Ltd
//    Copyright (C) 1997-1999 AT&T Laboratories Cambridge
//
//  This file is part of omniNames.
//
//  omniNames is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see http://www.gnu.org/licenses/
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#if defined(__VMS) && __VMS_VER < 70000000
#  include <omniVMS/unlink.hxx>
#  include <omniVms/utsname.hxx>
#endif
#include <NamingContext_i.h>
#include <ObjectBinding.h>
#include <INSMapper.h>
#include <log.h>

#ifdef HAVE_STD
#  include <iostream>
#  include <iomanip>
   using namespace std;
#else
#  include <iostream.h>
#  include <iomanip.h>
#endif

#ifdef __WIN32__
#  include <io.h>
#  include <winbase.h>
#  define stat(x,y) _stat(x,y)
#  define unlink(x) _unlink(x)
#else
#  include <unistd.h>
#  include <sys/utsname.h>
#endif

#if defined(__nextstep__)
#  include <libc.h>
#  include <sys/param.h>
#endif

#ifndef O_SYNC
#  ifdef  O_FSYNC              // FreeBSD 3.2 does not have O_SYNC???
#    define O_SYNC O_FSYNC
#  endif
#endif

#ifdef HAVE_STD
#  define USE_STREAM_OPEN
#  define OPEN(name,mode,perm) open(name,mode)
#elif defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x500
#  define USE_STREAM_OPEN
#  define OPEN(name,mode,perm) open(name,mode,perm)
#elif defined(__DMC__)
#  define USE_STREAM_OPEN
#  define OPEN(name,mode,perm) open(name,mode,perm)
#elif defined(__ICC)
#  define USE_STREAM_OPEN
#  define OPEN(name,mode,perm) open(name,mode,perm)
#endif

#ifndef HAVE_STRDUP

// we have no strdup
static char *
strdup (char* str)
{
  char *newstr;

  newstr = (char *) malloc (strlen (str) + 1);
  if (newstr)
    strcpy (newstr, str);
  return newstr;
}
#endif  // not HAVE_STRDUP


static
inline void reallyFlush(ofstream& f) {
    f.flush();
#ifdef needsExplicitFsync
    ::fsync(f.rdbuf()->fd());
#endif
}

extern void usage();


//
// Constructor for class omniNameslog.  There will normally be only one
// instance of this class.  Unfortunately the initialisation of the class
// cannot be completed in the constructor - the rest is done in the init()
// member function.  This is because the main program cannot give us
// pointers to the ORB and the BOA until it has worked out which port to
// listen on, but it normally finds out the port from the log file.  So in
// this constructor we initialise as much as we can, find out the port from
// the log file if there is one and return.  Then the main program
// initialises the ORB and the BOA and calls init().
//

omniNameslog::omniNameslog(int& p, const char* arg_logdir,
			   int nohostname, int always)
  : port(p)
{
  startingUp = 1;
  checkpointNeeded = 1;
  line = 1;

#ifdef __WIN32__
  struct _stat sb;
#else
  struct stat sb;
#endif

  CORBA::String_var logdir;

  if (!arg_logdir)
    arg_logdir = getenv(DATADIR_ENV_VAR);
  
  if (!arg_logdir)
    arg_logdir = getenv(LOGDIR_ENV_VAR);
  
  if (arg_logdir)
    logdir = CORBA::string_dup(arg_logdir);
  else
    logdir = CORBA::string_dup(DEFAULT_LOGDIR);

  CORBA::String_var logname;

#if !defined(__WIN32__) && !defined(__VMS)
  if (logdir[strlen(logdir)-1] == '/') {
    logdir[strlen(logdir)-1] = '\0';		// strip trailing '/'
  }
#endif

  if (nohostname) {
    logname = CORBA::string_alloc(strlen(logdir) + strlen("/omninames"));
    sprintf(logname, "%s/omninames", (const char*)logdir);
  }
  else {

#if !defined(__WIN32__) && !defined(__VMS)
#  ifdef HAVE_UNAME

    struct utsname un;
    if (uname(&un) < 0) {
      LOG(1, "Error: cannot get the name of this host.");
      exit(1);
    }

    logname = CORBA::string_alloc(strlen(logdir) + strlen("/omninames-") +
				  strlen(un.nodename));
    sprintf(logname, "%s/omninames-%s", (const char*)logdir, un.nodename);

#  elif HAVE_GETHOSTNAME

    // Apparently on some AIX versions, MAXHOSTNAMELEN is too small (32) to
    // reflect the true size a hostname can be. Check and fix the value.

#    ifndef MAXHOSTNAMELEN
#      define MAXHOSTNAMELEN 256
#    elif   MAXHOSTNAMELEN < 64
#      undef  MAXHOSTNAMELEN
#      define MAXHOSTNAMELEN 256
#    endif

    char hostname[MAXHOSTNAMELEN+1];

    if (gethostname(hostname, MAXHOSTNAMELEN) < 0) {
      LOG(1, "Error: cannot get the name of this host.");
      exit(1);
    }
    logname = CORBA::string_alloc(strlen(logdir) + strlen("/omninames-") +
				  strlen(hostname));
    sprintf(logname, "%s/omninames-%s", (const char*)logdir, hostname);

#  endif // HAVE_UNAME

#elif defined(__WIN32__)

    // Get host name:

    DWORD machineName_buflen = MAX_COMPUTERNAME_LENGTH+1;
    CORBA::String_var machineName = CORBA::string_alloc(machineName_buflen-1);
    if (!GetComputerName((LPTSTR)(char*)machineName, &machineName_buflen)) {
      LOG(1, "Error: cannot get the name of this host.");
      exit(1);
    }

    logname = CORBA::string_alloc(strlen(logdir) + strlen("\\omninames-") +
				  + strlen(machineName));
    sprintf(logname, "%s\\omninames-%s",
	    (const char*)logdir, (const char*)machineName);
  
#else // VMS
    char last(logdir[strlen(logdir)-1]);
    if (last != ':' && last != ']') {
      LOG(1, "Error: " << LOGDIR_ENV_VAR << " (" << logdir <<
          ") is not a directory name.");
      exit(1);
    }

    struct utsname un;
    if (uname(&un) < 0) {
      LOG(1, "Error: cannot get the name of this host.");
      exit(1);
    }

    logname = CORBA::string_alloc(strlen(logdir) + strlen("/omninames-") +
				  strlen(un.nodename));
    sprintf(logname, "%somninames-%s", (const char*)logdir, un.nodename);
#endif
  }

#ifndef __VMS
  active_new = CORBA::string_alloc(strlen(logname)+strlen(".dat"));
  sprintf(active_new,"%s.dat", (const char*)logname);

  active_old = CORBA::string_alloc(strlen(logname)+strlen(".log"));
  sprintf(active_old,"%s.log", (const char*)logname);

  backup = CORBA::string_alloc(strlen(logname)+strlen(".bak"));
  sprintf(backup,"%s.bak", (const char*)logname);

  checkpt = CORBA::string_alloc(strlen(logname)+strlen(".ckp"));
  sprintf(checkpt,"%s.ckp", (const char*)logname);
#else
  // specify latest version:
  active_new = CORBA::string_alloc(strlen(logname)+strlen(".dat;"));
  sprintf(active_new,"%s.dat;", (const char*)logname);

  active_old = CORBA::string_alloc(strlen(logname)+strlen(".log;"));
  sprintf(active_old,"%s.log;", (const char*)logname);

  backup = CORBA::string_alloc(strlen(logname)+strlen(".bak;"));
  sprintf(backup,"%s.bak;", (const char*)logname);

  checkpt = CORBA::string_alloc(strlen(logname)+strlen(".ckp;"));
  sprintf(checkpt,"%s.ckp;", (const char*)logname);
#endif

  if (stat(active_old,&sb) == 0) {
    if (stat(active_new,&sb) == 0) {
      LOG(1, "Data file '" << active_new << "' and old log file '" <<
          active_old << "' both exist. Cannot start.");
      exit(1);
    }
    LOG(1, "Using old data file name '" << active_old << "'.");
    active = active_old;
  }
  else {
    active = active_new;
  }

  if (port == 0) {
    firstTime = 0;
  }
  else {
    if (always) {
      if (stat(active,&sb) == 0) {
	// Log file exists -- not first time
	firstTime = 0;
      }
      else {
	firstTime = 1;
      }
    }
    else {
      firstTime = 1;
    }
  }
  if (firstTime) {
    //
    // Starting for the first time - make sure log file doesn't exist, and
    // for safety, that there is no backup file either.
    //
  
    if (stat(active,&sb) == 0) {
      LOG(1, "Error: data file '" << active <<
          "' exists.  Can't use -start option.");
      exit(1);
    }
    if (stat(backup,&sb) == 0) {
      LOG(1, "Error: backup file '" << backup <<
          "' exists.  Can't use -start option.");
      exit(1);
    }

  }
  else {
    //
    // Restart - get port info from log file.
    //

#ifdef __WIN32__
    ifstream initf(active,ios::in);
#else
    ifstream initf(active);
#endif
    if (!initf) {
      LOG(1, "Error: cannot open data file '" << active << "': " <<
          strerror(errno));

      if (stat(backup,&sb) == 0) {
	if (always) {
	  if (unlink(backup) == 0) {
	    LOG(1, "Info: backup file '" << backup << "' removed.");
	  }
	  else {
	    LOG(1, "Backup file '" << backup <<
                "' exists and cannot be removed.");
	    exit(1);
	  }
	}
	else {
	  LOG(1, "Backup file '" << backup <<
              "' exists. Refusing to start. "
              "Remove the backup file to start omniNames.");
	  exit(1);
	}
      }
      else {
        exit(1);
      }
    }

    try {
      getPort(initf);
    }
    catch (IOError&) {
      LOG(1, "Error: reading data file '" << active << "' failed: " <<
          strerror(errno));
      initf.close();
      exit(1);

    }
    catch (ParseError&) {
      LOG(1, "Error: parse error in data file '" << active << "' at line " <<
          line << ".");
      initf.close();
      exit(1);
    }

    p = port;

    initf.close();
  }
}


void
omniNameslog::init(CORBA::ORB_ptr          the_orb,
		   PortableServer::POA_ptr the_poa,
		   PortableServer::POA_ptr the_ins_poa)
{
  orb     = the_orb;
  poa     = the_poa;
  ins_poa = the_ins_poa;

#ifdef __WIN32__
  // This allows the path to contain multi-byte characters.
  setlocale(LC_ALL, "");
#endif

  LOG(1, "Data file: '" << active << "'.");

  if (firstTime) {

    //
    // starting for the first time - create an initial log file with the
    // port specification and the root context.
    //

    LOG(1, "Starting omniNames for the first time.");

    try {
#ifdef USE_STREAM_OPEN
      logf.OPEN(active,ios::out|ios::trunc,0666);
      if (!logf)
	throw IOError();
#else
#  ifdef __WIN32__
      int fd = _open(active, O_WRONLY | O_CREAT | O_TRUNC, _S_IWRITE);
#  else
      int fd = open(active, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0666);
#  endif
      if (fd < 0)
	throw IOError();
      logf.attach(fd);
#endif
      putPort(port, logf);

      // Build persistent identifier
      {
	CORBA::Object_var ref = the_poa->create_reference("");
	PortableServer::ObjectId_var pid = the_poa->reference_to_id(ref);
	persistentId = pid;

	// POA's ids are 12 bytes, the last 4 incrementing with each
	// activated object. We use the first 8 as the persistent
	// identifier.
	persistentId.length(8);
	putPersistent(persistentId, logf);
      }
      reallyFlush(logf);

      {
	PortableServer::ObjectId_var refid =
	  PortableServer::string_to_ObjectId("NameService");

	putCreate(refid, logf);
      }

      logf.close();
      if (!logf)
	throw IOError();

// a bug in sparcworks C++ means that the fd doesn't get closed.
#if defined(__sunos__) && defined(__SUNPRO_CC) && __SUNPRO_CC < 0x500
      if (close(fd) < 0)
	throw IOError();
#endif

    }
    catch (IOError& ex) {
      LOG(1, "Error: cannot create initial data file '" << active << "': " <<
          strerror(errno));
      LOG(1, "You can set the environment variable " << DATADIR_ENV_VAR <<
          " to specify the directory where the data files are kept.");
      logf.close();
      unlink(active);
      exit(1);
    }

    LOG(1, "Wrote initial data file '" << active << "'.");
  }

  // claim the global lock as a writer (ie exclusive).

  NamingContext_i::lock.writerIn();

  ifstream initf(active);

  if (!initf) {
    LOG(1, "Error: cannot open data file '" << active << "'.");
    exit(1);
  }

  try {
    line = 1;

    while (initf && (initf.peek() != EOF)) {

      char* cmd;

      getNonfinalString(cmd, initf);

      if (strcmp(cmd, "port") == 0) {
	while (initf && (initf.get() != '\n'));	// ignore rest of line
	line++;
      }
      else if (strcmp(cmd, "persistent") == 0) {
	getPersistent(initf);
      }
      else if (strcmp(cmd, "create") == 0) {
	getCreate(initf);
      }
      else if (strcmp(cmd, "destroy") == 0) {
	getDestroy(initf);
      }
      else if (strcmp(cmd, "bind") == 0) {
	getBind(initf);
      }
      else if (strcmp(cmd, "unbind") == 0) {
	getUnbind(initf);
      }
      else {
	LOG(1, "Error: unknown command '" << cmd << "' in data file '" <<
            active << "'.");
	throw ParseError();
      }

      delete [] cmd;
    }
    initf.close();
  }
  catch (IOError&) {
    LOG(1, "Error: reading data file '" << active << "' failed: " <<
        strerror(errno));
    initf.close();
    exit(1);

  }
  catch (ParseError&) {
    LOG(1, "Error: parse error in data file '" << active << "' at line " <<
        line << ".");
    initf.close();
  }


  LOG(1, "Read data file '" << active << "' successfully.");

  CosNaming::NamingContext_ptr rootContext
    = NamingContext_i::headContext->_this();

  {
    // Check to see if we need an INS forwarding agent
    omniIOR_var ior;
    ior = rootContext->_getIOR();

    IIOP::ProfileBody iiop;
    const IOP::TaggedProfileList& profiles = ior->iopProfiles();
    for (CORBA::ULong index = 0; index < profiles.length(); index++) {
      if (profiles[index].tag == IOP::TAG_INTERNET_IOP) {
	IIOP::unmarshalProfile(profiles[index],iiop);
	break;
      }
    }

    if (strncmp((const char*)iiop.object_key.get_buffer(),
		"NameService", 11)) {
      LOG(1, "Pre-INS data file.");
      new INSMapper(the_ins_poa, rootContext);
    }
  }

  CORBA::String_var p = orb->object_to_string(rootContext);
  LOG(1, "Root context is " << p);

  // Now use the backdoor to tell the bootstrap agent in this
  // address space to return this root context in response to
  // CORBA::InitialReferences::get("NameService");
  _omni_set_NameService(rootContext);

  CORBA::release(rootContext);	// dispose of the object reference

#ifdef USE_STREAM_OPEN
  logf.OPEN(active,ios::out|ios::app,0666);
  if (!logf) {
    LOG(1, "Error: cannot open data file '" << active << "' for writing.");
    exit(1);
  }
#else
#  ifdef __WIN32__
  int fd = _open(active, O_WRONLY | O_APPEND);
#  else
  int fd = open(active, O_WRONLY | O_APPEND | O_SYNC);
#  endif

  if (fd < 0) {
    LOG(1, "Error: cannot open data file '" << active << "' for writing.");
    exit(1);
  }
  logf.attach(fd);
#endif

  startingUp = 0;

  NamingContext_i::lock.writerOut();

  // remove checkpoint. This will protect us from trouble if the previous
  // incarnation crashed during checkpointing and at the
  // time when both active and checkpoint are linked to the same file.

  unlink(checkpt);
}


void
omniNameslog::create(const PortableServer::ObjectId& id)
{
  if (!startingUp) {
    try {
      putCreate(id, logf);
      reallyFlush(logf);
    }
    catch (IOError& ex) {
      LOG(1, "I/O error writing data file: " << strerror(errno));
      logf.clear();
      throw CORBA::PERSIST_STORE();
    }
    checkpointNeeded = 1;
  }
}

void
omniNameslog::destroy(CosNaming::NamingContext_ptr nc)
{
  if (!startingUp) {
    try {
      putDestroy(nc, logf);
      reallyFlush(logf);
    }
    catch (IOError& ex) {
      LOG(1, "I/O error writing data file: " << strerror(errno));
      logf.clear();
      throw CORBA::PERSIST_STORE();
    }
    checkpointNeeded = 1;
  }
}

void
omniNameslog::bind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n,
                   CORBA::Object_ptr obj, CosNaming::BindingType t)
{
  if (!startingUp) {
    try {
      putBind(nc, n, obj, t, logf);
      reallyFlush(logf);
    }
    catch (IOError& ex) {
      LOG(1, "I/O error writing data file: " << strerror(errno));
      logf.clear();
      throw CORBA::PERSIST_STORE();
    }
    checkpointNeeded = 1;
  }
}

void
omniNameslog::unbind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n)
{
  if (!startingUp) {
    try {
      putUnbind(nc, n, logf);
      reallyFlush(logf);
    }
    catch (IOError& ex) {
      LOG(1, "I/O error writing data file: " << strerror(errno));
      logf.clear();
      throw CORBA::PERSIST_STORE();
    }
    checkpointNeeded = 1;
  }
}


void
omniNameslog::checkpoint()
{
  if (!checkpointNeeded) {
    LOG(5, "No checkpoint needed.");
    return;
  }

  LOG(1, "Checkpointing Phase 1: Prepare.");

  //
  // Get the global lock as a reader.  This means clients will still be able
  // to do "resolve" and "list" operations, but anyone who tries to alter
  // the state in any way will block until we've finished.
  //

  NamingContext_i::lock.readerIn();

  ofstream ckpf;
  int fd = -1;

  try {

#ifdef USE_STREAM_OPEN
    ckpf.OPEN(checkpt, ios::out|ios::trunc, 0666);
    if (!ckpf) {
      LOG(1, "Error: cannot open checkpoint file '" << checkpt <<
          "' for writing.");
      throw IOError();
    }
#else
#  ifdef __WIN32__
    fd = _open(checkpt, O_WRONLY | O_CREAT | O_TRUNC, _S_IWRITE);
#  else
    fd = open(checkpt, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0666);
#  endif

    if (fd < 0) {
      LOG(1, "Error: cannot open checkpoint file '" << checkpt <<
          "' for writing.");
      throw IOError();
    }

    ckpf.attach(fd);
#endif
    putPort(port, ckpf);

    if (persistentId.length())
      putPersistent(persistentId, ckpf);

    NamingContext_i* nci;

    for (nci = NamingContext_i::headContext; nci; nci = nci->next) {
      PortableServer::ObjectId_var id = nci->PR_id();
      putCreate(id, ckpf);
    }

    for (nci = NamingContext_i::headContext; nci; nci = nci->next) {
      for (ObjectBinding* ob = nci->headBinding; ob; ob = ob->next) {
	putBind(nci->_this(), ob->binding.binding_name, ob->object,
		ob->binding.binding_type, ckpf);
      }
    }

    ckpf.close();
    if (!ckpf)
      throw IOError();

// a bug in sparcworks C++ means that the fd doesn't get closed.
#if defined(__sunos__) && defined(__SUNPRO_CC) && __SUNPRO_CC < 0x500
    if (close(fd) < 0)
      throw IOError();
#endif

  }
  catch (IOError& ex) {
    LOG(1, "I/O error writing checkpoint file: " << strerror(errno));
    LOG(1, "Abandoning checkpoint");
    ckpf.close();

// a bug in sparcworks C++ means that the fd doesn't get closed.
#if defined(__sunos__) && defined(__SUNPRO_CC) && __SUNPRO_CC < 0x500
    close(fd);
#endif
    NamingContext_i::lock.readerOut();
    unlink(checkpt);
    return;
  }


  //
  // Now commit the checkpoint to become the active log.
  //

  LOG(1, "Checkpointing Phase 2: Commit.");

// a bug in sparcworks C++ means that the fd doesn't get closed.
#if defined(__sunos__) && defined(__SUNPRO_CC) && __SUNPRO_CC < 0x500
  close(logf.rdbuf()->fd());
#endif

  logf.close();

  unlink(backup);

#if defined(__WIN32__)
  if (!CopyFile(active,backup,TRUE)) {
#elif defined(__VMS)
  if (rename(active, backup) < 0) {
#else
  if (link(active,backup) < 0) {
#endif
    // Failure here leaves old active and checkpoint file.
    LOG(1, "Error: failed to link backup file '" << backup <<
        "' to old data file '" << active << "'.");
    exit(1);
  }

#ifndef __VMS
  if (unlink(active) < 0) {
    // Failure here leaves active and backup pointing to the same (old) file.
    LOG(1, "Error: failed to unlink old data file '" << active << "'.");
    exit(1);
  }
#endif

  active = active_new;

#if defined(__WIN32__)
  if (!CopyFile(checkpt,active,TRUE)) {
#elif defined(__VMS)
  if (rename(checkpt,active) < 0) {
#else
  if (link(checkpt,active) < 0) {
#endif
    // Failure here leaves no active but backup points to the old file.
    LOG(1, "Error: failed to link data file '" << active <<
        "' to checkpoint file '" << checkpt << "'.");
    exit(1);
  }

#ifndef __VMS
  if (unlink(checkpt) < 0) {
    // Failure here leaves active and checkpoint pointing to the same file.
    LOG(1, "Error: failed to unlink checkpoint file '" << checkpt << "'.");
    exit(1);
  }
#endif

#ifdef USE_STREAM_OPEN
  logf.OPEN(active,ios::out|ios::app,0666);
  if (!logf) {
    LOG(1, "Error: cannot open data file '" << active << "' for writing.");
    exit(1);
  }
#else
#  ifdef __WIN32__
  fd = _open(active, O_WRONLY | O_APPEND);
#  else
  fd = open(active, O_WRONLY | O_APPEND | O_SYNC);
#  endif

  if (fd < 0) {
    LOG(1, "Error: cannot open new data file '" << active << "' for writing.");
    exit(1);
  }
  logf.attach(fd);
#endif

  NamingContext_i::lock.readerOut();

  LOG(1, "Checkpointing completed.");
  checkpointNeeded = 0;
}


void
omniNameslog::putPort(int p, ostream& file)
{
  file << "port " << p << '\n';
  if (!file) throw IOError();
}


//
// getPort is different from the other get... functions in that the "command"
// ("port") hasn't been read yet, and also we cannot use any CORBA stuff
// since ORB_init may not have been called yet.
//

void
omniNameslog::getPort(istream& file)
{
  char* str;

  getNonfinalString(str, file);

  if (strcmp(str, "port") != 0) {
    LOG(1, "Error: data file doesn't start with \"port\".");
    throw ParseError();
  }

  delete [] str;

  getFinalString(str, file);

  port = atoi(str);

  delete [] str;

  if (port == 0) {
    LOG(1, "Error: invalid port specified in data file.");
    throw ParseError();
  }
}


void
omniNameslog::putPersistent(const PortableServer::ObjectId& id, ostream& file)
{
  file << "persistent ";
  putKey(id, file);
  file << '\n';
  if (!file) throw IOError();
}


void
omniNameslog::getPersistent(istream& file)
{
  getKey(persistentId, file);
  omniORB::setPersistentServerIdentifier(persistentId);
}


void
omniNameslog::putCreate(const PortableServer::ObjectId& id, ostream& file)
{
  file << "create ";
  putKey(id, file);
  file << '\n';
  if (!file) throw IOError();
}


void
omniNameslog::getCreate(istream& file)
{
  //
  // Argument to "create" is the object key of the naming context.
  //

  PortableServer::ObjectId id;
  NamingContext_i*         rc;

  getKey(id, file);

  if (id.length() == 12) // SYS_ASSIGNED_ID_SIZE + TRANSIENT_SUFFIX_SIZE
    rc = new NamingContext_i(poa, id, this);
  else
    rc = new NamingContext_i(ins_poa, id, this);

  rc->_remove_ref();
}


void
omniNameslog::putDestroy(CosNaming::NamingContext_ptr nc, ostream& file)
{
  file << "destroy ";
  CORBA::String_var s = orb->object_to_string(nc);
  putString(s, file);
  file << '\n';
  if (!file) throw IOError();
}


void
omniNameslog::getDestroy(istream& file)
{
  //
  // Argument to "destroy" is NamingContext IOR.
  //

  char* str;
  getFinalString(str, file);
  CORBA::Object_var o;
  try {
    o = orb->string_to_object(str);
  }
  catch (...) {
    LOG(1, "getDestroy: invalid IOR.");
    delete [] str;
    throw ParseError();
  }

  delete [] str;

  CosNaming::NamingContext_var nc = CosNaming::NamingContext::_narrow(o);
  if (CORBA::is_nil(nc)) {
    LOG(1, "getDestroy: IOR not a NamingContext.");
    throw ParseError();
  }
  nc->destroy();
}


void
omniNameslog::putBind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n,
	     CORBA::Object_ptr obj, CosNaming::BindingType t, ostream& file)
{
  file << "bind ";
  CORBA::String_var s = orb->object_to_string(nc);
  putString(s, file);
  file << ' ';
  putString(n[0].id, file);
  file << ' ';
  putString(n[0].kind, file);
  if (t == CosNaming::nobject)
    file << " nobject ";
  else
    file << " ncontext ";
  s = orb->object_to_string(obj);
  putString(s, file);
  file << '\n';
  if (!file) throw IOError();
}


void
omniNameslog::getBind(istream& file)
{
  //
  // First arg is NamingContext IOR.
  //

  char* str;
  getNonfinalString(str, file);
  CORBA::Object_var o;
  try {
    o = orb->string_to_object(str);
  }
  catch (...) {
    LOG(1, "getBind: invalid IOR.");
    delete [] str;
    throw ParseError();
  }
  delete [] str;

  CosNaming::NamingContext_var nc = CosNaming::NamingContext::_narrow(o);
  if (CORBA::is_nil(nc)) {
    LOG(1, "getBind: IOR not a NamingContext.");
    throw ParseError();
  }

  //
  // 2nd & 3rd args are name id and kind.
  //

  CosNaming::Name name(1);
  name.length(1);

  getNonfinalString(str, file);
  name[0].id = str;

  getNonfinalString(str, file);
  name[0].kind = str;

  //
  // 4th arg is binding type.
  //

  char* bindingType;
  getNonfinalString(bindingType, file);

  //
  // 5th arg is object IOR.
  //

  getFinalString(str, file);
  try {
    o = orb->string_to_object(str);
  }
  catch (...) {
    LOG(1, "getDestroy: invalid IOR.");
    delete [] str;
    throw ParseError();
  }
  delete [] str;

  if (strcmp(bindingType, "ncontext") == 0) {

    CosNaming::NamingContext_var nc2
      = CosNaming::NamingContext::_narrow(o);
    if (CORBA::is_nil(nc2)) {
      LOG(1, "bind: IOR not a NamingContext.");
      throw ParseError();
    }
    nc->rebind_context(name, nc2);
  }
  else {
    nc->rebind(name, o);
  }

  delete [] bindingType;
}



void
omniNameslog::putUnbind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n,
	       ostream& file)
{
  file << "unbind ";
  CORBA::String_var s = orb->object_to_string(nc);
  putString(s, file);
  file << ' ';
  putString(n[0].id, file);
  file << ' ';
  putString(n[0].kind, file);
  file << '\n';
  if (!file) throw IOError();
}


void
omniNameslog::getUnbind(istream& file)
{
  //
  // First arg is NamingContext IOR.
  //

  char* str;
  getNonfinalString(str, file);
  CORBA::Object_var o;
  try {
    o = orb->string_to_object(str);
  }
  catch (...) {
    LOG(1, "getUnbind: invalid IOR.");
    delete [] str;
    throw ParseError();
  }
  delete [] str;

  CosNaming::NamingContext_var nc = CosNaming::NamingContext::_narrow(o);
  if (CORBA::is_nil(nc)) {
    LOG(1, "getUnbind: IOR not a NamingContext.");
    throw ParseError();
  }

  //
  // 2nd & 3rd args are name id and kind.
  //

  CosNaming::Name name(1);
  name.length(1);

  getNonfinalString(str, file);
  name[0].id = str;

  getFinalString(str, file);
  name[0].kind = str;

  nc->unbind(name);
}


void
omniNameslog::putKey(const PortableServer::ObjectId& id, ostream& file)
{
  file << hex;
  for (unsigned int i = 0; i < id.length(); i++) {
    file << setfill('0') << setw(2) << (int)id[i];
  }
  file << dec;
}


void
omniNameslog::getKey(PortableServer::ObjectId& id, istream& file)
{
  char* str;
  getFinalString(str, file);

  int l = strlen(str) / 2;
  id.length(l);
  char* p = str;
  for (int i = 0; i < l; i++) {
    int n;
    sscanf(p,"%02x",&n);
    id[i] = n;
    p += 2;
  }
  delete [] str;
}


void
omniNameslog::putString(const char* str, ostream& file)
{
  for (int i = strlen(str); i > 0; i--, str++) {
    switch (*str) {
      case '\\':
	file.put('\\');
	file.put('\\');
	break;
      case ' ':
	file.put('\\');
	file.put(' ');
	break;
      case '\n':
	file.put('\\');
	file.put('n');
	break;
      case '\t':
	file.put('\\');
	file.put('t');
	break;
      case '\r':
	file.put('\\');
	file.put('r');
	break;
      default:
	file.put(*str);
	break;
    }
  }
}


void
omniNameslog::getFinalString(char*& buf, istream& file)
{
  if (getString(buf, file) != '\n')
    throw ParseError();
  line++;
}

void
omniNameslog::getNonfinalString(char*& buf, istream& file)
{
  if (getString(buf, file) != ' ')
    throw ParseError();
}

int
omniNameslog::getString(char*& buf, istream& file)
{
  int bufsz = 512;
  buf = new char[bufsz];

  char* p = buf;

  int backslash = 0;

  while (1) {

    char c;

    if (!file.get(c)) {	// I/O problem or EOF
      delete [] buf;
      if (!file.eof())
	throw IOError();
      throw ParseError();
    }

    if (backslash) {

      backslash = 0;

      switch (c) {
      case '\\':
	*p++ = '\\';
	break;
      case ' ':
	*p++ = ' ';
	break;
      case 'n':
	*p++ = '\n';
	break;
      case 't':
	*p++ = '\t';
	break;
      case 'r':
	*p++ = '\r';
	break;
      default:
        LOG(1, "Unknown character following '\\' in data file.");
      }
    }
    else {
      switch (c) {
      case '\\':
	backslash = 1;
	break;
      case ' ':
      case '\n':
	*p = '\0';
	return (int)c;
	break;
      default:
	*p++ = c;
      }
    }

    if (p == (buf + bufsz)) {

      // buffer is too small

      char *obuf = buf;
      buf = new char[bufsz+bufsz];
      memcpy(buf, obuf, bufsz);
      delete [] obuf;
      p = buf + bufsz;
      bufsz += bufsz;
    }
  }
}
