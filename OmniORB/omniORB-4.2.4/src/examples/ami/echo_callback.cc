// AMI callback echo example. Use as a client to echo/eg2_impl
//
// Usage: echo_callback <object reference>
//

#include <echo_ami.hh>

#ifdef HAVE_STD
#  include <iostream>
#  include <fstream>
   using namespace std;
#else
#  include <iostream.h>
#endif

class EchoHandler_i : public virtual POA_AMI_EchoHandler
{
public:
  inline EchoHandler_i() {};
  virtual ~EchoHandler_i() {}

  void echoString(const char* ami_return_val);
  void echoString_excep(Messaging::ExceptionHolder* excep_holder);
};

void
EchoHandler_i::echoString(const char* ami_return_val)
{
  cout << "echoString return: " << ami_return_val << endl;
}

void
EchoHandler_i::echoString_excep(Messaging::ExceptionHolder* excep_holder)
{
  try {
    excep_holder->raise_exception();
  }
  catch (CORBA::Exception& ex) {
    cout << "echoString exception: " << ex._name() << endl;
  }
}


//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    if (argc != 2) {
      cerr << "usage: echo_callback <object reference>" << endl;
      return 1;
    }

    // Get reference to Echo object
    CORBA::Object_var obj = orb->string_to_object(argv[1]);
    Echo_var echoref = Echo::_narrow(obj);

    if (CORBA::is_nil(echoref)) {
      cerr << "Can't narrow reference to type Echo (or it was nil)." << endl;
      return 1;
    }

    // Resolve and activate the Root POA
    CORBA::Object_var poa_obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(poa_obj);
    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();

    // Create an EchoHandler object to receive the callback
    PortableServer::Servant_var<EchoHandler_i> handler_svt = new EchoHandler_i;
    AMI_EchoHandler_var handler = handler_svt->_this();

    // Make the asynchronous call
    echoref->sendc_echoString(handler, "Hello async!");

    // Wait for a while -- the call should complete within this time
    omni_thread::sleep(2);

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
