// AMI pollable set echo example that uses the pollable set to watch
// for a DII call as well as an AMI call.
//
// Use as a client to echo/eg2_impl
//
// Usage: echo_dii_pollable_set <object reference>
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

    // Make an asynchronous call
    AMI_EchoPoller_var poller = echoref->sendp_echoString("Hello async");

    // Make a deferred synchronous DII call
    CORBA::Request_var req = echoref->_request("echoString");
    CORBA::String_var  arg = (const char*) "Hello DII";

    req->add_in_arg() <<= arg;
    req->set_return_type(CORBA::_tc_string);
    req->send_deferred();


    // Create PollableSet
    CORBA::PollableSet_var pset = poller->create_pollable_set();

    // Get DIIPollable and add to set
    CORBA::DIIPollable_var dii_pollable = pset->create_dii_pollable();
    pset->add_pollable(dii_pollable);

    try {
      while (1) {
        cout << pset->number_left() << " pollers left" << endl;

        CORBA::Pollable_var pollable = pset->get_ready_pollable(2000);
        AMI_EchoPoller*     poller   = AMI_EchoPoller::_downcast(pollable);

        if (poller) {
          CORBA::String_var result;
          poller->echoString(0, result.out());
          cout << "AMI call returned: " << (const char*)result << endl;
        }
        else if (req->poll_response()) {
          const char* dii_ret;
          req->return_value() >>= dii_ret;
          cout << "DII call returned: " << dii_ret << endl;
        }
        else {
          cout << "Unexpected poller returned from pollable set!" << endl;
        }
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
