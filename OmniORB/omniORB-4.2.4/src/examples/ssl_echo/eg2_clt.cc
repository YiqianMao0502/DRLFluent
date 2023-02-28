// eg2_clt.cc - This is the source code of example 2 used in Chapter 2
//              "The Basics" of the omniORB user guide.
//
//              This is the client. The object reference is given as a
//              stringified IOR on the command line.
//
// Usage: eg2_clt <object reference>
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


static void hello(Echo_ptr e)
{
  CORBA::String_var src = (const char*) "Hello!";

  CORBA::String_var dest = e->echoString(src);

  cout << "I said, \"" << (char*)src << "\"." << endl
       << "The Echo object replied, \"" << (char*)dest <<"\"." << endl;
}

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  sslContext::certificate_authority_file = "root.pem";
  sslContext::key_file = "client.pem";
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


  try {
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    if (argc != 2) {
      cerr << "usage:  eg2_clt <object reference>" << endl;
      return 1;
    }

    CORBA::Object_var obj = orb->string_to_object(argv[1]);

    Echo_var echoref = Echo::_narrow(obj);

    if (CORBA::is_nil(echoref)) {
      cerr << "Can't narrow reference to type Echo (or it was nil)." << endl;
      return 1;
    }

    for (CORBA::ULong count=0; count<10; count++) 
      hello(echoref);

    orb->destroy();
  }
  catch (CORBA::TRANSIENT&) {
    cerr << "Caught system exception TRANSIENT -- unable to contact the "
         << "server." << endl;
  }
  catch (CORBA::SystemException& ex) {
    cerr << "Caught a CORBA::" << ex._name() << endl;
  }
  catch (CORBA::Exception& ex) {
    cerr << "Caught CORBA::Exception: " << ex._name() << endl;
  }
  return 0;
}
