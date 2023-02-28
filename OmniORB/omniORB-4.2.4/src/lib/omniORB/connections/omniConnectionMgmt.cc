// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniConnectionMgmt.cc      Created on: 2006/07/10
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2006 Apasphere Ltd.
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
// Description:
//    Proprietary omniORB connection management API

#include <omniORB4/CORBA.h>
#include <omniORB4/omniInterceptors.h>
#include <omniORB4/omniConnectionMgmt.h>
#include <omniORB4/omniConnectionData.hh>
#include <initialiser.h>
#include <giopRope.h>
#include <giopStrand.h>
#include <giopStrandFlags.h>
#include <giopStream.h>
#include <GIOP_C.h>
#include <GIOP_S.h>
#include <objectAdapter.h>
#include <remoteIdentity.h>


OMNI_NAMESPACE_BEGIN(omni)

//
// restrictedGiopRope is a giopRope subclass that knows what its
// connection id is, and sets the maximum number of connections.
//

class restrictedGiopRope : public giopRope {
public:
  restrictedGiopRope(const giopAddressList& addrlist,
                     omniIOR::IORInfo*      info,
		     CORBA::ULong           connection_id,
		     CORBA::ULong           max_connections,
		     CORBA::Boolean         data_batch,
		     CORBA::Boolean         permit_interleaved)

    : giopRope(addrlist, info),
      pd_connection_id(connection_id),
      pd_data_batch(data_batch)
  {
    if (max_connections < maxStrands())
      maxStrands(max_connections);

    oneCallPerConnection(!permit_interleaved);
  }

  virtual IOP_C* acquireClient(const omniIOR*      ior,
			       const CORBA::Octet* key,
			       CORBA::ULong        keysize,
			       omniCallDescriptor* cd);
  // Override giopRope acquireClient, so we can set the connection
  // batching flags if need be.

protected:
  CORBA::Boolean match(const giopAddressList&, omniIOR::IORInfo* info) const;
  // Override giopRope to additionally check connection id.

private:
  CORBA::ULong   pd_connection_id;
  CORBA::Boolean pd_data_batch;
};


//
// RestrictedInfo is an IORExtraInfo subclass for holding restricted
// connection information in an omniIOR.
//

class RestrictedInfo : public omniIOR::IORExtraInfo {
public:
  RestrictedInfo()
    : omniIOR::IORExtraInfo(omniConnectionData::TAG_RESTRICTED_CONNECTION)
  {}
  virtual ~RestrictedInfo() {}

  omniConnectionData::ComponentData data;

  static inline RestrictedInfo*
  get(omniIOR::IORInfo* info)
  {
    omniIOR::IORExtraInfoList& extras = info->extraInfo();
    CORBA::ULong idx;
    for (idx=0; idx < extras.length(); ++idx) {
      if (extras[idx]->compid == omniConnectionData::TAG_RESTRICTED_CONNECTION)
        break;
    }
    if (idx == extras.length()) {
      // Tag not found
      return 0;
    }
    return (RestrictedInfo*)extras[idx];
  }

private:
  RestrictedInfo(const RestrictedInfo&);
  RestrictedInfo& operator=(const RestrictedInfo&);
  // Not implemented
};


//
// giopRope::acquireClient specialisation
//

IOP_C*
restrictedGiopRope::acquireClient(const omniIOR*      ior,
				  const CORBA::Octet* key,
				  CORBA::ULong        keysize,
				  omniCallDescriptor* cd)
{
  IOP_C* iop_c = giopRope::acquireClient(ior, key, keysize, cd);

  if (pd_data_batch) {
    GIOP_C*     giop_c = (GIOP_C*)iop_c;
    giopStrand& strand = giop_c->strand();
    strand.flags |= GIOPSTRAND_ENABLE_TRANSPORT_BATCHING;
  }
  return iop_c;
}


//
// giopRope::match specialisation
//

CORBA::Boolean
restrictedGiopRope::match(const giopAddressList& addrlist,
                          omniIOR::IORInfo*      info) const
{
  CORBA::Boolean m = giopRope::match(addrlist, info);

  if (!m)
    return 0;

  RestrictedInfo* rinfo = RestrictedInfo::get(info);
  if (!rinfo)
    return 0;

  return rinfo->data.connection_id == pd_connection_id;
}

