// -*- Mode: C++; -*-
//                            Package   : omniORB
// valueTracker.cc            Created on: 2003/09/15
//                            Author    : Duncan Grisby
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
//    Position trackers for indirections in valuetypes.
//

#include <omniORB4/CORBA.h>
#include <omniORB4/valueType.h>
#include <valueTrackerImpl.h>


OMNI_NAMESPACE_BEGIN(omni)

// Hash table sizes
static int tableSizes[] = {
  128 + 3,              // 2^7
  1024 + 9,             // 2^10
  8192 + 27,            // 2^13
  32768 + 3,            // 2^15
  65536 + 45,           // 2^16
  131072 + 9,
  262144 + 39,
  524288 + 39,
  1048576 + 9,          // 2^20
  2097152 + 5,
  4194304 + 3,
  8388608 + 33,
  16777216 + 27,
  33554432 + 9,         // 2^25
  67108864 + 71,
  134217728 + 39,
  268435456 + 9,
  536870912 + 5,
  1073741824 + 83,      // 2^30 -- I'd be suprised if this is exceeded!
  -1                    // Sentinel to detect the end, just to be paranoid.
};

const CORBA::ULong OutputValueTracker::PD_MAGIC = 0x432b4f56; // "C+OV"
const CORBA::ULong InputValueTracker::PD_MAGIC  = 0x432b4956; // "C+IV"

// Hash table entries

enum OTKind { OT_VALUE, OT_REPOID, OT_REPOIDS };

struct OutputTableEntry {

  OutputTableEntry(const CORBA::ValueBase* v,
                   omni::ptr_arith_t       h,
                   CORBA::Long             p,
                   OutputTableEntry*       n)
    : kind(OT_VALUE), value(v), position(p), hashbase(h), next(n) {}

  OutputTableEntry(const char*       r,
                   omni::ptr_arith_t h,
                   CORBA::Long       p,
                   OutputTableEntry* n)
    : kind(OT_REPOID), repoId(r), position(p), hashbase(h), next(n) {}

  OutputTableEntry(const _omni_ValueIds* r,
                   omni::ptr_arith_t     h,
                   CORBA::Long           p,
                   OutputTableEntry*     n)
    : kind(OT_REPOIDS), repoIds(r), position(p), hashbase(h), next(n) {}

  OTKind kind;
  union {
    const CORBA::ValueBase* value;
    const char*        	    repoId;
    const _omni_ValueIds*   repoIds;
  };
  CORBA::Long          	    position;
  omni::ptr_arith_t         hashbase;
  OutputTableEntry*    	    next;
};

enum ITKind { IT_VALUE, IT_REPOID, IT_REPOIDS, IT_INDIRECT };

struct InputTableEntry {

  InputTableEntry(CORBA::ValueBase* v, CORBA::Long p, InputTableEntry* n)
    : kind(IT_VALUE), value(v), position(p), next(n) {}

  InputTableEntry(char* r, CORBA::Long p, InputTableEntry* n)
    : kind(IT_REPOID), repoId(r), position(p), next(n) {}

  InputTableEntry(_omni_ValueIds* r, CORBA::Long p, InputTableEntry* n)
    : kind(IT_REPOIDS), repoIds(r), position(p), next(n) {}

  InputTableEntry(InputTableEntry* i, CORBA::Long p, InputTableEntry* n)
    : kind(IT_INDIRECT), indirect(i), position(p), next(n) {}

  ITKind kind;
  union {
    CORBA::ValueBase* value;
    char*             repoId;
    _omni_ValueIds*   repoIds;
    InputTableEntry*  indirect;
  };
  CORBA::Long         position;
  InputTableEntry*    next;
};



static CORBA::Boolean
listsMatch(const _omni_ValueIds* l1, const _omni_ValueIds* l2)
{
  if (l1 == l2)
    return 1;

  if ((l1->idcount != l2->idcount) || (l1->hashval != l2->hashval))
    return 0;

  for (CORBA::Long i=0; i < l1->idcount; i++) {
    if (!omni::ptrStrMatch(l1->repoIds[i].repoId, l2->repoIds[i].repoId))
      return 0;
  }
  return 1;
}



OutputValueTracker::
OutputValueTracker()
  : pd_magic(PD_MAGIC), pd_in_truncatable(0), pd_table(0),
    pd_table_count(0), pd_table_size(0), pd_table_limit(0), pd_table_next_idx(0)

