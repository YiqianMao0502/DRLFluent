// anyExample_impl.cc - This is the source code of the example used in
//                      Chapter 9 "Type Any and TypeCode" of the omniORB
//                      users guide.
//
//                      This is the object implementation.
//
// Usage: anyExample_impl
//
//        On startup, the object reference is printed to cout as a
//        stringified IOR. This string should be used as the argument to 
//        anyExample_clt.
//

#include <anyExample.hh>

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif

class anyExample_i : public POA_anyExample {
public:
  inline anyExample_i() {}
  virtual ~anyExample_i() {}
  virtual CORBA::Any* testOp(const CORBA::Any& a);
};


CORBA::Any* anyExample_i::testOp(const CORBA::Any& a)
{
  cout << "Any received, containing: " << endl;

#ifndef NO_FLOAT
  CORBA::Double d;
#endif

  CORBA::Long l;
  const char* str;

  testStruct* tp;


  if (a >>= l) {
    cout << "Long: " << l << endl;
  }
#ifndef NO_FLOAT
  else if (a >>= d) {
    cout << "Double: " << (double)d << endl;
  }
#endif
  else if (a >>= str) {
    cout << "String: " << str << endl;
  }
  else if (a >>= tp) {
    cout << "testStruct: l: " << tp->l << endl;
    cout << "            s: " << tp->s << endl;
  }
  else {
    cout << "Unknown value." << endl;
  }

  CORBA::Any* ap = new CORBA::Any;

  *ap <<= (CORBA::ULong) 314;

  cout << "Returning Any containing: ULong: 314\n" << endl;
  return ap;
}

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

    anyExample_i* myobj = new anyExample_i();

    PortableServer::ObjectId_var myobjid = poa->activate_object(myobj);

    obj = myobj->_this();
    CORBA::String_var sior(orb->object_to_string(obj));
    cout << (char*)sior << endl;

    myobj->_remove_ref();

    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();

    orb->run();
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
