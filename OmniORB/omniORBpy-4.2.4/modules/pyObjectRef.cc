// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyObjectRef.cc             Created on: 1999/07/29
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2002-2014 Apasphere Ltd
//    Copyright (C) 1999 AT&T Laboratories Cambridge
//
//    This file is part of the omniORBpy library
//
//    The omniORBpy library is free software; you can redistribute it
//    and/or modify it under the terms of the GNU Lesser General
//    Public License as published by the Free Software Foundation;
//    either version 2.1 of the License, or (at your option) any later
//    version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library. If not, see http://www.gnu.org/licenses/
//
// Description:
//    Versions of ORB object ref functions which deal with Python
//    objects, rather than C++ objects

#include <omnipy.h>
#include <omniORBpy.h>

// Internal omniORB interfaces
#include <objectTable.h>
#include <remoteIdentity.h>
#include <inProcessIdentity.h>
#include <objectAdapter.h>
#include <omniORB4/omniURI.h>
#include <giopStrand.h>
#include <giopStream.h>
#include <omniCurrent.h>
#include <poaimpl.h>

OMNI_USING_NAMESPACE(omni)

#if defined(HAS_Cplusplus_Namespace)
using omniORB::operator==;
#endif


class Py_omniObjRef : public virtual CORBA::Object,
		      public virtual omniObjRef
{
public:
  Py_omniObjRef(const char*        repoId,
		omniIOR*           ior,
		omniIdentity*      id)

    : omniObjRef(repoId, ior, id)
  {
    _PR_setobj(this);
  }
  virtual ~Py_omniObjRef() { }

  virtual const char* _localServantTarget();

private:
  virtual void* _ptrToObjRef(const char* target);

  // Not implemented:
  Py_omniObjRef(const Py_omniObjRef&);
  Py_omniObjRef& operator=(const Py_omniObjRef&);
};

const char*
Py_omniObjRef::_localServantTarget()
{
  return omniPy::string_Py_omniServant;
}

void*
Py_omniObjRef::_ptrToObjRef(const char* target)
{
  if (omni::ptrStrMatch(target, omniPy::string_Py_omniObjRef))
    return (Py_omniObjRef*)this;

  if (omni::ptrStrMatch(target, CORBA::Object::_PD_repoId))
    return (CORBA::Object_ptr)this;

  return 0;
}


PyObject*
omniPy::createPyCorbaObjRef(const char*             targetRepoId,
			    const CORBA::Object_ptr objref)
{
  if (CORBA::is_nil(objref)) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  if (objref->_NP_is_pseudo())
    return createPyPseudoObjRef(objref);

  omniObjRef* ooref = objref->_PR_getobj();

  const char*    actualRepoId = ooref->_mostDerivedRepoId();
  PyObject*      objrefClass;
  CORBA::Boolean fullTypeUnknown = 0;

  // Try to find objref class for most derived type:
  objrefClass = PyDict_GetItemString(pyomniORBobjrefMap, (char*)actualRepoId);

  if (targetRepoId &&
      !omni::ptrStrMatch(targetRepoId, actualRepoId) &&
      !omni::ptrStrMatch(targetRepoId, CORBA::Object::_PD_repoId)) {

    // targetRepoId is not plain CORBA::Object, and is different from
    // actualRepoId

    if (objrefClass) {
      // We've got an objref class for the most derived type. Is it a
      // subclass of the target type?
      PyObject* targetClass = PyDict_GetItemString(pyomniORBobjrefMap,
						   (char*)targetRepoId);

      if (!PyObject_IsSubclass(objrefClass, targetClass)) {
	// Actual type is not derived from the target. Surprisingly
	// enough, this is valid -- the repoId in an object reference
	// is not necessarily that of the most derived type for the
	// object. If we are expecting interface A, and actually get
	// unrelated B, the object might actually have interface C,
	// derived from both A and B.
	//
	// In this situation, we must create an object reference of
	// the target type, not the object's claimed type.
	objrefClass     = targetClass;
	fullTypeUnknown = 1;
      }
    }
    else {
      // No objref class for the most derived type -- try to find one for
      // the target type:
      objrefClass     = PyDict_GetItemString(pyomniORBobjrefMap,
					     (char*)targetRepoId);
      fullTypeUnknown = 1;
    }
  }
  if (!objrefClass) {
    // No target type, or stub code bug:
    objrefClass     = PyObject_GetAttrString(pyCORBAmodule, (char*)"Object");
    fullTypeUnknown = 1;
  }

  OMNIORB_ASSERT(objrefClass); // Couldn't even find CORBA.Object!

  omniPy::PyRefHolder args(PyTuple_New(1));
  PyTuple_SET_ITEM(args, 0, createPyObjRefObject(objref));

  PyObject* pyobjref = PyObject_CallObject(objrefClass, args);

  if (!pyobjref) {
    // Oh dear -- return the error to the program
    return 0;
  }

  if (fullTypeUnknown) {
    PyObject* idstr = String_FromString(actualRepoId);
    PyObject_SetAttrString(pyobjref, (char*)"_NP_RepositoryId", idstr);
    Py_DECREF(idstr);
  }

  return pyobjref;
}


