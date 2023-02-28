READ ME FIRST!

This is omniORB 4.2.4

omniORB is copyright Apasphere Ltd, AT&T Laboratories Cambridge and
others. It is free software. The programs in omniORB are distributed
under the GNU General Public Licence as published by the Free Software
Foundation. See the file COPYING for copying permission of these
programs. The libraries in omniORB are distributed under the GNU
Lesser General Public Licence. See the file COPYING.LIB for copying
permission of these libraries.

We impose no restriction on the use of the IDL compiler output. The
stub code produced by the IDL compiler is not considered a derived
work of it.


README files
============

README.win32.txt - contains important information on building and
                   using omniORB on Windows NT and Windows 95.

README.unix.txt  - contains important information on building and
                   using omniORB on Unix / Linux platforms, including
                   cross-compilation

Other readmes live the the readmes/ subdirectory. See if there is one
for your platform.



Technical Highlights
====================

omniORB is an Object Request Broker (ORB) which implements
specification 2.6 of the Common Object Request Broker Architecture
(CORBA).

- C++ and Python language bindings.

- Full support for the Portable Object Adapter (POA)

- Support for the Interoperable Naming Service (INS)

- Internet Inter-ORB Protocol (IIOP 1.2) is used as the native
  protocol.

- The omniORB runtime is fully multithreaded. It uses native platform
  thread support encapsulated with a small class library, omnithread,
  to abstract away from differences in native thread APIs.

- TypeCode and type Any are supported.

- DynAny is supported.

- The Dynamic Invocation and Dynamic Skeleton interfaces are supported.

- Valuetype and abstract interfaces are supported.

- Asynchronous Method Invocation (AMI) supported, including both the
  polling and callback models.

- A COS Naming Service, omniNames, is provided.

- Many platforms are supported, including most Unix platforms and
  Windows.

  It should be straightforward to port omniORB to any platform which
  supports POSIX style threads, BSD style sockets and has a decent C++
  compiler which supports thread-safe exceptions. See readmes/PORTING
  for more information.

- It has been successfully tested for interoperability via IIOP with
  other ORBs.


Related projects
================

For some very dynamic situations, you may wish to use an Interface
Repository. The omniifr project provides one for omniORB:

    https://github.com/omniorb/omniifr


Installation	
============

Installation instructions are provided in the following files:

- README.unix.txt  for all Unix / Linux platforms
- README.win32.txt for Windows, both 32 and 64 bit.


Documentation
=============

- omniORB user guides are located in the ./doc directory. They are
  available in PDF and HTML formats, as well as LaTeX source.

- README files are provided throughout the distribution tree. They
  provide specific information about the directories and files, tools
  and examples.  You may want to explore the tree to become familiar
  with the information provided in the README files.


Examples
========

- Example programs that demonstrate the use of omniORB are located in
  ./src/examples.


Recommended Additional Documentation
====================================

omniORB is based on several OMG specifications. They may be downloaded
from

  https://www.omg.org/spec/CORBA/


Contact and support
===================

If you have any queries, suggestions and problems in using omniORB,
the best place to ask is on the omniORB mailing list. See here for
subscription details and archives:

  https://www.omniorb.net/list.html


Commercial support is available for omniORB. For details, see

  https://www.omniorb-support.com/


For general queries and discussion about CORBA, try the newsgroup
comp.object.corba.
