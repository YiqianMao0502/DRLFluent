// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniInterceptors.h         Created on: 22/09/2000
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2006-2013 Apasphere Ltd
//    Copyright (C) 2000 AT&T Laboratories, Cambridge
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

#ifndef __OMNIINTERCEPTORS_H__
#define __OMNIINTERCEPTORS_H__

#ifndef __CORBA_H_EXTERNAL_GUARD__
#include <omniORB4/CORBA.h>
#endif

class omniLocalIdentity;
class omniCallDescriptor;
class omniServant;

OMNI_NAMESPACE_BEGIN(omni)

class omniInterceptorP;
class giopRope;
class giopStrand;
class GIOP_S;
class GIOP_C;
class orbServer;

class omniInterceptors {
public:

  //////////////////////////////////////////////////////////////////
  class encodeIOR_T {
  public:

    class info_T {
    public:
      omniIOR&                 ior;
      IIOP::ProfileBody&       iiop;
      const omniIORHints&      hints;
      CORBA::Boolean           default_only;

      info_T(omniIOR& i, IIOP::ProfileBody& body,
	     const omniIORHints& h, CORBA::Boolean b) :
         ior(i), iiop(body), hints(h), default_only(b) {}

    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };

  //////////////////////////////////////////////////////////////////
  class decodeIOR_T {
  public:

    class info_T {
    public:
      const IIOP::ProfileBody& iiop;
      omniIOR&                 ior;
      CORBA::Boolean           has_iiop_body;

      info_T(const IIOP::ProfileBody& body, omniIOR& i, CORBA::Boolean b) :
         iiop(body), ior(i), has_iiop_body(b) {}

    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  class clientOpenConnection_T {
  public:

    class info_T {
    public:
      GIOP_C&        giop_c;
      CORBA::Boolean reject;
      const char*    why;

      info_T(GIOP_C& c) :
        giop_c(c), reject(0), why(0) {}

      // Accessors for connection details
      const char* operation();
      const char* myaddress();
      const char* peeraddress();
      const char* peeridentity();
      void*       peerdetails();

    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  class clientSendRequest_T {
  public:

    class info_T {
    public:
      GIOP_C&                  giop_c;
      IOP::ServiceContextList  service_contexts;

      info_T(GIOP_C& c) :
        giop_c(c), service_contexts(5) {}

      // Accessors for operation / connection details
      const char* operation();
      const char* myaddress();
      const char* peeraddress();
      const char* peeridentity();
      void*       peerdetails();

    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  class clientReceiveReply_T {
  public:

    class info_T {
    public:
      GIOP_C&                  giop_c;
      IOP::ServiceContextList& service_contexts;
      
      info_T(GIOP_C& c, IOP::ServiceContextList& sc) :
	giop_c(c), service_contexts(sc) {}

      // Accessors for operation / connection details
      const char* operation();
      const char* myaddress();
      const char* peeraddress();
      const char* peeridentity();
      void*       peerdetails();

    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  class serverAcceptConnection_T {
  public:

    class info_T {
    public:
      giopStrand&    strand;
      CORBA::Boolean reject;
      const char*    why;

      info_T(giopStrand& s) : 
	strand(s), reject(0), why(0) {}

      // Accessors for connection details
      const char* myaddress();
      const char* peeraddress();
      const char* peeridentity();
      void*       peerdetails();

    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  class serverReceiveRequest_T {
  public:

    class info_T {
    public:
      GIOP_S& giop_s;

      info_T(GIOP_S& s) : 
	giop_s(s) {}

      // Accessors for operation / connection details
      const char* operation();
      const char* myaddress();
      const char* peeraddress();
      const char* peeridentity();
      void*       peerdetails();

    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  class serverSendReply_T {
  public:

    class info_T {
    public:
      GIOP_S& giop_s;
      
      info_T(GIOP_S& s) :
	giop_s(s) {}

      // Accessors for operation / connection details
      const char* operation();
      const char* myaddress();
      const char* peeraddress();
      const char* peeridentity();
      void*       peerdetails();

    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  class serverSendException_T {
  public:

    class info_T {
    public:
      GIOP_S&                  giop_s;
      const CORBA::Exception*  exception;

      info_T(GIOP_S& s, const CORBA::Exception* e) :
	giop_s(s), exception(e) {}

      // Accessors for operation / connection details
      const char* operation();
      const char* myaddress();
      const char* peeraddress();
      const char* peeridentity();
      void*       peerdetails();

    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  class createRope_T {
  public:
    
    class info_T {
    public:
      const giopAddressList& addrlist;
      omniIOR::IORInfo*      ior_info;
      giopRope*&             rope;

      info_T(const giopAddressList& a, omniIOR::IORInfo* i, giopRope*& r) :
        addrlist(a), ior_info(i), rope(r) {}
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  class createIdentity_T {
  public:
    
    class info_T {
    public:
      omniIOR*            ior;
      const char*         targetRepoId;
      omniIdentity*&      invoke_handle;
      CORBA::Boolean      held_internalLock;

      info_T(omniIOR* i, const char* t, omniIdentity*& id, CORBA::Boolean b) :
	ior(i), targetRepoId(t), invoke_handle(id), held_internalLock(b) {}
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  class createORBServer_T {
  public:
    
    class info_T {
    public:
      omnivector<orbServer*>& servers;

      info_T(omnivector<orbServer*>& s) : servers(s) {}
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  class createPolicy_T {
  public:
    
    class info_T {
    public:
      CORBA::PolicyType  type;
      const CORBA::Any&  value;
      CORBA::Policy_ptr& policy;

      info_T(CORBA::PolicyType t, const CORBA::Any& v, CORBA::Policy_ptr& p) :
	type(t), value(v), policy(p) {}
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  class createThread_T {
  public:
    
    class info_T {
    public:
      virtual void run() = 0;
      virtual omni_thread* self() = 0;
    };

    typedef void (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  class assignUpcallThread_T {
  public:
    
    class info_T {
    public:
      virtual void run() = 0;
      virtual omni_thread* self() = 0;
    };

    typedef void (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };

  //////////////////////////////////////////////////////////////////
  class assignAMIThread_T {
  public:
    
    class info_T {
    public:
      virtual void run() = 0;
      virtual omni_thread* self() = 0;
    };

    typedef void (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };

  //////////////////////////////////////////////////////////////////
  class invokeLocalCall_T {
  public:
    typedef void (*interceptFunc)(omniCallDescriptor*, omniServant*);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  encodeIOR_T                encodeIOR;
  decodeIOR_T                decodeIOR;
  clientOpenConnection_T     clientOpenConnection;
  clientSendRequest_T        clientSendRequest;
  clientReceiveReply_T       clientReceiveReply;
  serverAcceptConnection_T   serverAcceptConnection;
  serverReceiveRequest_T     serverReceiveRequest;
  serverSendReply_T          serverSendReply;
  serverSendException_T      serverSendException;
  createRope_T               createRope;
  createIdentity_T           createIdentity;
  createORBServer_T          createORBServer;
  createPolicy_T             createPolicy;
  createThread_T             createThread;
  assignUpcallThread_T       assignUpcallThread;
  assignAMIThread_T          assignAMIThread;
  invokeLocalCall_T          invokeLocalCall;

  //////////////////////////////////////////////////////////////////
  friend class omni_interceptor_initialiser;

 private:
  omniInterceptors();
  ~omniInterceptors();
};

OMNI_NAMESPACE_END(omni)


#endif // __OMNIINTERCEPTORS_H__
