// -*- Mode: C++; -*-
//                          Package   : omniNames
// NamingContext_i.cc       Author    : Tristan Richardson (tjr)
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

#include <assert.h>
#include <NamingContext_i.h>
#include <ObjectBinding.h>
#include <BindingIterator_i.h>
#include <omniORB4/omniURI.h>

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif

ReadersWritersLock NamingContext_i::lock;
NamingContext_i* NamingContext_i::headContext = (NamingContext_i*)0;
NamingContext_i* NamingContext_i::tailContext = (NamingContext_i*)0;

OMNI_USING_NAMESPACE(omni)

//
// Ctor.
//
NamingContext_i::NamingContext_i(PortableServer::POA_ptr poa,
				 const PortableServer::ObjectId& id,
				 omniNameslog* l)
  : redolog(l), nc_poa(poa)
{
  headBinding = tailBinding = (ObjectBinding*)0;
  size = 0;

  WriterLock w(lock);

  redolog->create(id);

  prev = tailContext;
  next = (NamingContext_i*)0;
  tailContext = this;
  if (prev) {
    prev->next = this;
  }
  else {
    headContext = this;
  }

  poa->activate_object_with_id(id, this);
}


//
// new_context().
//

CosNaming::NamingContext_ptr
NamingContext_i::new_context()
{
  LOG(4, "new_context");

  CORBA::Object_var ref = names_poa->create_reference(
				    CosNaming::NamingContext::_PD_repoId);
  PortableServer::ObjectId_var id = names_poa->reference_to_id(ref);

  NamingContext_i* nc = new NamingContext_i(names_poa, id, redolog);
  CosNaming::NamingContext_ptr ncref = nc->_this();
  nc->_remove_ref();

  return ncref;
}



//
// resolve_simple() returns the ObjectBinding for a given simple name.
// The thread calling this must have called either lock.readerIn() or
// lock.writerIn() before calling this routine.
//

ObjectBinding* 
NamingContext_i::resolve_simple(const CosNaming::Name& n)
{
  assert(n.length() == 1);

  LOG(4, "resolve_simple (" << n[0].id << '.' << n[0].kind << ")");

  for (ObjectBinding* ob = headBinding; ob; ob = ob->next) {

    assert(ob->binding.binding_name.length() == 1);

    if ((strcmp(n[0].id,ob->binding.binding_name[0].id) == 0) &&
	(strcmp(n[0].kind,ob->binding.binding_name[0].kind) == 0))
      {
	LOG(4, "resolve_simple: found (" << n[0].id << '.' << n[0].kind << ")");

	return ob;
      }
  }

  LOG(4, "resolve_simple: didn't find (" << n[0].id << '.' << n[0].kind << ")");

  throw CosNaming::NamingContext::NotFound(CosNaming::NamingContext::
					   missing_node, n);
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}


//
// resolve_compound() returns an object reference for the first component of
// the given name (which must be a context), and the rest of the name.
//

CosNaming::NamingContext_ptr
NamingContext_i::resolve_compound(const CosNaming::Name& n,
				  CosNaming::Name& restOfName)
{
  if (omniORB::trace(4)) {
    omniORB::logger log("omniNames: ");
    log << "resolve_compound name (";

    for (unsigned int j = 0; j < n.length(); j++) {
      if (j != 0) log << '/';
      log << n[j].id << '.' << n[j].kind;
    }
    log << ")\n";
  }

  if (n.length() == 0) {
    throw CosNaming::NamingContext::InvalidName();
  }

  CosNaming::Name contextName = n;
  contextName.length(1);
  restOfName.length(n.length() - 1);
  for (unsigned int i = 0; i < n.length() - 1; i++) {
    restOfName[i] = n[i + 1];
  }

  ObjectBinding* ob;

  ReaderLock r(lock);

  try {
    ob = resolve_simple(contextName);
  }
  catch (CosNaming::NamingContext::NotFound& ex) {
    ex.rest_of_name = n;
    throw;
  }

  CosNaming::NamingContext_var context
    = CosNaming::NamingContext::_narrow(ob->object);

  if (CORBA::is_nil((CosNaming::NamingContext_ptr)context) ||
      (ob->binding.binding_type != CosNaming::ncontext))
  {
    LOG(4, "resolve_compound: object (" << n[0].id << '.' << n[0].kind <<
        ") not bound as a context; raising exception");
    throw CosNaming::NamingContext::NotFound(CosNaming::NamingContext::
					     not_context, n);
  }
  return CosNaming::NamingContext::_duplicate(context);
}