//
// decodeIOR interceptor handles the restricted connection component
//

static CORBA::Boolean
decodeIORInterceptor(omniInterceptors::decodeIOR_T::info_T& iinfo)
{
  const IIOP::ProfileBody& iiop = iinfo.iiop;
  omniIOR&                 ior  = iinfo.ior;

  const IOP::MultipleComponentProfile& components = iiop.components;

  CORBA::ULong index;

  try {
    for (index=0; index < components.length(); ++index) {
      if (components[index].tag ==
	  omniConnectionData::TAG_RESTRICTED_CONNECTION) {

	omniORB::logs(25, "Found a restricted connection IOR component.");

	const IOP::TaggedComponent& c = components[index];
	cdrEncapsulationStream      stream(c.component_data, 1);

	RestrictedInfo* rinfo = new RestrictedInfo();
	rinfo->data <<= stream;

	if (rinfo->data.version != 1 && omniORB::trace(5)) {
	  omniORB::logger log;
	  log << "Warning: received restricted connection IOR component "
	      << "with unknown version " << (int)rinfo->data.version << ".\n";
	}

	// Add the information to the omniIOR's extra info list.
        omniIOR::IORInfo* info  = ior.getIORInfo();
        CORBA::ULong      flags = GIOPSTRAND_CONNECTION_MANAGEMENT;

        if (rinfo->data.flags & omniConnectionData::COMP_DATA_BATCH)
          flags |= GIOPSTRAND_ENABLE_TRANSPORT_BATCHING;

        info->flags(info->flags() | flags);

	omniIOR::IORExtraInfoList& infolist = info->extraInfo();
	CORBA::ULong i = infolist.length();
	infolist.length(i+1);
	infolist[i] = (omniIOR::IORExtraInfo*)rinfo;
	return 1;
      }
    }
  }
  catch (CORBA::SystemException&) {
    omniORB::logs(10, "Invalid restricted connection IOR component "
		  "encountered.");
  }
  return 1;
}


//
// createRope interceptor
//

static CORBA::Boolean
createRopeInterceptor(omniInterceptors::createRope_T::info_T& iinfo)
{
  const giopAddressList& addrlist = iinfo.addrlist;
  omniIOR::IORInfo*      info     = iinfo.ior_info;

  RestrictedInfo* rinfo = RestrictedInfo::get(info);
  if (!rinfo)
    return 1;

  CORBA::ULong   connection_id      = rinfo->data.connection_id;
  CORBA::ULong   max_connections    = rinfo->data.max_connections;
  CORBA::Boolean data_batch         =
    rinfo->data.flags & omniConnectionData::COMP_DATA_BATCH;

  CORBA::Boolean permit_interleaved =
    rinfo->data.flags & omniConnectionData::COMP_PERMIT_INTERLEAVED;

  if (omniORB::trace(25)) {
    omniORB::logger log;
    log << "Create rope using restricted connection id "
        << connection_id << ".\n";
  }

  iinfo.rope = new restrictedGiopRope(addrlist, info,
                                      connection_id,
                                      max_connections,
                                      data_batch,
                                      permit_interleaved);
  return 1;
}


//
// clientSendRequest interceptor to set the service context
//

