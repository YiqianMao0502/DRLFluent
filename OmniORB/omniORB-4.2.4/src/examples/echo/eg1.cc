// eg1.cc - This is the source code of example 1 used in Chapter 2
//          "The Basics" of the omniORB user guide.
//
//          In this example, both the object implementation and the
//          client are in the same process.
//
// Usage: eg1
//

#include <echo.hh>

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif

// This is the object implementation.

class Echo_i : public POA_Echo
{
public:
  inline Echo_i() {}
  virtual ~Echo_i() {}
  virtual char* echoString(const char* mesg);
};


char* Echo_i::echoString(const char* mesg)
{
  // Memory management rules say we must return a newly allocated
  // string.
  return CORBA::string_dup(mesg);
}


//////////////////////////////////////////////////////////////////////

// This function acts as a client to the object.

static void hello(Echo_ptr e)
{
  if( CORBA::is_nil(e) ) {
    cerr << "hello: The object reference is nil!" << endl;
    return;
  }

  CORBA::String_var src = (const char*) "Hello!";
  // String literals are (char*) rather than (const char*) on some
  // old compilers.  Thus it is essential to cast to (const char*)
  // here to ensure that the string is copied, so that the
  // CORBA::String_var does not attempt to 'delete' the string
  // literal.

  CORBA::String_var dest = e->echoString(src);

  cout << "I said, \"" << (char*)src << "\"." << endl
       << "The Echo object replied, \"" << (char*)dest <<"\"." << endl;
}

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {
    // Initialise the ORB.
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    // Obtain a reference to the root POA.
    CORBA::Object_var       obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

    // We allocate the servant (implementation object) on the heap.
    // The servant is reference counted. We start out holding a
    // reference, and when the object is activated, the POA holds
    // another reference. The PortableServer::Servant_var<> template
    // automatically releases our reference when it goes out of scope.
    PortableServer::Servant_var<Echo_i> myecho = new Echo_i();

    // Activate the object. This tells the POA that this object is
    // ready to accept requests.
    PortableServer::ObjectId_var myechoid = poa->activate_object(myecho);

    // Obtain a reference to the object.
    Echo_var myechoref = myecho->_this();

    // Obtain a POAManager, and tell the POA to start accepting
    // requests on its objects.
    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();

    // Do the client-side call.
    hello(myechoref);

    // Clean up all the resources.
    orb->destroy();
  }
  catch (CORBA::SystemException& ex) {
    cerr << "Caught CORBA::" << ex._name() << endl;
  }
  catch (CORBA::Exception& ex) {
    cerr << "Caught CORBA::Exception: " << ex._name() << endl;
  }
  return 0;
}
