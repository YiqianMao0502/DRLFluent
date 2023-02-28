#include <echo_callback.hh>

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif
#include <stdlib.h>


// Implementation of cb::CallBack.

class cb_i : public virtual POA_cb::CallBack
{
public:
  inline cb_i() {}
  virtual ~cb_i() {}

  virtual void call_back(const char* mesg) {
    cout << "cb_client: call_back(\"" << mesg << "\")" << endl;
  }
};

//////////////////////////////////////////////////////////////////////

static void do_single(cb::Server_ptr server, cb::CallBack_ptr cb)
{
  if (CORBA::is_nil(server)) {
    cerr << "cb_client: The server reference is nil!" << endl;
    return;
  }

  cout << "cb_client: server->one_time(call_back, \"Hello!\")" << endl;
  server->one_time(cb, "Hello!");
  cout << "cb_client: Returned." << endl;
}


static void do_register(cb::Server_ptr server, cb::CallBack_ptr cb,
			int period, int time_to_shutdown)
{
  if (CORBA::is_nil(server)) {
    cerr << "cb_client: The server reference is nil!" << endl;
    return;
  }

  cout << "cb_client: server->register(call_back, \"Hello!\", "
       << period << ")" << endl;
  server->_cxx_register(cb, "Hello!", period);
  cout << "cb_client: Returned." << endl;

  omni_thread::sleep(time_to_shutdown);
  cout << "cb_client: Finished." << endl;
}

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    if (argc != 2 && argc != 4) {
      cerr << "usage:  cb_client <object reference> [<call-back period>"
	" <time to shutdown>]" << endl;
      return 1;
    }

    {
      // Get the reference the server.
      CORBA::Object_var obj = orb->string_to_object(argv[1]);
      cb::Server_var server = cb::Server::_narrow(obj);

      // Initialise the POA.
      obj = orb->resolve_initial_references("RootPOA");
      PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);
      PortableServer::POAManager_var pman = poa->the_POAManager();
      pman->activate();

      // Register a CallBack object in this process.
      cb_i* mycallback = new cb_i();
      cb::CallBack_var callback = mycallback->_this();  // *implicit activation*
      mycallback->_remove_ref();

      if (argc == 2)
        do_single(server, callback);
      else
        do_register(server, callback, atoi(argv[2]),
                    atoi(argv[3]));

    }
    // Clean-up.  This also destroys the call-back object.
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