{
  omniORB::logs(25, "Create output value indirection tracker");
  resizeTable();
}

OutputValueTracker::
~OutputValueTracker()
{
  omniORB::logs(25, "Delete output value indirection tracker");

  for (CORBA::ULong i=0; i < pd_table_size; i++) {
    if (pd_table[i]) {
      OutputTableEntry *e, *n;
      for (e = pd_table[i]; e; e = n) {
	n = e->next;
	delete e;
      }
    }
  }
  delete [] pd_table;
}

void
OutputValueTracker::
resizeTable()
{
  int isize = tableSizes[pd_table_next_idx];
  if (isize == -1)
    return;

  CORBA::ULong size = isize;
  CORBA::ULong idx;

  ++pd_table_next_idx;

  OutputTableEntry** table = new OutputTableEntry*[size];
  for (idx=0; idx != size; ++idx)
    table[idx] = 0;

  if (pd_table) {
    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Resize output value tracker table to " << size << " entries.\n";
    }
    OutputTableEntry *e, *n;
    CORBA::ULong     h;

    for (idx=0; idx != pd_table_size; ++idx) {
      for (e = pd_table[idx]; e; e = n) {
        n        = e->next;
        h        = e->hashbase % size;
        e->next  = table[h];
        table[h] = e;
      }
    }

    delete [] pd_table;
  }

  pd_table_size  = size;
  pd_table_limit = size * 2 / 3;
  pd_table       = table;
}


CORBA::Long
OutputValueTracker::
addValue(const CORBA::ValueBase* val, CORBA::Long current)
{
  OutputTableEntry* e;

  CORBA::ULong h = ((omni::ptr_arith_t)val % pd_table_size);

  for (e = pd_table[h]; e; e = e->next) {
    if (e->kind == OT_VALUE && e->value == val)
      return e->position;
  }
  add();
  e = new OutputTableEntry(val, (omni::ptr_arith_t)val,
                           current, pd_table[h]);
  pd_table[h] = e;
  return -1;
}

CORBA::Long
OutputValueTracker::
addRepoId(const char* repoId, CORBA::ULong hashval, CORBA::Long current)
{
  OutputTableEntry* e;

  CORBA::ULong h = hashval % pd_table_size;

  for (e = pd_table[h]; e; e = e->next) {
    if (e->kind == OT_REPOID && omni::ptrStrMatch(repoId, e->repoId))
      return e->position;
  }
  add();
  e = new OutputTableEntry(repoId, hashval, current, pd_table[h]);
  pd_table[h] = e;
  return -1;
}


CORBA::Long
OutputValueTracker::
addRepoIds(const _omni_ValueIds* repoIds, CORBA::Long current)
{
  OutputTableEntry* e;

  CORBA::ULong h = repoIds->hashval % pd_table_size;

  for (e = pd_table[h]; e; e = e->next) {
    if (e->kind == OT_REPOIDS && listsMatch(e->repoIds, repoIds))
      return e->position;
  }
  add();
  e = new OutputTableEntry(repoIds, repoIds->hashval, current, pd_table[h]);
  pd_table[h] = e;
  return -1;
}


InputValueTracker::
InputValueTracker()
  : pd_magic(PD_MAGIC), pd_in_truncatable(0), pd_table(0),
    pd_table_count(0), pd_table_size(0), pd_table_limit(0), pd_table_next_idx(0)
{
  omniORB::logs(25, "Create input value indirection tracker");
  resizeTable();
}

InputValueTracker::
~InputValueTracker()
{
  omniORB::logs(25, "Delete input value indirection tracker");

  for (CORBA::ULong i=0; i < pd_table_size; i++) {
    if (pd_table[i]) {
      InputTableEntry *e, *n;
      for (e = pd_table[i]; e; e = n) {
	switch (e->kind) {
	case IT_VALUE:
	  CORBA::remove_ref(e->value);
	  break;
	case IT_REPOID:
	  CORBA::string_free(e->repoId);
	  break;
	case IT_REPOIDS:
	  // Note that the individual repoId strings are not freed
	  // here, since they have been separately registered. They
	  // will be freed from their own entries by the IT_REPOID
	  // case above.
	  delete [] e->repoIds->repoIds;
	  delete e->repoIds;
	  break;
	case IT_INDIRECT:
	  break;
	}
	n = e->next;
	delete e;
      }
    }
  }
  delete [] pd_table;
}


