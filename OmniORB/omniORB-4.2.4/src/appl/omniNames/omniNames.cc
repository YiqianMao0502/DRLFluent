// -*- Mode: C++; -*-
//                          Package   : omniNames
// omniNames.cc             Author    : Tristan Richardson (tjr)
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <omnithread.h>
#include <NamingContext_i.h>

#include "omniNames.h"


#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif

#ifdef __WIN32__
#  include <io.h>
#else
#  include <unistd.h>
#endif

#ifndef O_SYNC
#  ifdef  O_FSYNC              // FreeBSD 3.2 does not have O_SYNC???
#    define O_SYNC O_FSYNC
#  endif
#endif

// Minimum idle period before we take a checkpoint (15 mins)
#define DEFAULT_IDLE_TIME_BTW_CHKPT  (15*60)


PortableServer::POA_var names_poa;

void
usage()
{
  cerr << "\nusage: omniNames [-start [<port>]]\n"
#ifdef __WIN32__
       <<   "                 [-install [<port>]\n"
       <<   "                 [-manual]\n"
       <<   "                 [-remove]\n"
#endif
       <<   "                 [-always]\n"
       <<   "                 [-datadir <directory name>]\n"
       <<   "                 [-nohostname]\n"
       <<   "                 [-errlog <file name>]\n"
       <<   "                 [-help]\n"
       <<   "                 [<omniORB-options>...]" << endl
       << "\nUse -start option to start omniNames for the first time.\n"
       << "With no <port> argument, the standard default of "
       << IIOP::DEFAULT_CORBALOC_PORT << " is used.\n"
#ifdef __WIN32__
       << "\nUse -install option to install omniNames as a Windows service.\n"
       << "Use -remove to remove the omniNames Windows service.\n"
       << "Use -manual to request manual service start/stop rather than automatic.\n"
#endif
       << "\nUse -always in conjunction with -start to always start omniNames, regardless\n"
       << "of whether the data files already exist.\n"
       << "\nUse -datadir option to specify the directory where the data files are kept.\n"
       << "You can also set the environment variable " << DATADIR_ENV_VAR
       << " to specify the\ndirectory where the data files are kept.\n"
       << "\nUse -nohostname to suppress the inclusion of the hostname in the log files.\n"
       << "\nUse -errlog option to specify where error/debug output is redirected.\n"
       << "\nTo publish a specific IP address to clients, use\n"
       << " -ORBendPointPublish giop:tcp:<address>:\n"
       << "\nFor a list of omniORB options run with -ORBhelp\n";
}


//
// main
//

int
main(int argc, char **argv)
{
  int            port       = 0;
  const char* 	 logdir     = 0;
  const char* 	 errlog     = 0;
  CORBA::Boolean ignoreport = 0;
  CORBA::Boolean nohostname = 0;
  CORBA::Boolean always     = 0;

#ifdef __WIN32__
  CORBA::Boolean install    = 0;
  CORBA::Boolean remove     = 0;
  CORBA::Boolean manual     = 0;
  CORBA::Boolean runsvc     = 0;
#endif

  int    new_argc = 1;
  char** new_argv = new char*[argc];

  new_argv[0] = argv[0];

  // Process command line arguments
  for (int arg = 1; arg < argc; ++arg) {

    if (strcmp(argv[arg], "-start") == 0) {
      if (arg + 1 == argc || argv[arg+1][0] == '-') {
	port = IIOP::DEFAULT_CORBALOC_PORT;
      }
      else {
	port = atoi(argv[++arg]);
      }
    }
#ifdef __WIN32__
    else if (strcmp(argv[arg], "-install") == 0) {
      install = 1;
      if (arg + 1 == argc || argv[arg+1][0] == '-') {
	port = IIOP::DEFAULT_CORBALOC_PORT;
      }
      else {
	port = atoi(argv[++arg]);
      }
    }
    else if (strcmp(argv[arg], "-remove") == 0) {
      remove = 1;
    }
    else if (strcmp(argv[arg], "-manual") == 0) {
      manual = 1;
    }
    else if (strcmp(argv[arg], "-runsvc") == 0) {
      runsvc = 1;
    }
#endif
    else if (strcmp(argv[arg], "-always") == 0) {
      always = 1;
    }
    else if (strcmp(argv[arg], "-ignoreport") == 0) {
      ignoreport = 1;
    }
    else if (strcmp(argv[arg], "-nohostname") == 0) {
      nohostname = 1;
    }
    else if (strcmp(argv[arg], "-datadir") == 0 ||
             strcmp(argv[arg], "-logdir") == 0) {
      if (arg + 1 == argc) {
	usage();
	exit(1);
      }
      logdir = argv[++arg];
    }
    else if (strcmp(argv[arg], "-errlog") == 0) {
      if (arg + 1 == argc) {
	usage();
	exit(1);
      }
      errlog = argv[++arg];
    }
    else if (strcmp(argv[arg], "-help") == 0 ||
             strcmp(argv[arg], "--help") == 0) {
      usage();
      cerr << endl << endl;
      exit(0);
    }
    else if (strcmp(argv[arg], "-ORBhelp") == 0) {
      usage();
      cerr << endl << endl;
      new_argv[new_argc++] = (char*)"-ORBhelp";
      CORBA::ORB_var orb = CORBA::ORB_init(new_argc, new_argv);
      orb->destroy();
      exit(0);
    }
    else if ((strncmp(argv[arg], "-ORB", 4) != 0)) {
      usage();
      exit(1);
    }
    else {
      if (strcmp(argv[arg], "-ORBendPoint") == 0) {
        if (!ignoreport) {
          ignoreport = 1;
          LOG(1, "-ORBendPoint option overriding default endpoint.");
        }
      }
      new_argv[new_argc++] = argv[arg];
      if (arg+1 < argc)
	new_argv[new_argc++] = argv[++arg];
    }
  }

#ifdef __WIN32__
  if (install) {
    return omniNames::installService(port, logdir, errlog, ignoreport,
				     nohostname, manual, new_argc, new_argv);
  }
  else if (remove) {
    return omniNames::removeService();
  }
  else if (runsvc) {
    return omniNames::runService(port, logdir, errlog, ignoreport,
				 nohostname, new_argc, new_argv);
  }
#endif

  try {
    omniNames names(port, logdir, errlog, ignoreport, nohostname, always,
		    new_argc, new_argv);
    delete [] new_argv;
    names.run();
  }
  catch (const CORBA::INITIALIZE& ex) {
    // omniNames constructor has logged a suitable error.
    exit(1);
  }
  return 0;
}