PyObject*
omniPy::createPyPseudoObjRef(const CORBA::Object_ptr objref)
{
  {
    CORBA::ORB_var orbp = CORBA::ORB::_narrow(objref);
    if (!CORBA::is_nil(orbp)) {
      OMNIORB_ASSERT(omniPy::orb);
      return PyObject_GetAttrString(omniPy::pyomniORBmodule, (char*)"orb");
    }
  }
  {
    PortableServer::POA_var poa = PortableServer::POA::_narrow(objref);
    if (!CORBA::is_nil(poa)) return createPyPOAObject(poa);
  }
  {
    PortableServer::POAManager_var pm =
      PortableServer::POAManager::_narrow(objref);
    if (!CORBA::is_nil(pm)) return createPyPOAManagerObject(pm);
  }
  {
    PortableServer::Current_var pc = PortableServer::Current::_narrow(objref);
    if (!CORBA::is_nil(pc)) return createPyPOACurrentObject(pc);
  }

  {
    // No built in converter. Try the list of registered external functions
    int len = PySequence_Length(omniPy::py_pseudoFns);
    for (int i=0; i < len; i++) {
      PyObject* pyf = PySequence_GetItem(omniPy::py_pseudoFns, i);

#if (PY_VERSION_HEX <= 0x03000000)

      if (!PyCObject_Check(pyf)) {
	omniORB::logs(1, "WARNING: Entry in _omnipy.pseudoFns "
		      "is not a PyCObject.");
	continue;
      }
      omniORBpyPseudoFn f = (omniORBpyPseudoFn)PyCObject_AsVoidPtr(pyf);
#else
      if (!PyCapsule_CheckExact(pyf)) {
	omniORB::logs(1, "WARNING: Entry in _omnipy.pseudoFns "
		      "is not a PyCapsule.");
	continue;
      }
      omniORBpyPseudoFn f = (omniORBpyPseudoFn)PyCapsule_GetPointer(pyf, 0);
#endif
      PyObject* ret = f(objref);
      if (ret)
	return ret;
    }
  };

  try {
    // Use OMNIORB_THROW to get a nice trace message
    OMNIORB_THROW(INV_OBJREF, INV_OBJREF_NoPythonTypeForPseudoObj,
		  CORBA::COMPLETED_NO);
  }
  OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  return 0;
}



