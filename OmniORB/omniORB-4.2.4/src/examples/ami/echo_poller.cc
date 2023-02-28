// AMI poller echo example. Use as a client to echo/eg2_impl
//
// Usage: echo_poller <object reference>
//

#include <echo_ami.hh>

#ifdef HAVE_STD
#  include <iostream>
#  include <fstream>
   using namespace std;
#else
#  include <iostream.h>
#endif

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    if (argc != 2) {
      cerr << "usage: echo_poller <object reference>" << endl;
      return 1;
    }

    // Get reference to Echo object
    CORBA::Object_var obj = orb->string_to_object(argv[1]);
    Echo_var echoref = Echo::_narrow(obj);

    if (CORBA::is_nil(echoref)) {
      cerr << "Can't narrow reference to type Echo (or it was nil)." << endl;
      return 1;
    }

    // Make the asynchronous call
    AMI_EchoPoller_var poller = echoref->sendp_echoString("Hello async!");

    // Poll using is_ready
    while (!poller->is_ready(100))
      cout << "Not ready" << endl;

    CORBA::String_var result;
    poller->echoString(0, result.out());

    cout << "The call returned: " << (const char*)result << endl;

    // Make another call
    poller = echoref->sendp_echoString("Hello again async!");

    // Poll with type-specific method
    while (1) {
      try {
	poller->echoString(100, result.out());
	break;
      }
      catch (CORBA::TIMEOUT&) {
	cout << "Not ready" << endl;
      }
    }
    cout << "The second call returned: " << (const char*)result << endl;

    orb->destroy();
  }
  catch(CORBA::TRANSIENT&) {
    cerr << "Caught system exception TRANSIENT -- unable to contact the "
         << "server." << endl;
  }
  catch(CORBA::SystemException& ex) {
    cerr << "Caught a CORBA::" << ex._name() << endl;
  }
  catch(CORBA::Exception& ex) {
    cerr << "Caught CORBA::Exception: " << ex._name() << endl;
  }
  return 0;
}
