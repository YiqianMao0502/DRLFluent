// AMI pollable set echo example. Use as a client to echo/eg2_impl
//
// Usage: echo_pollable_set <object reference>
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
      cerr << "usage: echo_pollable_set <object reference>" << endl;
      return 1;
    }

    // Get reference to Echo object
    CORBA::Object_var obj = orb->string_to_object(argv[1]);
    Echo_var echoref = Echo::_narrow(obj);

    if (CORBA::is_nil(echoref)) {
      cerr << "Can't narrow reference to type Echo (or it was nil)." << endl;
      return 1;
    }

    // Make some asynchronous calls
    AMI_EchoPoller_var poller1 = echoref->sendp_echoString("Hello async 1!");
    AMI_EchoPoller_var poller2 = echoref->sendp_echoString("Hello async 2!");

    // Create PollableSet containing both pollers
    CORBA::PollableSet_var pset = poller1->create_pollable_set();
    pset->add_pollable(poller2);

    try {
      while (1) {
        cout << pset->number_left() << " pollers left" << endl;

        CORBA::Pollable_var pollable = pset->get_ready_pollable(2000);
        AMI_EchoPoller*     poller   = AMI_EchoPoller::_downcast(pollable);

        CORBA::String_var result;
        poller->echoString(0, result.out());
        cout << "The call returned: " << (const char*)result << endl;
      }
    }
    catch (CORBA::PollableSet::NoPossiblePollable&) {
      cout << "No possible pollable." << endl;
    }
    catch (CORBA::TIMEOUT&) {
      cout << "Timeout" << endl;
    }

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