omniObjRef*
omniPy::createObjRef(const char*    	targetRepoId,
		     omniIOR*       	ior,
		     CORBA::Boolean 	locked,
		     omniIdentity*  	id,
		     CORBA::Boolean     type_verified,
		     CORBA::Boolean     is_forwarded)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, locked);
  OMNIORB_ASSERT(targetRepoId);
  OMNIORB_ASSERT(ior);

  CORBA::Boolean called_create = 0;

  if (!id) {
    ior->duplicate();  // consumed by createIdentity
    id = omni::createIdentity(ior, omniPy::string_Py_omniServant, locked);
    called_create = 1;
    if (!id) {
      ior->release();
      return 0;
    }
  }

  if (omniORB::trace(10)) {
    omniORB::logger l;
    l << "Creating Python ref to ";
    if      (omniLocalIdentity    ::downcast(id)) l << "local";
    else if (omniInProcessIdentity::downcast(id)) l << "in process";
    else if (omniRemoteIdentity   ::downcast(id)) l << "remote";
    else                                          l << "unknown";
    l << ": " << id << "\n"
      " target id      : " << targetRepoId << "\n"
      " most derived id: " << (const char*)ior->repositoryID() << "\n";
  }

  omniObjRef* objref = new Py_omniObjRef(targetRepoId, ior, id);

  if (!type_verified &&
      !omni::ptrStrMatch(targetRepoId, CORBA::Object::_PD_repoId)) {

    objref->pd_flags.type_verified = 0;
  }

  if (is_forwarded) {
    omniORB::logs(10, "Reference has been forwarded.");
    objref->pd_flags.forward_location = 1;
  }

  {
    omni_optional_lock sync(*omni::internalLock, locked, locked);
    id->gainRef(objref);
    if (called_create)
      id->loseRef();
  }

  if (orbParameters::persistentId.length()) {
    // Check to see if we need to re-write the IOR.

    omniIOR::IORExtraInfoList& extra = ior->getIORInfo()->extraInfo();

    for (CORBA::ULong index = 0; index < extra.length(); index++) {

      if (extra[index]->compid == IOP::TAG_OMNIORB_PERSISTENT_ID)

	if (!id->inThisAddressSpace()) {

	  omniORB::logs(15, "Re-write local persistent object reference.");

	  omniObjRef* new_objref;
	  omniIORHints hints(0);
	  {
	    omni_optional_lock sync(*internalLock, locked, locked);

	    omniIOR* new_ior = new omniIOR(ior->repositoryID(),
					   id->key(), id->keysize(), hints);

	    new_objref = createObjRef(targetRepoId, new_ior,
				      1, 0, type_verified);
	  }
	  releaseObjRef(objref);
	  objref = new_objref;
	}
      break;
    }
  }
  return objref;
}


omniObjRef*
omniPy::createLocalObjRef(const char*         mostDerivedRepoId,
			  const char*         targetRepoId,
			  omniObjTableEntry*  entry,
			  omniObjRef*         orig_ref,
			  CORBA::Boolean      type_verified)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(targetRepoId);
  OMNIORB_ASSERT(entry);

  // See if a suitable reference exists in the local ref list.
  // Suitable means having the same most-derived-intf-repo-id, and
  // also supporting the <targetRepoId>.
  {
    omniObjRef* objref;

    omnivector<omniObjRef*>::iterator i    = entry->objRefs().begin();
    omnivector<omniObjRef*>::iterator last = entry->objRefs().end();

    for (; i != last; i++) {
      objref = *i;

      if (omni::ptrStrMatch(mostDerivedRepoId, objref->_mostDerivedRepoId()) &&
	  objref->_ptrToObjRef(omniPy::string_Py_omniObjRef) &&
	  omni::ptrStrMatch(targetRepoId, objref->pd_intfRepoId)) {

	// We just need to check that the ref count is not zero here,
	// 'cos if it is then the objref is about to be deleted!
	// See omni::releaseObjRef().

	omni::objref_rc_lock->lock();
	int dying = objref->pd_refCount == 0;
	if( !dying )  objref->pd_refCount++;
	omni::objref_rc_lock->unlock();

	if( !dying ) {
	  omniORB::logs(15, "omniPy::createLocalObjRef -- reusing "
			"reference from local ref list.");
	  return objref;
	}
      }
    }
  }
  // Reach here if we have to create a new objref.
  omniIOR* ior = orig_ref->_getIOR();
  return omniPy::createObjRef(targetRepoId, ior, 1, entry, type_verified);
}