static CORBA::Boolean
clientSendRequestInterceptor(omniInterceptors::
			     clientSendRequest_T::info_T& iinfo)
{
  GIOP_C&     giop_c = iinfo.giop_c;
  giopStrand& strand = giop_c.strand();

  if (!strand.first_call) {
    // Only consider the connection on the first call
    return 1;
  }

  RestrictedInfo* rinfo = RestrictedInfo::get(giop_c.ior()->getIORInfo());
  if (!rinfo) {
    // Not restricted -- return from interceptor
    return 1;
  }

  CORBA::ULong   connection_id = rinfo->data.connection_id;
  CORBA::ULong   max_threads   = rinfo->data.max_threads;
  CORBA::Boolean hold_open     =
    rinfo->data.flags & omniConnectionData::COMP_SERVER_HOLD_OPEN;

  // Build service context
  omniConnectionData::ServiceData data;
  data.version       = 1;
  data.flags         = hold_open ? omniConnectionData::SVC_HOLD_OPEN : 0;
  data.connection_id = connection_id;
  data.max_threads   = max_threads;

  cdrEncapsulationStream stream(CORBA::ULong(0), CORBA::Boolean(1));
  data >>= stream;

  // Copy the encapsulation contents into the service context data.
  CORBA::ULong len = iinfo.service_contexts.length();

  iinfo.service_contexts.length(len+1);
  iinfo.service_contexts[len].context_id = (omniConnectionData::
					    SVC_RESTRICTED_CONNECTION);
  stream.setOctetSeq(iinfo.service_contexts[len].context_data);

  if (omniORB::trace(25)) {
    omniORB::logger log;
    log << "Send restricted connection service context: ("
	<< connection_id << "," << data.flags << "," << max_threads << ").\n";
  }
  return 1;
}


//
// serverReceiveRequest interceptor receives and handles the service context.
//

static CORBA::Boolean
serverReceiveRequestInterceptor(omniInterceptors::
				serverReceiveRequest_T::info_T& iinfo)
{
  IOP::ServiceContextList& contexts = iinfo.giop_s.service_contexts();
  CORBA::ULong len = contexts.length();
  CORBA::ULong i;

  // Search for the context we handle
  for (i=0; i < len; ++i) {
    if (contexts[i].context_id ==
        omniConnectionData::SVC_RESTRICTED_CONNECTION) {

      cdrEncapsulationStream stream(contexts[i].context_data, 1);

      omniConnectionData::ServiceData data;
      data <<= stream;
      if (data.version != 1 && omniORB::trace(5)) {
	omniORB::logger log;
	log << "Warning: received restricted connection service context "
	    << "with unknown version " << (int)data.version << ".\n";
      }

      if (omniORB::trace(25)) {
	omniORB::logger log;
	log << "Receive restricted connection service context: ("
	    << data.connection_id << "," << data.flags << ","
	    << data.max_threads << ").\n";
      }
      
      // Handle connection options
      giopStrand& strand = iinfo.giop_s.strand();
      if (data.flags & omniConnectionData::SVC_HOLD_OPEN)
	strand.flags |= GIOPSTRAND_HOLD_OPEN;

      giopConnection* connection = strand.connection;
      if (data.max_threads < (CORBA::ULong)connection->max_workers())
	connection->max_workers(data.max_threads);

      return 1;
    }
  }
  return 1;
}


//
// addComponentData function adds restricted connection data to an
// IOR's tagged components.
//

static void
addComponentData(IOP::MultipleComponentProfile& components,
		 CORBA::ULong      		connection_id,
		 CORBA::ULong      		max_connections,
		 CORBA::ULong      		max_threads,
		 CORBA::Boolean    		data_batch,
		 CORBA::Boolean                 permit_interleaved,
		 CORBA::Boolean    		server_hold_open)
{
  CORBA::ULong index;

  // If there is an existing component, replace it
  for (index=0; index < components.length(); ++index) {
    if (components[index].tag ==
	omniConnectionData::TAG_RESTRICTED_CONNECTION) {
      omniORB::logs(25, "Replacing existing TAG_RESTRICTED_CONNECTION "
		    "component.");
      break;
    }
  }
  // If no existing component, add a new one
  if (index == components.length())
    components.length(index+1);

  cdrEncapsulationStream stream(CORBA::ULong(0), CORBA::Boolean(1));

  omniConnectionData::ComponentData data;
  data.version = 1;
  data.flags   =
    ((data_batch         ? omniConnectionData::COMP_DATA_BATCH         : 0) |
     (permit_interleaved ? omniConnectionData::COMP_PERMIT_INTERLEAVED : 0) |
     (server_hold_open   ? omniConnectionData::COMP_SERVER_HOLD_OPEN   : 0));

  data.connection_id   = connection_id;
  data.max_connections = max_connections;
  data.max_threads     = max_threads;

  data >>= stream;

  components[index].tag = omniConnectionData::TAG_RESTRICTED_CONNECTION;
  stream.setOctetSeq(components[index].component_data);
}

