// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyPOAFunc.cc               Created on: 2000/02/04
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2003-2014 Apasphere Ltd
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
//
// Description:
//    POA functions

#include <omnipy.h>


static PyObject*
raisePOAException(const char* ename, PyObject* args=0)
{
  omniPy::PyRefHolder
    pypoa(PyObject_GetAttrString(omniPy::pyPortableServerModule, (char*)"POA"));

  omniPy::PyRefHolder
    excc(PyObject_GetAttrString(pypoa, (char*)ename));
  
  omniPy::PyRefHolder
    exci(PyObject_CallObject(excc, args ? args : omniPy::pyEmptyTuple));

  PyErr_SetObject(excc, exci);
  return 0;
}

static inline CORBA::ULong
getEnumVal(PyObject* pyenum)
{
  omniPy::PyRefHolder ev(PyObject_GetAttrString(pyenum, (char*)"_v"));
  return omniPy::getULongVal(ev);
}


static CORBA::Policy_ptr
createPolicyObject(PortableServer::POA_ptr poa, PyObject* pypolicy)
{
  if (!pypolicy)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);

  CORBA::Policy_ptr policy = 0;
  
  omniPy::PyRefHolder pyptype(PyObject_GetAttrString(pypolicy,
                                                     (char*)"_policy_type"));

  omniPy::PyRefHolder pyvalue(PyObject_GetAttrString(pypolicy,
                                                     (char*)"_value"));

  if (pyptype.valid() && pyvalue.valid()) {

    switch (omniPy::getULongVal(pyptype)) {

    case 16: // ThreadPolicy
      policy = poa->
	create_thread_policy((PortableServer::
			      ThreadPolicyValue)
			     getEnumVal(pyvalue));
      break;

    case 17: // LifespanPolicy
      policy = poa->
	create_lifespan_policy((PortableServer::
				LifespanPolicyValue)
			       getEnumVal(pyvalue));
      break;

    case 18: // IdUniquenessPolicy
      policy = poa->
	create_id_uniqueness_policy((PortableServer::
				     IdUniquenessPolicyValue)
				    getEnumVal(pyvalue));
      break;

    case 19: // IdAssignmentPolicy
      policy = poa->
	create_id_assignment_policy((PortableServer::
				     IdAssignmentPolicyValue)
				    getEnumVal(pyvalue));
      break;

    case 20: // ImplicitActivationPolicy
      policy = poa->
	create_implicit_activation_policy((PortableServer::
					   ImplicitActivationPolicyValue)
					  getEnumVal(pyvalue));
      break;

    case 21: // ServantRetentionPolicy
      policy = poa->
	create_servant_retention_policy((PortableServer::
					 ServantRetentionPolicyValue)
					getEnumVal(pyvalue));
      break;

    case 22: // RequestProcessingPolicy
      policy = poa->
	create_request_processing_policy((PortableServer::
					  RequestProcessingPolicyValue)
					 getEnumVal(pyvalue));
      break;

    case 37: // BidirectionalPolicy
      policy = new BiDirPolicy::BidirectionalPolicy(omniPy::getULongVal(pyvalue));
      break;

    case 0x41545402: // EndPointPublishPolicy
      {
        if (!PyList_Check(pyvalue))
          THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO,
                             omniPy::formatString("EndPointPublishPolicy value "
                                                  "should be a list of "
                                                  "strings, not %r", "O",
                                                  pyvalue->ob_type));

        CORBA::ULong     len = PyList_GET_SIZE(pyvalue);
        CORBA::StringSeq seq(len);
        seq.length(len);

        for (CORBA::ULong idx=0; idx != len; ++idx) {
          PyObject* item = PyList_GET_ITEM(pyvalue, idx);

          if (!String_Check(item))
            THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO,
                               omniPy::formatString("EndPointPublishPolicy "
                                                    "value should be a list of "
                                                    "strings, not list of %r",
                                                    "O", item->ob_type));

          seq[idx] = CORBA::string_dup(String_AsString(item));
        }
        try {
          policy = new omniPolicy::EndPointPublishPolicy(seq);
        }
        catch (CORBA::PolicyError& ex) {
          THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO,
                             omniPy::formatString("Invalid "
                                                  "EndPointPublishPolicy "
                                                  "value %r",
                                                  "O", pyvalue.obj()));
        }
      }
      break;

    default:
      {
        // Is there a function registered in _omnipy.policyFns?
        PyObject* pyf = PyDict_GetItem(omniPy::py_policyFns, pyptype);
        
        if (pyf) {

#if (PY_VERSION_HEX <= 0x03000000)
          if (PyCObject_Check(pyf)) {
            omniORBpyPolicyFn f = (omniORBpyPolicyFn)PyCObject_AsVoidPtr(pyf);
            policy = f(pyvalue);
          }
          else {
            omniORB::logs(1, "WARNING: Entry in _omnipy.policyFns is not a "
                          "PyCObject.");
          }
#else
          if (PyCapsule_CheckExact(pyf)) {
            omniORBpyPolicyFn f = (omniORBpyPolicyFn)PyCapsule_GetPointer(pyf,
                                                                          0);
            policy = f(pyvalue);
          }
          else {
            omniORB::logs(1, "WARNING: Entry in _omnipy.policyFns is not a "
                          "PyCapsule.");
          }
#endif
        }
      }
    }
  }

  if (!policy || CORBA::is_nil(policy)) {
    PyErr_Clear();
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO,
                       omniPy::formatString("Invalid Policy object %r", "O",
                                            pypolicy));
  }
  return policy;
}
  

