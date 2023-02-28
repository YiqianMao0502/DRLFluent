// -*- Mode: C++; -*-
//                            Package   : omniORB
// valueTrackerImpl.h         Created on: 2003/08/05
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2003-2005 Apasphere Ltd.
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
//    Value tracker implementation classes for C++
//

#ifndef __VALUETRACKERIMPL_H__
#define __VALUETRACKERIMPL_H__

#include <omniORB4/CORBA.h>

#ifdef _dyn_attr
# error "A local CPP macro _dyn_attr has already been defined."
#endif

#if defined(_OMNIORB_DYNAMIC_LIBRARY)
#  define _dyn_attr
#else
#  define _dyn_attr _OMNIORB_NTDLL_IMPORT
#endif


OMNI_NAMESPACE_BEGIN(omni)

struct OutputTableEntry;
struct InputTableEntry;


class OutputValueTracker : public ValueIndirectionTracker {
public:
  OutputValueTracker();
  virtual ~OutputValueTracker();

  inline CORBA::Boolean valid()
  {
    return pd_magic == PD_MAGIC;
  }

  CORBA::Long addValue(const CORBA::ValueBase* val,
		       CORBA::Long             current);
  // Look to see if the value has been marshalled before. If so,
  // return its offset; if not, add it to the table and return -1.

  CORBA::Long addRepoId(const char*  repoId,
			CORBA::ULong hashval,
			CORBA::Long  current);
  // As above, for a single repository id.

  CORBA::Long addRepoIds(const _omni_ValueIds* repoIds,
			 CORBA::Long           current);
  // As above, for a list of repository ids.

  inline void startTruncatable() {
    pd_in_truncatable++;
  }
  inline void endTruncatable() {
    pd_in_truncatable--;
  }
  inline CORBA::Boolean inTruncatable() {
    return (pd_in_truncatable > 0);
  }
  // Inside a truncatable value, repoIds of nested values must always
  // be sent, in case the receiver truncates the first instance of the
  // nested value away, but later receives an indirection to it.

private:
  void resizeTable();

  inline void add()
  {
    if (++pd_table_count == pd_table_limit)
      resizeTable();
  }

  static _dyn_attr const CORBA::ULong PD_MAGIC; // "C+OV"
  CORBA::ULong       pd_magic;

  CORBA::ULong       pd_in_truncatable;
  OutputTableEntry** pd_table;
  CORBA::ULong       pd_table_count;
  CORBA::ULong       pd_table_size;
  CORBA::ULong       pd_table_limit;
  CORBA::ULong       pd_table_next_idx;
};

class InputValueTracker : public ValueIndirectionTracker {
public:
  InputValueTracker();
  virtual ~InputValueTracker();

  inline CORBA::Boolean valid()
  {
    return pd_magic == PD_MAGIC;
  }

  void addValue(CORBA::ValueBase* val, CORBA::Long current);
  // Add record of unmarshalled value. Takes ownership of the reference.

  void addRepoId(char* repoId, CORBA::Long current);
  // Add record of unmarshalled repoId. Takes ownership of the string.

  void addRepoIds(_omni_ValueIds* repoIds, CORBA::Long current);
  // Add record of list of unmarshalled repoIds. Takes ownership of
  // the list structure. The strings stored in the list are _not_
  // owned, since they are separately registered with addRepoId.

  CORBA::ValueBase* lookupValue(CORBA::Long pos,
				CORBA::Long current,
				CORBA::CompletionStatus comp);
  // Lookup value at specified position, from the current position.
  // Throw MARSHAL_InvalidIndirection if not found. Caller must
  // add_ref if it needs to keep the value.

  const char* lookupRepoId(CORBA::Long pos,
			   CORBA::Long current,
			   CORBA::CompletionStatus comp);
  // As above for single repoId.

  const _omni_ValueIds* lookupRepoIds(CORBA::Long pos,
				      CORBA::Long current,
				      CORBA::CompletionStatus comp);
  // As above for list of repoIds.

private:
  void resizeTable();

  inline void add()
  {
    if (++pd_table_count == pd_table_limit)
      resizeTable();
  }

  static _dyn_attr const CORBA::ULong PD_MAGIC; // "C+IV"
  CORBA::ULong      pd_magic;

  CORBA::ULong      pd_in_truncatable;
  InputTableEntry** pd_table;
  CORBA::ULong      pd_table_count;
  CORBA::ULong      pd_table_size;
  CORBA::ULong      pd_table_limit;
  CORBA::ULong      pd_table_next_idx;
};


OMNI_NAMESPACE_END(omni)


#undef _dyn_attr

#endif // __VALUETRACKERIMPL_H__