//
// resolve.
//

CORBA::Object_ptr
NamingContext_i::resolve(const CosNaming::Name& n)
{
  if (omniORB::trace(3)) {
    omniORB::logger log("omniNames: ");
    log << "resolve (";

    for (unsigned int j = 0; j < n.length(); j++) {
      if (j != 0) log << '/';
      log << n[j].id << '.' << n[j].kind;
    }
    log << ")\n";
  }

  if (n.length() == 1) {
    ReaderLock r(lock);
    ObjectBinding* ob = resolve_simple(n);
    return CORBA::Object::_duplicate(ob->object);
  }
  else {
    CosNaming::Name restOfName;
    CosNaming::NamingContext_var context = resolve_compound(n, restOfName);
    return context->resolve(restOfName);
  }
}


//
// bind_helper implements the 4 flavours of bind ([re]bind[_context]).
//

void
NamingContext_i::bind_helper(const CosNaming::Name& n, CORBA::Object_ptr obj,
			     CosNaming::BindingType t, CORBA::Boolean rebind)
{
  if (n.length() == 1) {
    //
    // Bind a simple name - i.e. bind object in this context.
    //

    LOG(2, "bind simple name (" << n[0].id << '.' << n[0].kind << ')');

    WriterLock w(lock);

    ObjectBinding* ob = 0;

    try {
      ob = resolve_simple(n);
      if (!rebind)
	throw CosNaming::NamingContext::AlreadyBound();
    }
    catch (CosNaming::NamingContext::NotFound& ex) {
      ob = 0;
    }

    CosNaming::NamingContext_var nc = _this();
    redolog->bind(nc, n, obj, t);

    if (ob) {
      LOG(4, "rebind: unbinding simple name (" << n[0].id << '.' << n[0].kind
          << ')');
      delete ob;
    }

    new ObjectBinding(n, t, obj, this);

    LOG(4, "bound simple name (" << n[0].id << '.' << n[0].kind << ')');
  }
  else {
    //
    // Bind a compound name <c1;c2;...;cn> - i.e. bind object to name
    // <c2;...;cn> in the context <c1>.
    //

    if (omniORB::trace(2)) {
      omniORB::logger log("omniNames: ");
      log << "bind compound name (";
    
      for (unsigned int j = 0; j < n.length(); j++) {
        if (j != 0) log << '/';
        log << n[j].id << '.' << n[j].kind;
      }
      log << ")\n";
    }

    CosNaming::Name restOfName;
    CosNaming::NamingContext_var context = resolve_compound(n, restOfName);

    if (t == CosNaming::nobject) {
      if (rebind)
	context->rebind(restOfName, obj);
      else
	context->bind(restOfName, obj);
    }
    else {
      if (rebind)
	context->rebind_context(restOfName,
				CosNaming::NamingContext::_narrow(obj));
      else
	context->bind_context(restOfName,
			      CosNaming::NamingContext::_narrow(obj));
    }
  }
}


//
// unbind()
//

void
NamingContext_i::unbind(const CosNaming::Name& n)
{
  if (n.length() == 1) {
    //
    // Unbind a simple name - i.e. remove it from this context.
    //

    LOG(2, "unbind simple name (" << n[0].id << '.' << n[0].kind << ')');

    WriterLock w(lock);

    ObjectBinding* ob = resolve_simple(n);

    CosNaming::NamingContext_var nc = _this();
    redolog->unbind(nc, n);

    delete ob;
  }
  else {
    //
    // Unbind a compound name <c1;c2;...;cn> - i.e. unbind the name
    // <c2;...;cn> from the context <c1>.
    //

    if (omniORB::trace(2)) {
      omniORB::logger log("omniNames: ");
      log << "unbind compound name (";
    
      for (unsigned int j = 0; j < n.length(); j++) {
        if (j != 0) log << '/';
        log << n[j].id << '.' << n[j].kind;
      }
      log << ")\n";
    }
    CosNaming::Name restOfName;
    CosNaming::NamingContext_var context = resolve_compound(n, restOfName);

    context->unbind(restOfName);
  }
}