extern "C" {

  static void
  pyPOA_dealloc(PyPOAObject* self)
  {
    {
      omniPy::InterpreterUnlocker _u;
      CORBA::release(self->poa);
      CORBA::release(self->base.obj);
    }
    Py_TYPE(self)->tp_free((PyObject*)self);
  }

  static PyObject*
  pyPOA_create_POA(PyPOAObject* self, PyObject* args)
  {
    char*     name;
    PyObject* pyPM;
    PyObject* pypolicies;

    if (!PyArg_ParseTuple(args, (char*)"sOO",
			  &name, &pyPM, &pypolicies))
      return 0;

    RAISE_PY_BAD_PARAM_IF(!(PyList_Check(pypolicies) ||
			    PyTuple_Check(pypolicies)),
			  BAD_PARAM_WrongPythonType);

    PortableServer::POAManager_ptr pm;

    if (pyPM == Py_None)
      pm = PortableServer::POAManager::_nil();
    else {
      RAISE_PY_BAD_PARAM_IF(!omniPy::pyPOAManagerCheck(pyPM),
                            BAD_PARAM_WrongPythonType);

      pm = ((PyPOAManagerObject*)pyPM)->pm;
    }

    try {
      // Convert Python Policy objects to C++ Policy objects
      CORBA::ULong numpolicies = PySequence_Length(pypolicies);
      CORBA::PolicyList policies(numpolicies);
      policies.length(numpolicies);

      for (CORBA::ULong i=0; i < numpolicies; i++) {
	policies[i] = createPolicyObject(self->poa,
					 PySequence_GetItem(pypolicies, i));
      }

      // Call the function
      PortableServer::POA_ptr child;
      {
	omniPy::InterpreterUnlocker _u;
	child = self->poa->create_POA(name, pm, policies);
      }
      return omniPy::createPyPOAObject(child);
    }
    catch (PortableServer::POA::AdapterAlreadyExists& ex) {
      return raisePOAException("AdapterAlreadyExists");
    }
    catch (PortableServer::POA::InvalidPolicy& ex) {
      return raisePOAException("InvalidPolicy",
                               Py_BuildValue((char*)"(i)", ex.index));
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  pyPOA_find_POA(PyPOAObject* self, PyObject* args)
  {
    char* name;
    int   activate_it;

    if (!PyArg_ParseTuple(args, (char*)"si", &name, &activate_it))
      return 0;

    // Call the function
    try {
      PortableServer::POA_ptr found;
      {
	omniPy::InterpreterUnlocker _u;
	found = self->poa->find_POA(name, activate_it);
      }
      return omniPy::createPyPOAObject(found);
    }
    catch (PortableServer::POA::AdapterNonExistent& ex) {
      return raisePOAException("AdapterNonExistent");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject* pyPOA_destroy(PyPOAObject* self, PyObject* args)
  {
    int eth, wait;

    if (!PyArg_ParseTuple(args, (char*)"ii", &eth, &wait))
      return 0;

    // Call the function
    try {
      {
	omniPy::InterpreterUnlocker _u;
	self->poa->destroy(eth, wait);
      }
      Py_INCREF(Py_None);
      return Py_None;
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  pyPOA_get_the_name(PyPOAObject* self, PyObject* args)
  {
    try {
      char*     name   = self->poa->the_name();
      PyObject* pyname = String_FromString(name);
      CORBA::string_free(name);
      return pyname;
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  pyPOA_get_the_parent(PyPOAObject* self, PyObject* args)
  {
    try {
      return omniPy::createPyPOAObject(self->poa->the_parent());
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  pyPOA_get_the_children(PyPOAObject* self, PyObject* args)
  {
    try {
      PortableServer::POAList_var pl = self->poa->the_children();

      PyObject* pypl = PyList_New(pl->length());

      for (CORBA::ULong i=0; i < pl->length(); i++)
	PyList_SetItem(pypl, i,
		       omniPy::createPyPOAObject(PortableServer::POA::
						 _duplicate(pl[i])));
      return pypl;
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  pyPOA_get_the_POAManager(PyPOAObject* self, PyObject* args)
  {
    try {
      return omniPy::createPyPOAManagerObject(self->poa->the_POAManager());
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  pyPOA_get_the_activator(PyPOAObject* self, PyObject* args)
  {
    try {
      PyObject*         pyobj   = 0;
      CORBA::Object_ptr lobjref = 0;
      const char*       repoId;
      {
	omniPy::InterpreterUnlocker u;
	{
	  PortableServer::AdapterActivator_var act = self->poa->the_activator();

	  if (CORBA::is_nil(act)) {
	    lobjref = 0;
	  }
	  else if (act->_NP_is_pseudo()) {
	    try {
	      u.lock();
	      pyobj = omniPy::getPyObjectForLocalObject(act);
	      u.unlock();
	    }
	    catch (...) {
	      u.unlock();
	      throw;
	    }
	  }
	  else {
	    repoId  = act->_PR_getobj()->_mostDerivedRepoId();
	    lobjref = omniPy::makeLocalObjRef(repoId, act);
	  }
	}
      }
      if (pyobj) {
	return pyobj;
      }
      else if (lobjref) {
	return omniPy::createPyCorbaObjRef(0, lobjref);
      }
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  pyPOA_set_the_activator(PyPOAObject* self, PyObject* args)
  {
    PyObject *pyact;
    if (!PyArg_ParseTuple(args, (char*)"O", &pyact)) return 0;

    CORBA::Boolean local = 0;

    CORBA::Object_ptr actobj = omniPy::getObjRef(pyact);
    if (!actobj) {
      actobj = omniPy::getLocalObjectForPyObject(pyact);
      local = 1;
    }

    RAISE_PY_BAD_PARAM_IF(!actobj, BAD_PARAM_WrongPythonType);

    try {
      omniPy::InterpreterUnlocker _u;

      // Ensure local object is released while interpreter lock is not held
      CORBA::Object_var localobj;
      if (local)
	localobj = actobj;

      PortableServer::AdapterActivator_var act =
	PortableServer::AdapterActivator::_narrow(actobj);

      if (CORBA::is_nil(act))
	OMNIORB_THROW(INV_OBJREF, INV_OBJREF_InterfaceMisMatch,
		      CORBA::COMPLETED_NO);

      self->poa->the_activator(act);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject*
  pyPOA_get_servant_manager(PyPOAObject* self, PyObject* args)
  {
    try {
      PyObject*         pyobj   = 0;
      CORBA::Object_ptr lobjref = 0;
      const char*       repoId;
      {
	omniPy::InterpreterUnlocker u;
	{
	  PortableServer::ServantManager_var
            sm = self->poa->get_servant_manager();

	  if (CORBA::is_nil(sm)) {
	    lobjref = 0;
	  }
	  else if (sm->_NP_is_pseudo()) {
	    try {
	      u.lock();
	      pyobj = omniPy::getPyObjectForLocalObject(sm);
	      u.unlock();
	    }
	    catch (...) {
	      u.unlock();
	      throw;
	    }
	  }
	  else {
	    repoId  = sm->_PR_getobj()->_mostDerivedRepoId();
	    lobjref = omniPy::makeLocalObjRef(repoId, sm);
	  }
	}
      }
      if (pyobj) {
	return pyobj;
      }
      else if (lobjref) {
	return omniPy::createPyCorbaObjRef(0, lobjref);
      }
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    catch (PortableServer::POA::WrongPolicy& ex) {
      return raisePOAException("WrongPolicy");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  pyPOA_set_servant_manager(PyPOAObject* self, PyObject* args)
  {
    PyObject *pymgr;
    if (!PyArg_ParseTuple(args, (char*)"O", &pymgr)) return 0;

    CORBA::Boolean local = 0;

    CORBA::Object_ptr mgrobj = omniPy::getObjRef(pymgr);
    if (!mgrobj) {
      mgrobj = omniPy::getLocalObjectForPyObject(pymgr);
      local = 1;
    }

    RAISE_PY_BAD_PARAM_IF(!mgrobj, BAD_PARAM_WrongPythonType);

    try {
      omniPy::InterpreterUnlocker _u;

      // Ensure local object is released while interpreter lock is not held
      CORBA::Object_var localobj;
      if (local)
	localobj = mgrobj;

      PortableServer::ServantManager_var mgr =
	PortableServer::ServantManager::_narrow(mgrobj);

      if (CORBA::is_nil(mgr))
	OMNIORB_THROW(INV_OBJREF, INV_OBJREF_InterfaceMisMatch,
		      CORBA::COMPLETED_NO);

      self->poa->set_servant_manager(mgr);
    }
    catch (PortableServer::POA::WrongPolicy& ex) {
      return raisePOAException("WrongPolicy");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject*
  pyPOA_get_servant(PyPOAObject* self, PyObject* args)
  {
    try {
      PortableServer::Servant servant;
      omniPy::Py_omniServant* pyos;
      {
	omniPy::InterpreterUnlocker _u;
	servant = self->poa->get_servant();
	pyos = (omniPy::Py_omniServant*)servant->
	                        _ptrToInterface(omniPy::string_Py_omniServant);
      }
      if (pyos) {
	PyObject* pyservant = pyos->pyServant();
	pyos->_locked_remove_ref();
	return pyservant;
      }
      else {
	// Oh dear -- the servant is C++, not Python. OBJ_ADAPTER
	// seems the most sensible choice of exception.
	{
	  omniPy::InterpreterUnlocker _u;
	  servant->_remove_ref();
	}
	OMNIORB_THROW(OBJ_ADAPTER,
		      OBJ_ADAPTER_IncompatibleServant, CORBA::COMPLETED_NO);
      }
    }
    catch (PortableServer::POA::NoServant& ex) {
      return raisePOAException("NoServant");
    }
    catch (PortableServer::POA::WrongPolicy& ex) {
      return raisePOAException("WrongPolicy");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    return 0;
  }

  static PyObject*
  pyPOA_set_servant(PyPOAObject* self, PyObject* args)
  {
    PyObject* pyServant;

    if (!PyArg_ParseTuple(args, (char*)"O", &pyServant)) return 0;

    omniPy::Py_omniServant* pyos = omniPy::getServantForPyObject(pyServant);
    RAISE_PY_BAD_PARAM_IF(!pyos, BAD_PARAM_WrongPythonType);

    omniPy::PYOSReleaseHelper _r(pyos);

    try {
      {
	omniPy::InterpreterUnlocker _u;
	self->poa->set_servant(pyos);
      }
      Py_INCREF(Py_None);
      return Py_None;
    }
    catch (PortableServer::POA::WrongPolicy& ex) {
      return raisePOAException("WrongPolicy");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  pyPOA_activate_object(PyPOAObject* self, PyObject* args)
  {
    PyObject* pyServant;

    if (!PyArg_ParseTuple(args, (char*)"O", &pyServant)) return 0;

    omniPy::Py_omniServant* pyos = omniPy::getServantForPyObject(pyServant);
    RAISE_PY_BAD_PARAM_IF(!pyos, BAD_PARAM_WrongPythonType);

    omniPy::PYOSReleaseHelper _r(pyos);

    try {
      PortableServer::ObjectId_var oid;
      {
	omniPy::InterpreterUnlocker _u;
	oid = self->poa->activate_object(pyos);
      }
      return RawString_FromStringAndSize((const char*)oid->NP_data(),
                                         oid->length());
    }
    catch (PortableServer::POA::ServantAlreadyActive& ex) {
      return raisePOAException("ServantAlreadyActive");
    }
    catch (PortableServer::POA::WrongPolicy& ex) {
      return raisePOAException("WrongPolicy");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    return 0;
  }

  static PyObject*
  pyPOA_activate_object_with_id(PyPOAObject* self, PyObject* args)
  {
    PyObject*  pyServant;
    char*      oidstr;
    Py_ssize_t oidlen;

    if (!PyArg_ParseTuple(args, (char*)"s#O",
			  &oidstr, &oidlen, &pyServant))
      return 0;

    omniPy::Py_omniServant* pyos = omniPy::getServantForPyObject(pyServant);
    RAISE_PY_BAD_PARAM_IF(!pyos, BAD_PARAM_WrongPythonType);

    omniPy::PYOSReleaseHelper _r(pyos);

    try {
      PortableServer::ObjectId oid(oidlen, oidlen, (CORBA::Octet*)oidstr, 0);
      {
	omniPy::InterpreterUnlocker _u;
	self->poa->activate_object_with_id(oid, pyos);
      }
      Py_INCREF(Py_None);
      return Py_None;
    }
    catch (PortableServer::POA::ServantAlreadyActive& ex) {
      return raisePOAException("ServantAlreadyActive");
    }
    catch (PortableServer::POA::ObjectAlreadyActive& ex) {
      return raisePOAException("ObjectAlreadyActive");
    }
    catch (PortableServer::POA::WrongPolicy& ex) {
      return raisePOAException("WrongPolicy");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  pyPOA_deactivate_object(PyPOAObject* self, PyObject* args)
  {
    char*      oidstr;
    Py_ssize_t oidlen;

    if (!PyArg_ParseTuple(args, (char*)"s#", &oidstr, &oidlen))
      return 0;

    try {
      PortableServer::ObjectId oid(oidlen, oidlen, (CORBA::Octet*)oidstr, 0);
      {
	omniPy::InterpreterUnlocker _u;
	self->poa->deactivate_object(oid);
      }
      Py_INCREF(Py_None);
      return Py_None;
    }
    catch (PortableServer::POA::ObjectNotActive& ex) {
      return raisePOAException("ObjectNotActive");
    }
    catch (PortableServer::POA::WrongPolicy& ex) {
      return raisePOAException("WrongPolicy");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  pyPOA_create_reference(PyPOAObject* self, PyObject* args)
  {
    char* repoId;

    if (!PyArg_ParseTuple(args, (char*)"s", &repoId))
      return 0;

    try {
      CORBA::Object_ptr lobjref;
      {
	omniPy::InterpreterUnlocker _u;
	{
	  CORBA::Object_var objref;
	  objref  = self->poa->create_reference(repoId);
	  lobjref = omniPy::makeLocalObjRef(repoId, objref);
	}
      }
      return omniPy::createPyCorbaObjRef(repoId, lobjref);
    }
    catch (PortableServer::POA::WrongPolicy& ex) {
      return raisePOAException("WrongPolicy");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  pyPOA_create_reference_with_id(PyPOAObject* self, PyObject* args)
  {
    char*      oidstr;
    Py_ssize_t oidlen;
    char*      repoId;

    if (!PyArg_ParseTuple(args, (char*)"s#s",
			  &oidstr, &oidlen, &repoId))
      return 0;

    try {
      PortableServer::ObjectId oid(oidlen, oidlen, (CORBA::Octet*)oidstr, 0);
      CORBA::Object_ptr lobjref;
      {
	omniPy::InterpreterUnlocker _u;
	{
	  CORBA::Object_var objref;
	  objref  = self->poa->create_reference_with_id(oid, repoId);
	  lobjref = omniPy::makeLocalObjRef(repoId, objref);
	}
      }
      return omniPy::createPyCorbaObjRef(repoId, lobjref);
    }
    catch (PortableServer::POA::WrongPolicy& ex) {
      return raisePOAException("WrongPolicy");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  pyPOA_servant_to_id(PyPOAObject* self, PyObject* args)
  {
    PyObject* pyServant;

    if (!PyArg_ParseTuple(args, (char*)"O", &pyServant)) return 0;

    omniPy::Py_omniServant* pyos = omniPy::getServantForPyObject(pyServant);
    RAISE_PY_BAD_PARAM_IF(!pyos, BAD_PARAM_WrongPythonType);

    omniPy::PYOSReleaseHelper _r(pyos);

    try {
      PortableServer::ObjectId_var oid;
      {
	omniPy::InterpreterUnlocker _u;
	oid = self->poa->servant_to_id(pyos);
      }
      return RawString_FromStringAndSize((const char*)oid->NP_data(),
                                         oid->length());
    }
    catch (PortableServer::POA::ServantNotActive& ex) {
      return raisePOAException("ServantNotActive");
    }
    catch (PortableServer::POA::WrongPolicy& ex) {
      return raisePOAException("WrongPolicy");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    return 0;
  }

  static PyObject*
  pyPOA_servant_to_reference(PyPOAObject* self, PyObject* args)
  {
    PyObject* pyServant;

    if (!PyArg_ParseTuple(args, (char*)"O", &pyServant)) return 0;

    omniPy::Py_omniServant* pyos = omniPy::getServantForPyObject(pyServant);
    RAISE_PY_BAD_PARAM_IF(!pyos, BAD_PARAM_WrongPythonType);

    omniPy::PYOSReleaseHelper _r(pyos);

    try {
      CORBA::Object_ptr lobjref;
      {
	omniPy::InterpreterUnlocker _u;
	{
	  CORBA::Object_var objref;
	  objref  = self->poa->servant_to_reference(pyos);
	  lobjref = omniPy::makeLocalObjRef(pyos->_mostDerivedRepoId(),objref);
	}
      }
      return omniPy::createPyCorbaObjRef(pyos->_mostDerivedRepoId(), lobjref);
    }
    catch (PortableServer::POA::ServantNotActive& ex) {
      return raisePOAException("ServantNotActive");
    }
    catch (PortableServer::POA::WrongPolicy& ex) {
      return raisePOAException("WrongPolicy");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  pyPOA_reference_to_servant(PyPOAObject* self, PyObject* args)
  {
    PyObject* pyobjref;

    if (!PyArg_ParseTuple(args, (char*)"O", &pyobjref)) return 0;

    CORBA::Object_ptr objref = omniPy::getObjRef(pyobjref);

    RAISE_PY_BAD_PARAM_IF(!objref, BAD_PARAM_WrongPythonType);

    try {
      PortableServer::Servant servant;
      omniPy::Py_omniServant* pyos;
      {
	omniPy::InterpreterUnlocker _u;
	servant = self->poa->reference_to_servant(objref);
	pyos = (omniPy::Py_omniServant*)servant->
                                _ptrToInterface(omniPy::string_Py_omniServant);
      }
      if (pyos) {
	PyObject* pyservant = pyos->pyServant();
	pyos->_locked_remove_ref();
	return pyservant;
      }
      else {
	// Oh dear -- the servant is C++, not Python. OBJ_ADAPTER
	// seems the most sensible choice of exception.
	{
	  omniPy::InterpreterUnlocker _u;
	  servant->_remove_ref();
	}
	OMNIORB_THROW(OBJ_ADAPTER,
		      OBJ_ADAPTER_IncompatibleServant, CORBA::COMPLETED_NO);
      }
    }
    catch (PortableServer::POA::ObjectNotActive& ex) {
      return raisePOAException("ObjectNotActive");
    }
    catch (PortableServer::POA::WrongAdapter& ex) {
      return raisePOAException("WrongAdapter");
    }
    catch (PortableServer::POA::WrongPolicy& ex) {
      return raisePOAException("WrongPolicy");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    return 0;
  }

  static PyObject*
  pyPOA_reference_to_id(PyPOAObject* self, PyObject* args)
  {
    PyObject* pyobjref;

    if (!PyArg_ParseTuple(args, (char*)"O", &pyobjref)) return 0;

    CORBA::Object_ptr objref = omniPy::getObjRef(pyobjref);

    RAISE_PY_BAD_PARAM_IF(!objref, BAD_PARAM_WrongPythonType);

    try {
      PortableServer::ObjectId_var oid;
      {
	omniPy::InterpreterUnlocker _u;
	oid = self->poa->reference_to_id(objref);
      }
      return RawString_FromStringAndSize((const char*)oid->NP_data(),
                                         oid->length());
    }
    catch (PortableServer::POA::WrongAdapter& ex) {
      return raisePOAException("WrongAdapter");
    }
    catch (PortableServer::POA::WrongPolicy& ex) {
      return raisePOAException("WrongPolicy");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    return 0;
  }

  static PyObject*
  pyPOA_id_to_servant(PyPOAObject* self, PyObject* args)
  {
    char*      oidstr;
    Py_ssize_t oidlen;

    if (!PyArg_ParseTuple(args, (char*)"s#", &oidstr, &oidlen))
      return 0;

    try {
      PortableServer::ObjectId oid(oidlen, oidlen, (CORBA::Octet*)oidstr, 0);
      PortableServer::Servant  servant;
      omniPy::Py_omniServant*  pyos;
      {
	omniPy::InterpreterUnlocker _u;
	servant = self->poa->id_to_servant(oid);
	pyos = (omniPy::Py_omniServant*)servant->
                                _ptrToInterface(omniPy::string_Py_omniServant);
      }
      if (pyos) {
	PyObject* pyservant = pyos->pyServant();
	pyos->_locked_remove_ref();
	return pyservant;
      }
      else {
	// Oh dear -- the servant is C++, not Python. OBJ_ADAPTER
	// seems the most sensible choice of exception.
	{
	  omniPy::InterpreterUnlocker _u;
	  servant->_remove_ref();
	}
	OMNIORB_THROW(OBJ_ADAPTER,
		      OBJ_ADAPTER_IncompatibleServant, CORBA::COMPLETED_NO);
      }
    }
    catch (PortableServer::POA::ObjectNotActive& ex) {
      return raisePOAException("ObjectNotActive");
    }
    catch (PortableServer::POA::WrongPolicy& ex) {
      return raisePOAException("WrongPolicy");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    return 0;
  }

  static PyObject*
  pyPOA_id_to_reference(PyPOAObject* self, PyObject* args)
  {
    char*      oidstr;
    Py_ssize_t oidlen;

    if (!PyArg_ParseTuple(args, (char*)"s#", &oidstr, &oidlen))
      return 0;

    try {
      PortableServer::ObjectId oid(oidlen, oidlen, (CORBA::Octet*)oidstr, 0);
      CORBA::Object_ptr lobjref;
      const char* mdri;
      {
	omniPy::InterpreterUnlocker _u;
	{
	  CORBA::Object_var objref;
	  objref  = self->poa->id_to_reference(oid);
	  mdri    = objref->_PR_getobj()->_mostDerivedRepoId();
	  lobjref = omniPy::makeLocalObjRef(mdri, objref);
	}
      }
      return omniPy::createPyCorbaObjRef(0, lobjref);
    }
    catch (PortableServer::POA::ObjectNotActive& ex) {
      return raisePOAException("ObjectNotActive");
    }
    catch (PortableServer::POA::WrongPolicy& ex) {
      return raisePOAException("WrongPolicy");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    return 0;
  }


  ////////////////////////////////////////////////////////////////////////////
  // Python method table                                                    //
  ////////////////////////////////////////////////////////////////////////////

  static PyMethodDef pyPOA_methods[] = {
    {(char*)"create_POA",
     (PyCFunction)pyPOA_create_POA,
     METH_VARARGS},

    {(char*)"find_POA",
     (PyCFunction)pyPOA_find_POA,
     METH_VARARGS},

    {(char*)"destroy",
     (PyCFunction)pyPOA_destroy,
     METH_VARARGS},

    {(char*)"_get_the_name",
     (PyCFunction)pyPOA_get_the_name,
     METH_NOARGS},

    {(char*)"_get_the_parent",
     (PyCFunction)pyPOA_get_the_parent,
     METH_NOARGS},

    {(char*)"_get_the_children",
     (PyCFunction)pyPOA_get_the_children,
     METH_NOARGS},

    {(char*)"_get_the_POAManager",
     (PyCFunction)pyPOA_get_the_POAManager,
     METH_NOARGS},

    {(char*)"_get_the_activator",
     (PyCFunction)pyPOA_get_the_activator,
     METH_NOARGS},

    {(char*)"_set_the_activator",
     (PyCFunction)pyPOA_set_the_activator,
     METH_VARARGS},

    {(char*)"get_servant_manager",
     (PyCFunction)pyPOA_get_servant_manager,
     METH_NOARGS},

    {(char*)"set_servant_manager",
     (PyCFunction)pyPOA_set_servant_manager,
     METH_VARARGS},

    {(char*)"get_servant",
     (PyCFunction)pyPOA_get_servant,
     METH_NOARGS},

    {(char*)"set_servant",
     (PyCFunction)pyPOA_set_servant,
     METH_VARARGS},

    {(char*)"activate_object",
     (PyCFunction)pyPOA_activate_object,
     METH_VARARGS},

    {(char*)"activate_object_with_id",
     (PyCFunction)pyPOA_activate_object_with_id,
     METH_VARARGS},

    {(char*)"deactivate_object",
     (PyCFunction)pyPOA_deactivate_object,
     METH_VARARGS},

    {(char*)"create_reference",
     (PyCFunction)pyPOA_create_reference,
     METH_VARARGS},

    {(char*)"create_reference_with_id",
     (PyCFunction)pyPOA_create_reference_with_id,
     METH_VARARGS},

    {(char*)"servant_to_id",
     (PyCFunction)pyPOA_servant_to_id,
     METH_VARARGS},

    {(char*)"servant_to_reference",
     (PyCFunction)pyPOA_servant_to_reference,
     METH_VARARGS},

    {(char*)"reference_to_servant",
     (PyCFunction)pyPOA_reference_to_servant,
     METH_VARARGS},

    {(char*)"reference_to_id",
     (PyCFunction)pyPOA_reference_to_id,
     METH_VARARGS},

    {(char*)"id_to_servant",
     (PyCFunction)pyPOA_id_to_servant,
     METH_VARARGS},

    {(char*)"id_to_reference",
     (PyCFunction)pyPOA_id_to_reference,
     METH_VARARGS},

    {NULL,NULL}
  };

  static PyTypeObject PyPOAType = {
    PyVarObject_HEAD_INIT(0,0)
    (char*)"_omnipy.PyPOAObject",      /* tp_name */
    sizeof(PyPOAObject),               /* tp_basicsize */
    0,                                 /* tp_itemsize */
    (destructor)pyPOA_dealloc,         /* tp_dealloc */
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
    (char*)"Internal POA object",      /* tp_doc */
    0,                                 /* tp_traverse */
    0,                                 /* tp_clear */
    0,                                 /* tp_richcompare */
    0,                                 /* tp_weaklistoffset */
    0,                                 /* tp_iter */
    0,                                 /* tp_iternext */
    pyPOA_methods,                     /* tp_methods */
  };
}

PyObject*
omniPy::createPyPOAObject(PortableServer::POA_ptr poa)
{
  PyPOAObject* self = PyObject_New(PyPOAObject, &PyPOAType);
  self->poa = poa;
  self->base.obj = CORBA::Object::_duplicate(poa);

  omniPy::PyRefHolder args(PyTuple_New(1));
  PyTuple_SET_ITEM(args, 0, (PyObject*)self);

  return PyObject_CallObject(omniPy::pyPOAClass, args);
}

CORBA::Boolean
omniPy::pyPOACheck(PyObject* pyobj)
{
  return pyobj->ob_type == &PyPOAType;
}

void
omniPy::initPOAFunc(PyObject* d)
{
  PyPOAType.tp_base = omniPy::PyObjRefType;
  int r = PyType_Ready(&PyPOAType);
  OMNIORB_ASSERT(r == 0);
}
