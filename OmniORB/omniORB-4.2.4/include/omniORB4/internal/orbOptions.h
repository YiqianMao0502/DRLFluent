// -*- Mode: C++; -*-
//                            Package   : omniORB
// orbOptions.h               Created on: 13/8/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2013 Apasphere Ltd
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

#ifndef __ORBOPTIONS_H__
#define __ORBOPTIONS_H__

#include <omniORB4/CORBA.h>

OMNI_NAMESPACE_BEGIN(omni)

class orbOptions;

class orbOptions {
 public:

  ////////////////////////////////////////////////////////////////////////
  typedef CORBA::StringSeq     sequenceString;
  typedef CORBA::StringSeq_var sequenceString_var;

  ////////////////////////////////////////////////////////////////////////
  class BadParam {
  public:
    BadParam(const char* k, const char* v, const char* y) :
      key(k),value(v),why(y) {}

    CORBA::String_var key;
    CORBA::String_var value;
    CORBA::String_var why;

    BadParam(const BadParam& b) : key(b.key), value(b.value), why(b.why) {}

  private:
    BadParam();
    BadParam& operator=(const BadParam&);
  };


  ////////////////////////////////////////////////////////////////////////
  class Unknown {
  public:
    Unknown(const char* k, const char* v) : key(k),value(v) {}

    CORBA::String_var key;
    CORBA::String_var value;

    Unknown(const Unknown& k) : key(k.key), value(k.value) {}

  private:
    Unknown();
    Unknown& operator=(const Unknown&);
  };

  ////////////////////////////////////////////////////////////////////////
  enum Source { fromFile, fromEnvironment, fromRegistry, fromArgv, 
		fromArray, fromInternal };


  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  class Handler {
  public:

    const char* key() const { return key_; }
    const char* usage() const { return usage_; }
    const char* usageArgv() const { return argvUsage_; }
    CORBA::Boolean argvYes() const { return argvYes_; }
    CORBA::Boolean argvHasNoValue() const { return argvHasNoValue_; }

    virtual void visit(const char* value,Source source)
      OMNI_THROW_SPEC (BadParam) = 0;
    virtual void dump(sequenceString& result) = 0;

  protected:
    Handler(const char* k, const char* u,
	    CORBA::Boolean yesorno, const char* arg_u,
	    CORBA::Boolean novalue = 0) :
      key_(k), usage_(u),
      argvYes_(yesorno), argvUsage_(arg_u), argvHasNoValue_(novalue) {}

    virtual ~Handler() {}

  private:

    const char*          key_;
    const char*          usage_;
    const CORBA::Boolean argvYes_;
    const char*          argvUsage_;
    const CORBA::Boolean argvHasNoValue_;

    Handler();
    Handler(const Handler&);
    Handler& operator=(const Handler&);
  };


  void registerHandler(Handler& h);
  // Register a handler for an option. The option's name is identified by
  // h->key().
  //
  // When subsequently this->visit() is called, h->visit() will be called
  // for each of the options that matches the option's name.
  //
  // h->argvYes() if true indicates that this option can be specified in
  // the ORB_init argument list. When this->extractInitOptions() is called,
  // the arguments will be searched and those that match the key of the
  // handler will be extracted.
  //
  // Any modules that have configuration options must call this method before
  // ORB_init is called. Typically this is done inside the ctor of the
  // initialiser singleton.
  //
  // Thread Safety preconditions:
  //    Not thread safe


  ////////////////////////////////////////////////////////////////////////
  void reset();
  // Remove any options that have been added previously with addOption() or
  // addOptions().

