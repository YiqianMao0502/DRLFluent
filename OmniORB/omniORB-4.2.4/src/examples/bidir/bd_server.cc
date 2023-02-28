#include <echo_callback.hh>

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif


static CORBA::ORB_ptr orb;
static int            dying = 0;
static int            num_active_servers = 0;
static omni_mutex     mu;
static omni_condition sigobj(&mu);

//////////////////////////////////////////////////////////////////////

// A thread object used to server clients registered
// using the cb::Server::register() method.

class server_thread : public omni_thread {
public:
  inline server_thread(cb::CallBack_ptr client, const char* mesg, int period)
    : pd_client(cb::CallBack::_duplicate(client)),
      pd_mesg(mesg), pd_period(period) {}

  virtual void run(void* arg);

private:
  cb::CallBack_var  pd_client;
  CORBA::String_var pd_mesg;
  int               pd_period;
};


void
server_thread::run(void* arg)
{
  try {
    while (!dying) {
      omni_thread::sleep(pd_period);
      pd_client->call_back(pd_mesg);
    }
  }
  catch (...) {
    cout << "cb_server: Lost a client!" << endl;
  }

  cout << "cb_server: Worker thread is exiting." << endl;

  mu.lock();
  int do_signal = --num_active_servers == 0;
  mu.unlock();
  if (do_signal)  sigobj.signal();
}

//////////////////////////////////////////////////////////////////////

// Implementation of cb::Server.

class server_i : public POA_cb::Server
{
public:
  inline server_i() {}
  virtual ~server_i();

  virtual void one_time(cb::CallBack_ptr cb, const char* mesg);

  // NB. Because 'register' is a C++ keyword, we have to
  // use the prefix _cxx_ here.
  virtual void _cxx_register(cb::CallBack_ptr cb, const char* mesg,
			     CORBA::UShort period_secs);

  virtual void shutdown();
};


server_i::~server_i()
{
  cout << "bd_server: The server object has been destroyed." << endl;
}


void
server_i::one_time(cb::CallBack_ptr cb, const char* mesg)
{
  if (CORBA::is_nil(cb)) {
    cerr << "Received a nil callback.\n";
    return;
  }

  cout << "bd_server: Doing a single call-back: " << mesg << endl;

  cb->call_back(mesg);
}


void
server_i::_cxx_register(cb::CallBack_ptr cb, const char* mesg,
		      CORBA::UShort period_secs)
{
  if (CORBA::is_nil(cb)) {
    cerr << "Received a nil callback.\n";
    return;
  }

  cout << "bd_server: Starting a new worker thread" << endl;

  mu.lock();
  num_active_servers++;
  mu.unlock();

  server_thread* server = new server_thread(cb, mesg, period_secs);
  server->start();
}


void
server_i::shutdown()
{
  omni_mutex_lock sync(mu);

  if (!dying) {
    cout << "bd_server: I am being shutdown!" << endl;

    // Tell the servers to exit, and wait for them to do so.
    dying = 1;

    while (num_active_servers > 0)  sigobj.wait();

    // Shutdown the ORB (but do not wait for completion).  This also
    // causes the main thread to unblock from CORBA::ORB::run().
    orb->shutdown(0);
  }
}

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {
    orb = CORBA::ORB_init(argc, argv);

    {
      CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
      PortableServer::POA_var rootpoa = PortableServer::POA::_narrow(obj);
      PortableServer::POAManager_var pman = rootpoa->the_POAManager();
      pman->activate();

      // Create a POA with the Bidirectional policy
      CORBA::PolicyList pl;
      pl.length(1);
      CORBA::Any a;
      a <<= BiDirPolicy::BOTH;
      pl[0] = orb->create_policy(BiDirPolicy::BIDIRECTIONAL_POLICY_TYPE, a);

      PortableServer::POA_var poa = rootpoa->create_POA("bidir", pman, pl);

      server_i* myserver = new server_i();
      PortableServer::ObjectId_var oid = poa->activate_object(myserver);
      obj = myserver->_this();
      myserver->_remove_ref();

      CORBA::String_var sior(orb->object_to_string(obj));
      cout << (char*) sior << endl;
      orb->run();
    }

    cout << "bd_server: Returned from orb->run()." << endl;
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