OMNI_NAMESPACE_END(omni)


OMNI_USING_NAMESPACE(omni)


//
// makeRestrictedReference public function
//

CORBA::Object_ptr
omniConnectionMgmt::
makeRestrictedReference(CORBA::Object_ptr obj,
			CORBA::ULong      connection_id,
			CORBA::ULong      max_connections,
			CORBA::ULong      max_threads,
			CORBA::Boolean    data_batch,
			CORBA::Boolean    permit_interleaved,
			CORBA::Boolean    server_hold_open)
{
  if (CORBA::is_nil(obj))
    return obj;

  if (obj->_NP_is_pseudo())
    return CORBA::Object::_duplicate(obj);

  // Get the original object reference's IOR
  omniIOR_var orig_ior(obj->_PR_getobj()->_getIOR());

  if (omniORB::trace(20)) {
    omniORB::logger log;
    log << "Restrict object reference type '" << orig_ior->repositoryID()
	<< "' to connection id " << connection_id << ".\n";
  }

  // Copy its profiles
  IOP::TaggedProfileList_var profiles(
    new IOP::TaggedProfileList(orig_ior->iopProfiles()));

  CORBA::Long selected = orig_ior->addr_selected_profile_index();

  CORBA::Boolean done = 0;

  if (selected >= 0 && profiles[selected].tag == IOP::TAG_INTERNET_IOP) {
    IIOP::ProfileBody iiop;
    IIOP::unmarshalProfile(profiles[selected], iiop);
    if (iiop.version.major > 1 || iiop.version.minor > 0) {
      // IIOP 1.1 and up have tagged components within the IIOP
      // profile, so we can add the proprietary compoment to it.
      addComponentData(iiop.components,
		       connection_id, max_connections, max_threads,
		       data_batch, permit_interleaved, server_hold_open);
      IIOP::encodeProfile(iiop, profiles[selected]);
      done = 1;
    }
  }
  if (!done) {
    // There was no suitable IIOP profile to add the component. We
    // look for a MultipleComponentProfile to use.
    IOP::MultipleComponentProfile components;

    CORBA::ULong index;
    for (index=0; index < profiles->length(); ++index) {
      if (profiles[index].tag == IOP::TAG_MULTIPLE_COMPONENTS) {
	IIOP::unmarshalMultiComponentProfile(profiles[index], components);
	break;
      }
    }
    if (index == profiles->length()) {
      // No MultipleComponentProfile yet, so cause one to be added.
      profiles->length(index+1);
    }
    addComponentData(components,
		     connection_id, max_connections, max_threads,
		     data_batch, permit_interleaved, server_hold_open);
    
    // Marshal the MultipleComponentProfile into the profiles, either
    // replacing the existing one or adding a new one.
    IIOP::encodeMultiComponentProfile(components, profiles[index]);
  }

  // Finally, we build a new omniIOR with the modified profiles...
  omniIOR* ior = new omniIOR(CORBA::string_dup(orig_ior->repositoryID()),
			     profiles._retn());
  
  // ...and build a new object reference for it.
  omniObjRef* new_ref = omni::createObjRef(CORBA::Object::_PD_repoId, ior, 0);
  return (CORBA::Object_ptr)new_ref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}



//
// Module initialiser. Called during CORBA::ORB_init().
//

class omniConnectionMgmt_initialiser : public omniInitialiser {
public:
  void attach() {
    omniORB::logs(2, "omniConnectionMgmt extension activated.");

    // Register interceptors
    omniInterceptors* interceptors = omniORB::getInterceptors();

    interceptors->createRope          .add(createRopeInterceptor);
    interceptors->decodeIOR           .add(decodeIORInterceptor);
    interceptors->clientSendRequest   .add(clientSendRequestInterceptor);
    interceptors->serverReceiveRequest.add(serverReceiveRequestInterceptor);
  }
  void detach() {
  }
};

static omniConnectionMgmt_initialiser the_omniConnectionMgmt_initialiser;


//
// init() just registers the initialiser.
//

void
omniConnectionMgmt::init()
{
  omniInitialiser::install(&the_omniConnectionMgmt_initialiser);
}