omniNames::
omniNames(int            port,
	  const char*    logdir,
	  const char*    errlog,
	  CORBA::Boolean ignoreport,
	  CORBA::Boolean nohostname,
	  CORBA::Boolean always,
	  int argc, char** argv)

  : cond_(&mu_), stop_(0), running_(0)
{
  // Redirect log
  if (errlog) {
    try {
      omniORB::setLogFilename(errlog);
    }
    catch (CORBA::INITIALIZE&) {
      cerr << "Cannot open error log file: " << errlog << endl;
      exit(1);
    }
  }

  // Set up an instance of omniNameslog.  This also gives us back the
  // port number from the log file if "-start" wasn't specified.
  log_ = new omniNameslog(port, logdir, nohostname, always);

  // Build argument list for ORB_init()
  int new_argc = 1;
  char** new_argv = new char*[argc+4];

  new_argv[0] = argv[0];

  CORBA::String_var endpoint;

  if (!ignoreport) {
    endpoint = CORBA::string_alloc(20);
    sprintf((char*)endpoint, "giop:tcp::%d", port);
    new_argv[new_argc++] = (char*)"-ORBendPoint";
    new_argv[new_argc++] = (char*)endpoint;
  }
  new_argv[new_argc++] = (char*)"-ORBpoaUniquePersistentSystemIds";
  new_argv[new_argc++] = (char*)"1";

  for (int arg=1; arg < argc; ++arg)
    new_argv[new_argc++] = argv[arg];

  // Initialise ORB
  try {
    orb_ = CORBA::ORB_init(new_argc, new_argv);

    delete [] new_argv;

    PortableServer::POA_var root_poa;
    PortableServer::POA_var ins_poa;

    // Root POA
    CORBA::Object_var poaref = orb_->resolve_initial_references("RootPOA");
    root_poa = PortableServer::POA::_narrow(poaref);

    PortableServer::POAManager_var pman = root_poa->the_POAManager();

    CORBA::PolicyList pl(1);
    pl.length(1);
    pl[0] = root_poa->create_lifespan_policy(PortableServer::PERSISTENT);

    // Main naming context POA
    names_poa = root_poa->create_POA("", pman, pl);
    pman->activate();

    // Get the interoperable naming service POA
    poaref  = orb_->resolve_initial_references("omniINSPOA");
    ins_poa = PortableServer::POA::_narrow(poaref);
    pman    = ins_poa->the_POAManager();
    pman->activate();

    // Read the log file and set up all the naming contexts described in it.
    log_->init(orb_, names_poa, ins_poa);
  }
  catch (const CORBA::INITIALIZE& ex) {
    {
      omniORB::logger log("omniNames: ");
      log << "Failed to initialise: " << ex.NP_minorString()
          << "\nomniNames may already be running, or omniORB may be "
             "misconfigured.\n";
    }
    throw;
  }
}

void
omniNames::
run()
{
  int idle_time_btw_chkpt;
  char *itbc = getenv("OMNINAMES_ITBC");
  if (itbc == NULL || sscanf(itbc,"%d",&idle_time_btw_chkpt) != 1)
    idle_time_btw_chkpt = DEFAULT_IDLE_TIME_BTW_CHKPT;

  {
    omni_mutex_lock sync(mu_);

    // Signal that the naming service is running.
    running_ = 1;
    cond_.signal();

    do {
      log_->checkpoint();
      unsigned long s, n;
      omni_thread::get_time(&s, &n, idle_time_btw_chkpt);
      cond_.timedwait(s,n);
    } while (!stop_);
  }
  omniORB::logs(1, "omniNames shutting down.");
}

omniNames::
~omniNames()
{
  delete log_;
  orb_->destroy();
}

void
omniNames::
stop()
{
  omni_mutex_lock sync(mu_);
  stop_ = 1;
  cond_.signal();
}

CORBA::Boolean
omniNames::
waitForStart(int timeout)
{
  omni_mutex_lock sync(mu_);

  if (timeout) {
    unsigned long s, ns;
    omni_thread::get_time(&s, &ns, timeout);
    while (!running_) {
      if (!cond_.timedwait(s, ns))
	return 0;
    }
  }
  else {
    while (!running_) {
      cond_.wait();
    }
  }
  return 1;
}