omniObjRef*
omniPy::createLocalObjRef(const char* 	      mostDerivedRepoId,
			  const char* 	      targetRepoId,
			  const _CORBA_Octet* key,
			  int                 keysize,
			  omniObjRef*         orig_ref,
			  CORBA::Boolean      type_verified)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(targetRepoId);
  OMNIORB_ASSERT(key && keysize);

  // See if there's a suitable entry in the object table
  CORBA::ULong hashv = omni::hash(key, keysize);

  omniObjTableEntry* entry = omniObjTable::locateActive(key, keysize,
							hashv, 0);

  if (entry)
    return createLocalObjRef(mostDerivedRepoId, targetRepoId,
			     entry, orig_ref, type_verified);

  omniIOR* ior = orig_ref->_getIOR();
  return createObjRef(targetRepoId,ior,1,0,type_verified);
}




CORBA::Object_ptr
omniPy::makeLocalObjRef(const char* targetRepoId,
			const CORBA::Object_ptr objref)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  omniObjRef* ooref = objref->_PR_getobj();
  omniObjRef* newooref;

  {
    omni_tracedmutex_lock sync(*omni::internalLock);
    
    omniObjTableEntry* entry = omniObjTableEntry::downcast(ooref->_identity());

    if (entry)
      newooref = omniPy::createLocalObjRef(ooref->_mostDerivedRepoId(),
					   targetRepoId, entry, ooref, 1);
    else
      newooref = omniPy::createLocalObjRef(ooref->_mostDerivedRepoId(),
					   targetRepoId,
					   ooref->_identity()->key(),
					   ooref->_identity()->keysize(),
					   ooref, 1);
  }
  return (CORBA::Object_ptr)newooref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}


PyObject*
omniPy::copyObjRefArgument(PyObject* pytargetRepoId, PyObject* pyobjref,
			   CORBA::CompletionStatus compstatus)
{
  if (pyobjref == Py_None) {
    // Nil objref
    Py_INCREF(Py_None);
    return Py_None;
  }
  CORBA::Object_ptr objref = getObjRef(pyobjref);
  if (!objref) {
    // Not an objref
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting object reference, "
					    "got %r",
					    "O", pyobjref->ob_type));
  }

  // To copy an object reference, we have to take a number of things
  // into account. When the C++ object reference was created, it was
  // initialised with a most-derived repoId and a target repoId. If we
  // knew that the most-derived interface is compatible with the
  // target, then the Python objref is of the most derived type. If we
  // did not know the most-derived interface, or we did know it and
  // believed it to be incompatible with the target, then the Python
  // objref is of the target type, and it has a string attribute named
  // "_NP_RepositoryId" containing the most derived repoId.
  //
  // Now, as we are copying this objref, we have a target repoId,
  // which is possibly different from the objref's original target.
  // It's also possible that some time after we created the Python
  // objref, some new stubs were imported, so we now know about the
  // objref's most derived type when before we didn't.
  //
  // So, to copy the reference, we first see if the Python objref has
  // an attribute named "_NP_RepositoryId". If it does, all bets are
  // off, and we have to create a new C++ objref from scratch. If it
  // doesn't have the attribute, we look to see if the objref's class
  // is a subclass of the target objref class (or the same class). If
  // so, we can just incref the existing Python objref and return it;
  // if not, we have to build a new C++ objref.

  if (!PyObject_HasAttrString(pyobjref, (char*)"_NP_RepositoryId")) {

    PyObject* targetClass = PyDict_GetItem(pyomniORBobjrefMap,
					   pytargetRepoId);
    OMNIORB_ASSERT(targetClass);

    if (PyObject_IsInstance(pyobjref, targetClass)) {
      Py_INCREF(pyobjref);
      return pyobjref;
    }
  }
  // Create new C++ and Python objrefs with the right target type
  omniObjRef* ooref        = objref->_PR_getobj();
  const char* targetRepoId = String_AS_STRING(pytargetRepoId);

  if (targetRepoId[0] == '\0') targetRepoId = CORBA::Object::_PD_repoId;

  omniObjRef* newooref;
  {
    omniPy::InterpreterUnlocker _u;
    newooref = omniPy::createObjRef(targetRepoId, ooref->_getIOR(), 0, 0);
  }
  PyObject* r = createPyCorbaObjRef(targetRepoId,
				    (CORBA::Object_ptr)newooref->
				      _ptrToObjRef(CORBA::Object::_PD_repoId));
  if (!r) {
    if (omniORB::trace(1)) {
      {
	omniORB::logger l;
	l <<
	  "Caught an unexpected Python exception trying to create an "
	  "object reference.\n";
      }
      PyErr_Print();
    }
    PyErr_Clear();
    OMNIORB_THROW(INTERNAL, 0, compstatus);
  }
  return r;
}