//
// bind_new_context()
//

CosNaming::NamingContext_ptr
NamingContext_i::bind_new_context(const CosNaming::Name& n)
{
  if (n.length() == 1) {
    //
    // Bind a new context with a simple name - i.e. create a new context and
    // bind it in this context.
    //

    LOG(2, "bind_new_context simple name (" <<
        n[0].id << '.' << n[0].kind << ')');

    CosNaming::NamingContext_ptr nc = new_context();
    try {
      bind_context(n, nc);
    }
    catch (...) {
      nc->destroy();
      CORBA::release(nc);
      throw;
    }
    return nc;
  }
  else {
    //
    // Bind a new context with a compound name <c1;c2;...;cn> - i.e.
    // bind_new_context <c2;...;cn> in the context <c1>.
    //

    if (omniORB::trace(2)) {
      omniORB::logger log("omniNames: ");
      log << "bind_new_context compound name (";
    
      for (unsigned int j = 0; j < n.length(); j++) {
        if (j != 0) log << '/';
        log << n[j].id << '.' << n[j].kind;
      }
      log << ")\n";
    }
    CosNaming::Name restOfName;
    CosNaming::NamingContext_var context = resolve_compound(n, restOfName);

    return context->bind_new_context(restOfName);
  }
}


//
// destroy()
//

void
NamingContext_i::destroy()
{
  LOG(4, "destroy");

  WriterLock w(lock);

  if (headBinding)
    throw CosNaming::NamingContext::NotEmpty();

  CosNaming::NamingContext_var nc = _this();
  redolog->destroy(nc);

  PortableServer::ObjectId_var id = nc_poa->servant_to_id(this);
  nc_poa->deactivate_object(id);
}


//
// list()
//

void
NamingContext_i::list(CORBA::ULong how_many, CosNaming::BindingList_out bl,
		      CosNaming::BindingIterator_out bi)
{
  lock.readerIn();

  LOG(3, "list context: how_many = " << how_many << ", size = " << size);

  CosNaming::BindingList_var all = new CosNaming::BindingList(size);
  all->length(size);

  unsigned int i;
  ObjectBinding* ob;

  for (ob = headBinding, i = 0; ob; ob = ob->next, i++) {
    assert(i < size);
    all[i] = ob->binding;
  }

  lock.readerOut();

  if (all->length() <= how_many) {
    // don't need an iterator.  All results can go back as a
    // result of this call
    bi = CosNaming::BindingIterator::_nil();
    bl = all._retn();
    return;
  }

  BindingIterator_i* bii = new BindingIterator_i(names_poa, all._retn());

  bi = bii->_this();
  bii->_remove_ref();

  bi->next_n(how_many, bl);
}


//
// destructor
//

NamingContext_i::~NamingContext_i()
{
  WriterLock w(lock);

  if (prev) {
    prev->next = next;
  }
  else {
    headContext = next;
  }
  if (next) {
    next->prev = prev;
  }
  else {
    tailContext = prev;
  }

  while (headBinding)
    delete headBinding;
}


//
// CosNaming::NamingContextExt operations
//

char*
NamingContext_i::to_string(const CosNaming::Name& name)
{
  return omniURI::nameToString(name);
}

CosNaming::Name*
NamingContext_i::to_name(const char* sn)
{
  return omniURI::stringToName(sn);
}

char*
NamingContext_i::to_url(const char* addr, const char* sn)
{
  return omniURI::addrAndNameToURI(addr, sn);
}

CORBA::Object_ptr
NamingContext_i::resolve_str(const char* sn)
{
  CosNaming::Name_var name = omniURI::stringToName(sn);
  return resolve(name);
}
