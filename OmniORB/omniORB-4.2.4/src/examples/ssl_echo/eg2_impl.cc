// eg2_impl.cc - This is the source code of example 2 used in Chapter 2
//               "The Basics" of the omniORB user guide.
//
//               This is the object implementation.
//
// Usage: eg2_impl
//
//        On startup, the object reference is printed to cout as a
//        stringified IOR. This string should be used as the argument to
//        eg2_clt.
//

#include <sys/stat.h>
#include <echo.hh>
#include <omniORB4/sslContext.h>

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif


class Echo_i : public POA_Echo
{
public:
  inline Echo_i() {}
  virtual ~Echo_i() {}
  virtual char* echoString(const char* mesg);
};


char* Echo_i::echoString(const char* mesg)
{
  cout << "Upcall: " << mesg << endl;
  return CORBA::string_dup(mesg);
}

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  sslContext::certificate_authority_file = "root.pem";
  sslContext::key_file = "server.pem";
  sslContext::key_file_password = "password";

  struct stat sb;
  if (stat(sslContext::certificate_authority_file,&sb) < 0) {
    cerr << "Cannot open certificate file: "
	 << sslContext::certificate_authority_file << endl;
    return 1;
  }
  if (stat(sslContext::key_file,&sb) < 0) {
    cerr << "Cannot open key file: "
	 << sslContext::key_file << endl;
    return 1;
  }

  int    my_argc = argc + 2;
  char** my_argv = new char*[my_argc];
  memcpy(my_argv,argv,sizeof(char*)*argc);
  my_argv[my_argc - 2] = "-ORBendPoint";
  my_argv[my_argc - 1] = "giop:ssl::";

  argc = my_argc;
  argv = my_argv;

  try {
    CORBA::ORB_var          orb = CORBA::ORB_init(argc, argv);
    CORBA::Object_var       obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

    PortableServer::Servant_var<Echo_i> myecho = new Echo_i();
      
    PortableServer::ObjectId_var myechoid = poa->activate_object(myecho);

    // Obtain a reference to the object, and print it out as a
    // stringified IOR.
    obj = myecho->_this();
    CORBA::String_var sior(orb->object_to_string(obj));
    cout << sior << endl;

    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();

    // Block until the ORB is shut down.
    orb->run();
  }
  catch (CORBA::SystemException& ex) {
    cerr << "Caught CORBA::" << ex._name() << endl;
  }
  catch (CORBA::Exception& ex) {
    cerr << "Caught CORBA::Exception: " << ex._name() << endl;
  }
  return 0;
}