CORBA::Object_ptr
omniPy::stringToObject(const char* uri)
{
  CORBA::Object_ptr cxxobj;
  omniObjRef* objref;

  {
    omniPy::InterpreterUnlocker _u;
    cxxobj = omniURI::stringToObject(uri);

    if (CORBA::is_nil(cxxobj) || cxxobj->_NP_is_pseudo()) {
      return cxxobj;
    }
    omniObjRef* cxxobjref = cxxobj->_PR_getobj();

    objref = omniPy::createObjRef(CORBA::Object::_PD_repoId,
				  cxxobjref->_getIOR(), 0, 0);
    CORBA::release(cxxobj);
  }
  return (CORBA::Object_ptr)objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}


CORBA::Object_ptr
omniPy::UnMarshalObjRef(const char* repoId, cdrStream& s)
{
  CORBA::String_var           id;
  IOP::TaggedProfileList_var  profiles;

  id = IOP::IOR::unmarshaltype_id(s);

  profiles = new IOP::TaggedProfileList();
  (IOP::TaggedProfileList&)profiles <<= s;

  if (profiles->length() == 0 && strlen(id) == 0) {
    // Nil object reference
    return CORBA::Object::_nil();
  }
  else {
    omniPy::InterpreterUnlocker _u;

    // It is possible that we reach here with the id string = '\0'.
    // That is alright because the actual type of the object will be
    // verified using _is_a() at the first invocation on the object.
    //
    // Apparently, some ORBs such as ExperSoft's do that. Furthermore,
    // this has been accepted as a valid behaviour in GIOP 1.1/IIOP 1.1.
    // 
    omniIOR* ior = new omniIOR(id._retn(),profiles._retn());

    giopStream* gs = giopStream::downcast(&s);
    if (gs) {
      giopStrand& g = gs->strand();
      if (g.isBiDir() && !g.isClient()) {
	// Check the POA policy to see if the servant's POA is willing
	// to use bidirectional on its callback objects.
	omniCurrent*        current = omniCurrent::get();
	omniCallDescriptor* desc    = current ? current->callDescriptor() : 0;

	if (desc && desc->poa() && desc->poa()->acceptBiDirectional()) {
	  const char* sendfrom = g.connection->peeraddress();
	  omniIOR::add_TAG_OMNIORB_BIDIR(sendfrom, *ior);
	}
      }
    }
    omniObjRef* objref = omniPy::createObjRef(repoId,ior,0);

    if (!objref) OMNIORB_THROW(MARSHAL, MARSHAL_InvalidIOR,
			       (CORBA::CompletionStatus)s.completion());
    return 
      (CORBA::Object_ptr)objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
  }
  return 0; // To shut GCC up
}


//
// Python objref type

