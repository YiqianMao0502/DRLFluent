// -*- Mode: C++; -*-
//                          Package   : omniMapper
// omniMapper.cc            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
//
//  This file is part of omniMapper.
//
//  omniMapper is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published
//  by the Free Software Foundation; either version 2 of the License,
//  or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see http://www.gnu.org/licenses/

#include <omniORB4/CORBA.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#ifdef HAVE_STD
#  include <iostream>
#  include <fstream>
   using namespace std;
#else
#  include <iostream.h>
#  include <fstream.h>
#endif

static void
usage()
{
  cerr << endl
       << "usage: omniMapper [-port <port>]" << endl
       << "                  [-config <config file>]" << endl
       << "                  [-v]" << endl
       << endl
       << "With no -port argument, the standard default of "
       << IIOP::DEFAULT_CORBALOC_PORT << " is used." << endl
       << endl
       << "Use -config to specify the location of omniMapper's config file."
       << endl << endl
       << "Alternatively, set the environment variable " << CONFIG_FILE_ENV_VAR
       << endl
       << "or use the default " << CONFIG_FILE_DEFAULT << "." << endl
       << endl
       << "Use -v to verbosely record what's going on." << endl;
  exit(1);
}


class Mapper :
  public PortableServer::ServantBase
{
public:

  Mapper(char* id, CORBA::Object_ptr obj);
  ~Mapper() {}

  CORBA::Boolean _dispatch(omniCallHandle&) { do_redir(); return 1; }
  CORBA::Boolean _is_a(const char* id) { do_redir(); return 1; }

  void do_redir();

private:
  CORBA::String_var id_;
  CORBA::Object_var obj_;
};


// Globals
CORBA::ORB_var                 orb;
PortableServer::POA_var        inspoa;
int                            verbose;


Mapper::Mapper(char* id, CORBA::Object_ptr obj)
  : id_(id), obj_(obj)
{
  int oidlen = strlen(id);
  PortableServer::ObjectId oid(oidlen, oidlen, (CORBA::Octet*)id, 0);
  inspoa->activate_object_with_id(oid, this);
  if (verbose)
    cout << "Initialising `" << id << "'." << endl;
}


void
Mapper::do_redir()
{
  if (verbose) {
    CORBA::String_var ior = orb->object_to_string(obj_);
    cout << "Mapping `" << (const char*)id_
	 << "' to " << (const char*)ior << endl;
  }
  throw omniORB::LOCATION_FORWARD(CORBA::Object::_duplicate(obj_),0);
}



static char* getString(istream& file) {
  int   bufsz     = 512;
  char* buf       = new char[bufsz];
  char* p         = buf;
  char  c;

  // Skip white space and comments
  do {
    if (!file.get(c)) { // IO problem or EOF
      delete [] buf;
      return 0;
    }
    if (c == '#') {
      do {
	if (!file.get(c)) {
	  delete [] buf;
	  return 0;
	}
      } while (c != '\n');
    }
  } while (isspace(c));

  *p++ = c;

  // Now get the rest of the string
  while (1) {
    if (!file.get(c)) {
      delete [] buf;
      return 0;
    }
    if (isspace(c)) {
      *p = '\0';
      return buf;
    }
    *p++ = c;

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


static int
processConfigFile(const char* configFile)
{
#ifdef __WIN32__
  ifstream cfile(configFile, ios::in);
#else
  ifstream cfile(configFile);
#endif
  if (!cfile) {
    cerr << "Error: cannot open config file `" << configFile << "'." << endl;
    return 0;
  }

  Mapper*           m;
  char*             id;
  CORBA::String_var ior;
  CORBA::Object_ptr obj;
  int               count = 0;

  while (!cfile.eof()) {
    id  = getString(cfile);
    if (!id) break;

    ior = getString(cfile);
    if (!(char*)ior) {
      cerr << "Error: config file ended unexpectedly." << endl;
      exit(1);
    }
    try {
      obj = orb->string_to_object(ior);
    }
    catch (...) {
      cerr << "Error: invalid IOR string `"
	   << (const char*)ior << "'." << endl;
      exit(1);
    }
    m = new Mapper(id, obj);
    m->_remove_ref();
    count++;
  }
  if (count == 0) {
    cerr << "Error: config file had no entries." << endl;
    exit(1);
  }
  return count;
}


static void
insertArgs(int& argc, char**& argv, int idx, int nargs)
{
  char** newArgv = new char*[argc+nargs];
  int i;
  for (i = 0; i < idx; i++) {
    newArgv[i] = argv[i];
  }
  for (i = idx; i < argc; i++) {
    newArgv[i+nargs] = argv[i];
  }
  argv = newArgv;
  argc += nargs;
}


int
main(int argc, char** argv)
{
  int        port        = 0;
  const char *configFile = 0;

  for (int i=1; i < argc; i++) {

    if (!strcmp(argv[i], "-port")) {
      if (argc < i+1) usage();
      port = atoi(argv[++i]);
    }
    else if (!strcmp(argv[i], "-config")) {
      if (argc < i+1) usage();
      configFile = argv[++i];
    }
    else if (!strcmp(argv[i], "-v")) {
      verbose = 1;
    }
  }
  if (port == 0) port = IIOP::DEFAULT_CORBALOC_PORT;

  insertArgs(argc, argv, 1, 2);
  argv[1] = strdup("-ORBendPoint");
  argv[2] = new char[20];
  sprintf(argv[2], "giop:tcp::%d", port);

  try {
    orb = CORBA::ORB_init(argc, argv);
  }
  catch (CORBA::INITIALIZE& ex) {
    cerr << "Failed to initialise the ORB." << endl;
    usage();
    return 1;
  }

  // Get hold of the INS POA and activate it
  try {
    CORBA::Object_var obj = orb->resolve_initial_references("omniINSPOA");
    inspoa                = PortableServer::POA::_narrow(obj);

    PortableServer::POAManager_var pm = inspoa->the_POAManager();
    pm->activate();
  }
  catch (CORBA::INITIALIZE& ex) {
    cerr << "Failed to initialise the POA. "
	 << "Is omniMapper already running?" << endl;
    usage();
    return 1;
  }

  // Figure out config file name
  if (!configFile &&
      (configFile = getenv(CONFIG_FILE_ENV_VAR)) == 0)
    configFile = CONFIG_FILE_DEFAULT;

  if (!processConfigFile(configFile)) {
    usage();
    exit(1);
  }
  if (verbose)
    cout << "omniMapper running." << endl;

  orb->run();

  return 0;
}