  ////////////////////////////////////////////////////////////////////////
  void visit() OMNI_THROW_SPEC(BadParam);
  // Call this method will cause the object to walk through all the options
  // accumulated so far via addOption(). For each of these options, its
  // handler will be called.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  void addOption(const char* key, const char* value, 
		 Source source=fromInternal) OMNI_THROW_SPEC (Unknown,BadParam);
  // Add to the internal option list a <key,value> tuple.
  // Both arguments are copied.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  void addOptions(const char* options[][2]) OMNI_THROW_SPEC (Unknown,BadParam);
  // Add the option list. Each element of the variable size array is
  // a key, value pair. The array ends with a key, value pair that is both
  // nil(0) in value.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  void extractInitOptions(int& argc, char** argv)
    OMNI_THROW_SPEC (Unknown,BadParam);
  // Extract the ORB_init options from the argv list. Extract the arguments
  // from the argument list for those registered handlers that can accept
  // ORB_init arguments.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  void getTraceLevel(int argc, char** argv) OMNI_THROW_SPEC (Unknown,BadParam);
  // Look for -ORBtraceLevel and -ORBtraceFile arguments very early
  // on, so the trace level can affect later option logging. Does not
  // remove the arguments -- that is done by extractInitOptions()
  // later.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  const char* getConfigFileName(int argc, char** argv, const char* fname)
    OMNI_THROW_SPEC (Unknown,BadParam);
  // Look for an -ORBconfigFile argument before processing the config
  // file. Does not remove the arguments -- that is done by
  // extractInitOptions() later.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  CORBA::Boolean importFromFile(const char* filename)
    OMNI_THROW_SPEC (Unknown,BadParam);

#if defined(NTArchitecture) && !defined(__ETS_KERNEL__)
  ////////////////////////////////////////////////////////////////////////
  CORBA::Boolean importFromRegistry() OMNI_THROW_SPEC (Unknown,BadParam);
#endif

  ////////////////////////////////////////////////////////////////////////
  void importFromEnv() OMNI_THROW_SPEC (Unknown,BadParam);

  ////////////////////////////////////////////////////////////////////////
  sequenceString* usage() const;
  // Return the list of recognised options and their usage string.
  //
  // Caller is responsible for freeing the memory allocated to the
  // returned list.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  sequenceString* usageArgv() const;
  // Return the list of recognised options that can be specified as the
  // ORB_Init options.
  //
  // Caller is responsible for freeing the memory allocated to the
  // returned list.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  sequenceString* dumpSpecified() const;
  // Return the list of options entered by addOption() so far.
  //
  // Caller is responsible for freeing the memory allocated to the
  // returned list.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  sequenceString* dumpCurrentSet() const;
  // Return the list of available options and their current value.
  //
  // Caller is responsible for freeing the memory allocated to the
  // returned list.
  //
  // Thread Safety preconditions:
  //    Not thread safe


  ////////////////////////////////////////////////////////////////////////
  static orbOptions& singleton();
  // Returns the singleton orbOptions object. It is safe to call this
  // function in static initialisers. Typcially usage is to call
  // singleton()->registerHandler() in the ctor of initialiser singletons.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  // Helper functions for Handler classes to use.
  //    get* functions return True(1) if the value string can be parsed
  //                   correctly.
  //    addKV* functions append to the sequenceString a stringified value
  //                     of the key value pair.
  //
  static CORBA::Boolean getBoolean(const char* value, CORBA::Boolean& result);
  static CORBA::Boolean getULong(const char* value, CORBA::ULong& result);
  static CORBA::Boolean getLong(const char* value, CORBA::Long& result);
  static void addKVBoolean(const char* key, CORBA::Boolean,sequenceString&);
  static void addKVULong(const char* key, CORBA::ULong,sequenceString&);
  static void addKVLong(const char* key, CORBA::Long,sequenceString&);
  static void addKVString(const char* key, const char* value, sequenceString&);

  static void move_args(int& argc,char **argv,int idx,int nargs);
  // Move the arguments at argv[idx--(idx+nargs-1)] to the end of
  // argv. Update argc to truncate the moved arguments from argv.

  static const char* expect_boolean_msg;
  static const char* expect_ulong_msg;
  static const char* expect_greater_than_zero_ulong_msg;

#ifdef __GNUG__
  friend class _keep_gcc_quiet_;
#endif

  friend class omni_orbOptions_initialiser;

 private:

  omnivector<Handler*> pd_handlers;
  CORBA::Boolean       pd_handlers_sorted;
  Handler* findHandler(const char* k);
  void     sortHandlers();

  struct HandlerValuePair {

    HandlerValuePair(Handler* h, const char* v, Source s) :
      handler_(h),value_(v),source_(s) {}

    Handler*               handler_;
    CORBA::String_var      value_;
    Source                 source_;
  };
  omnivector<HandlerValuePair*>   pd_values;

  orbOptions();
  ~orbOptions();

  orbOptions(const orbOptions&);
  orbOptions& operator=(const orbOptions&);
};

OMNI_NAMESPACE_END(omni)

#endif // __ORBOPTIONS_H__