extern "C" {

  static void
  pyObjRef_dealloc(PyObjRefObject* self)
  {
    {
      omniPy::InterpreterUnlocker _u;
      CORBA::release(self->obj);
    }
    Py_TYPE(self)->tp_free((PyObject*)self);
  }

  static PyObject*
  pyObjRef_invoke(PyObjRefObject* self, PyObject* args)
  {
    // Arg format
    //  (op_name, (in_desc,out_desc,exc_desc [, ctxt [,values]]), args)
    //
    //  exc_desc is a dictionary containing a mapping from repoIds to
    //  exception descriptor tuples.

    omniPy::Py_omniCallDescriptor::InvokeArgs iargs(self->obj, args);
    if (iargs.error())
      return 0;

    omniPy::Py_omniCallDescriptor call_desc(iargs);
    try {
      {
        omniPy::CDInterpreterUnlocker ul(call_desc);
        iargs.oobjref->_invoke(call_desc);
      }
      if (!call_desc.is_oneway()) {
	return call_desc.result();
      }
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    catch (omniPy::PyUserException& ex) {
      ex.setPyExceptionState();
    }
    catch (...) {
      omniORB::logs(1, "Unexpected C++ exception during Python invocation.");
      throw;
    }
    return 0;
  }

  static PyObject*
  pyObjRef_invoke_sendp(PyObjRefObject* self, PyObject* args)
  {
    // Arg format
    //  (op_name, descriptors, args, excep name)

    omniPy::Py_omniCallDescriptor::InvokeArgs iargs(self->obj, args);
    if (iargs.error())
      return 0;

    omniPy::Py_omniCallDescriptor* call_desc =
      new omniPy::Py_omniCallDescriptor(iargs, 1);

    iargs.oobjref->_invoke_async(call_desc);

    return call_desc->poller();
  }


  static PyObject*
  pyObjRef_invoke_sendc(PyObjRefObject* self, PyObject* args)
  {
    // Arg format
    //  (op_name, descriptors, args, excep name, callback)

    omniPy::Py_omniCallDescriptor::InvokeArgs iargs(self->obj, args);
    if (iargs.error())
      return 0;

    omniPy::Py_omniCallDescriptor* call_desc =
      new omniPy::Py_omniCallDescriptor(iargs, 0);

    iargs.oobjref->_invoke_async(call_desc);

    Py_INCREF(Py_None);
    return Py_None;
  }


  static PyObject*
  pyObjRef_isA(PyObjRefObject* self, PyObject* args)
  {
    char* repoId;

    if (!PyArg_ParseTuple(args, (char*)"s", &repoId))
      return 0;

    CORBA::Boolean isa;

    try {
      omniPy::InterpreterUnlocker ul;
      isa = self->obj->_is_a(repoId);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    return PyBool_FromLong(isa);
  }

  static PyObject*
  pyObjRef_nonExistent(PyObjRefObject* self, PyObject* args)
  {
    CORBA::Boolean nex;

    try {
      omniPy::InterpreterUnlocker ul;
      nex = self->obj->_non_existent();
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    return PyBool_FromLong(nex);
  }

  static PyObject*
  pyObjRef_isEquivalent(PyObjRefObject* self, PyObject* args)
  {
    PyObject* pyobjref2;

    if (!PyArg_ParseTuple(args, (char*)"O", &pyobjref2))
      return 0;

    CORBA::Object_ptr cxxobjref = omniPy::getObjRef(pyobjref2);
    RAISE_PY_BAD_PARAM_IF(!cxxobjref, BAD_PARAM_WrongPythonType);

    CORBA::Boolean ise;

    try {
      omniPy::InterpreterUnlocker ul;
      ise = self->obj->_is_equivalent(cxxobjref);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    return PyBool_FromLong(ise);
  }

  static PyObject*
  pyObjRef_hash(PyObjRefObject* self, PyObject* args)
  {
    CORBA::ULong max;

    if (!PyArg_ParseTuple(args, (char*)"i", &max))
      return 0;

    CORBA::ULong h = self->obj->_hash(max);
    return Int_FromLong(h);
  }


  static PyObject*
  pyObjRef_narrow(PyObjRefObject* self, PyObject* args)
  {
    char* repoId;
    int   checked;

    if (!PyArg_ParseTuple(args, (char*)"si", &repoId, &checked))
      return 0;

    CORBA::Boolean    isa;
    CORBA::Object_ptr cxxdest = 0;

    try {
      omniPy::InterpreterUnlocker ul;

      if (checked || self->obj->_NP_is_pseudo())
	isa = self->obj->_is_a(repoId);
      else
	isa = 1;

      if (isa) {
	if (!self->obj->_NP_is_pseudo()) {
	  omniObjRef* oosource = self->obj->_PR_getobj();
	  omniObjRef* oodest;
	  {
	    omni_tracedmutex_lock sync(*omni::internalLock);
	    oodest = omniPy::createObjRef(repoId, oosource->_getIOR(), 1,
					  oosource->_identity(), 1,
					  oosource->_isForwardLocation());
	  }
	  cxxdest = (CORBA::Object_ptr)
	                   (oodest->_ptrToObjRef(CORBA::Object::_PD_repoId));
	}
	else
	  cxxdest = CORBA::Object::_duplicate(self->obj);
      }
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    if (isa) {
      return omniPy::createPyCorbaObjRef(repoId, cxxdest);
    }
    else {
      Py_INCREF(Py_None);
      return Py_None;
    }
  }

  static PyObject*
  pyObjRef_disconnect(PyObjRefObject* self, PyObject* args)
  {
    try {
      omniObjRef* oo = self->obj->_PR_getobj();
      if (oo)
        oo->_NP_disconnect();
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyMethodDef pyObjRef_methods[] = {
    {(char*)"invoke",
     (PyCFunction)pyObjRef_invoke,
     METH_VARARGS},

    {(char*)"invoke_sendp",
     (PyCFunction)pyObjRef_invoke_sendp,
     METH_VARARGS},

    {(char*)"invoke_sendc",
     (PyCFunction)pyObjRef_invoke_sendc,
     METH_VARARGS},

    {(char*)"isA",
     (PyCFunction)pyObjRef_isA,
     METH_VARARGS},

    {(char*)"nonExistent",
     (PyCFunction)pyObjRef_nonExistent,
     METH_NOARGS},

    {(char*)"isEquivalent",
     (PyCFunction)pyObjRef_isEquivalent,
     METH_VARARGS},

    {(char*)"hash",
     (PyCFunction)pyObjRef_hash,
     METH_VARARGS},

    {(char*)"narrow",
     (PyCFunction)pyObjRef_narrow,
     METH_VARARGS},

    {(char*)"disconnect",
     (PyCFunction)pyObjRef_disconnect,
     METH_NOARGS},

    {NULL,NULL}
  };

  static PyTypeObject pyObjRefType = {
    PyVarObject_HEAD_INIT(0,0)
    (char*)"_omnipy.PyObjRefObject",   /* tp_name */
    sizeof(PyObjRefObject),            /* tp_basicsize */
    0,                                 /* tp_itemsize */
    (destructor)pyObjRef_dealloc,      /* tp_dealloc */
    0,                                 /* tp_print */
    0,                                 /* tp_getattr */
    0,                                 /* tp_setattr */
    0,                                 /* tp_compare */
    0,                                 /* tp_repr */
    0,                                 /* tp_as_number */
    0,                                 /* tp_as_sequence */
    0,                                 /* tp_as_mapping */
    0,                                 /* tp_hash  */
    0,                                 /* tp_call */
    0,                                 /* tp_str */
    0,                                 /* tp_getattro */
    0,                                 /* tp_setattro */
    0,                                 /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    (char*)"Internal ObjRef object",   /* tp_doc */
    0,                                 /* tp_traverse */
    0,                                 /* tp_clear */
    0,                                 /* tp_richcompare */
    0,                                 /* tp_weaklistoffset */
    0,                                 /* tp_iter */
    0,                                 /* tp_iternext */
    pyObjRef_methods,                  /* tp_methods */
  };
};


PyObject*
omniPy::createPyObjRefObject(CORBA::Object_ptr obj)
{
  PyObjRefObject* self = PyObject_New(PyObjRefObject, &pyObjRefType);
  self->obj = obj;
  return (PyObject*)self;
}

CORBA::Boolean
omniPy::pyObjRefCheck(PyObject* pyobj)
{
  return PyObject_TypeCheck(pyobj, &pyObjRefType);
}

PyTypeObject* omniPy::PyObjRefType;

void
omniPy::initObjRefFunc(PyObject* d)
{
  int r = PyType_Ready(&pyObjRefType);
  OMNIORB_ASSERT(r == 0);

  omniPy::PyObjRefType = &pyObjRefType;
}