void
InputValueTracker::
resizeTable()
{
  int isize = tableSizes[pd_table_next_idx];
  if (isize == -1)
    return;

  CORBA::ULong size = isize;
  CORBA::ULong idx;

  ++pd_table_next_idx;

  InputTableEntry** table = new InputTableEntry*[size];
  for (idx=0; idx != size; ++idx)
    table[idx] = 0;

  if (pd_table) {
    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Resize input value tracker table to " << size << " entries.\n";
    }
    InputTableEntry *e, *n;
    CORBA::ULong     h;

    for (idx=0; idx != pd_table_size; ++idx) {
      for (e = pd_table[idx]; e; e = n) {
        n        = e->next;
        h        = e->position % size;
        e->next  = table[h];
        table[h] = e;
      }
    }

    delete [] pd_table;
  }

  pd_table_size  = size;
  pd_table_limit = size * 2 / 3;
  pd_table       = table;
}



void
InputValueTracker::
addValue(CORBA::ValueBase* val, CORBA::Long current)
{
  add();
  CORBA::ULong hashval = current % pd_table_size;
  pd_table[hashval] = new InputTableEntry(val, current, pd_table[hashval]);
}

void
InputValueTracker::
addRepoId(char* repoId, CORBA::Long current)
{
  add();
  CORBA::ULong hashval = current % pd_table_size;
  pd_table[hashval] = new InputTableEntry(repoId, current, pd_table[hashval]);
}

void
InputValueTracker::
addRepoIds(_omni_ValueIds* repoIds, CORBA::Long current)
{
  add();
  CORBA::ULong hashval = current % pd_table_size;
  pd_table[hashval] = new InputTableEntry(repoIds,current,pd_table[hashval]);
}

CORBA::ValueBase*
InputValueTracker::
lookupValue(CORBA::Long pos,
	    CORBA::Long current,
	    CORBA::CompletionStatus comp)
{
  CORBA::ULong hashval = pos % pd_table_size;
  for (InputTableEntry* e = pd_table[hashval]; e; e = e->next) {
    if (e->position == pos) {

      if (e->kind == IT_INDIRECT)  // Indirection to an indirection
	e = e->indirect;

      if (e->kind == IT_VALUE) {
	hashval = current % pd_table_size;
	pd_table[hashval] = new InputTableEntry(e, current, pd_table[hashval]);
	return e->value;
      }
      break;
    }
  }
  OMNIORB_THROW(MARSHAL, MARSHAL_InvalidIndirection, comp);
  return 0;
}

const char*
InputValueTracker::
lookupRepoId(CORBA::Long pos,
	     CORBA::Long current,
	     CORBA::CompletionStatus comp)
{
  CORBA::ULong hashval = pos % pd_table_size;
  for (InputTableEntry* e = pd_table[hashval]; e; e = e->next) {
    if (e->position == pos) {

      if (e->kind == IT_INDIRECT)  // Indirection to an indirection
	e = e->indirect;

      if (e->kind == IT_REPOID) {
	hashval = current % pd_table_size;
	pd_table[hashval] = new InputTableEntry(e, current, pd_table[hashval]);
	return e->repoId;
      }
      break;
    }
  }
  OMNIORB_THROW(MARSHAL, MARSHAL_InvalidIndirection, comp);
  return 0;
}

const _omni_ValueIds*
InputValueTracker::
lookupRepoIds(CORBA::Long pos,
	      CORBA::Long current,
	      CORBA::CompletionStatus comp)
{
  CORBA::ULong hashval = pos % pd_table_size;
  for (InputTableEntry* e = pd_table[hashval]; e; e = e->next) {
    if (e->position == pos) {

      if (e->kind == IT_INDIRECT)  // Indirection to an indirection
	e = e->indirect;

      if (e->kind == IT_REPOIDS) {
	hashval = current % pd_table_size;
	pd_table[hashval] = new InputTableEntry(e, current, pd_table[hashval]);
	return e->repoIds;
      }
      break;
    }
  }
  OMNIORB_THROW(MARSHAL, MARSHAL_InvalidIndirection, comp);
  return 0;
}



OMNI_NAMESPACE_END(omni)
