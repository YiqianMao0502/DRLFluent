//
// servant_locator.cc
//
// Usage: servant_locator
//
//        On startup, the object reference is printed to cout as a
//        stringified IOR. This string should be used as the argument to 
//        eg2_clt (from the <omniORB>/src/examples/echo/ directory).
//
//        A message of 'shutdown' causes the server (you guessed it) to
//        shutdown.
//

#include <echo.hh>

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif


static CORBA::ORB_var orb;

//////////////////////////////////////////////////////////////////////

class Echo_i : public POA_Echo
{
public:
  virtual ~Echo_i() { cout << "Echo_i::~Echo_i()" << endl; }

  char* echoString(const char* mesg)
  {
    cout << "Echo_i::echoString(\"" << mesg << "\")" << endl;

    if (!strcmp(mesg, "shutdown"))
      orb->shutdown(0);

    return CORBA::string_dup(mesg);
  }
};

//////////////////////////////////////////////////////////////////////

class MyLocator_i : public PortableServer::ServantLocator
{
public:
  virtual ~MyLocator_i()
  { cout << "MyLocator_i::~MyLocator_i()" << endl; }

  PortableServer::Servant
  preinvoke(const PortableServer::ObjectId& oid,
	    PortableServer::POA_ptr adapter, const char* operation,
	    PortableServer::ServantLocator::Cookie& the_cookie)
  {
    cout << "MyLocator_i::preinvoke()" << endl;
    return new Echo_i;
  }

  void
  postinvoke(const PortableServer::ObjectId& oid,
	     PortableServer::POA_ptr adapter, const char* operation,
	     PortableServer::ServantLocator::Cookie the_cookie,
	     PortableServer::Servant the_servant)
  {
    cout << "MyLocator_i::postinvoke()" << endl;
    the_servant->_remove_ref();
  }
};

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {
    orb = CORBA::ORB_init(argc, argv);

    {
      // Get hold of the root poa, and activate it.
      CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
      PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj);
      PortableServer::POAManager_var poa_man = root_poa->the_POAManager();
      poa_man->activate();

      // Create a new poa to hold our echo objects.
      CORBA::PolicyList pl;
      pl.length(2);
      pl[0] = root_poa->create_request_processing_policy(PortableServer::USE_SERVANT_MANAGER);
      pl[1] = root_poa->create_servant_retention_policy(PortableServer::NON_RETAIN);

      PortableServer::POA_var poa = root_poa->create_POA("mypoa", poa_man, pl);

      // Activate a servant locator in the root poa.
      MyLocator_i* sl = new MyLocator_i;

      // Register the servant activator with our new poa.
      poa->set_servant_manager(sl);

      // Print out a reference to an object.
      CORBA::Object_var ref = poa->create_reference("IDL:Echo:1.0");
      CORBA::String_var sior(orb->object_to_string(ref));
      cout << (char*) sior << endl;

      orb->run();
    }
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
