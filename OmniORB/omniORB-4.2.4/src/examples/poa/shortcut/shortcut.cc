// shortcut.cc
//
// This demonstrates omniORB's proprietary local call shortcut policy,
// which avoids the overhead of calling through the POA for local
// calls.

#include <shortcut.hh>

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif

class ShortcutTest_i : public POA_ShortcutTest
{
public:
  inline ShortcutTest_i() {}
  virtual ~ShortcutTest_i() {}
  virtual void test() {}
};


// Object references and servant classes are distinct types. This is a
// template function that tests invocation performance on either an
// object reference or a servant.

template <class T>
void test(T obj, const char* kind)
{
  const int count = 10000000;

  omni_time_t before, after;

  omni_thread::get_time(before);
  for (int i=0; i != count; ++i)
    obj->test();
  omni_thread::get_time(after);
  
  omni_time_t duration = after - before;

  double dd = (double)duration.s * 1000000000.0 + duration.ns;

  cout << kind << " : " << (int)(count / (dd/1000000000.0))
       << " calls per second / " << (dd / count) << " nanoseconds per call."
       << endl;
}


int main(int argc, char** argv)
{
  try {
    CORBA::ORB_var    orb = CORBA::ORB_init(argc, argv);
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");

    PortableServer::POA_var        root_poa = PortableServer::POA::_narrow(obj);
    PortableServer::POAManager_var poa_man  = root_poa->the_POAManager();
    poa_man->activate();

    // Create two POAs, one with the shortcut policy, one without
    PortableServer::POA_var normal_poa, shortcut_poa;

    CORBA::PolicyList pl;

    normal_poa = root_poa->create_POA("normal", poa_man, pl);

    pl.length(1);
    pl[0] = omniPolicy::create_local_shortcut_policy(omniPolicy::LOCAL_CALLS_SHORTCUT);

    shortcut_poa = root_poa->create_POA("shortcut", poa_man, pl);

    // Activate an object in each POA
    PortableServer::Servant_var<ShortcutTest_i> sc1 = new ShortcutTest_i();
    PortableServer::Servant_var<ShortcutTest_i> sc2 = new ShortcutTest_i();
    
    PortableServer::ObjectId_var oid;

    oid = normal_poa->activate_object(sc1);
    oid = shortcut_poa->activate_object(sc2);

    ShortcutTest_var obj1 = sc1->_this();
    ShortcutTest_var obj2 = sc2->_this();

    test(obj1.in(),  "Normal POA  ");
    test(obj2.in(),  "Shortcut POA");
    test(sc2.in(),   "Direct call ");

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
