This file contains information on installing, building and using
omniORB on Unix platforms.

Unless specified otherwise, the information applies to all Unix
platforms. Platform specific information is also available in separate
files.

The primary Unix platforms tested during omniORB development are Linux
and Mac OS X. It is known to work on the vast majority of other Unix
platforms.


Roadmap
=======

The directory structure of this distribution looks as follows:

./readmes                         : platform specific readme files
./doc                             : omniORB documentation
./man                             : omniORB manual pages
./mk                              : make configuration files
./config                          : configuration files for target platform
./include                         : include files
./src                             : source files
./src/lib/omnithread              : source files for the omnithread library
./src/lib/omniORB                 : source files for the ORB runtime library
./src/tool/omniidl                : source files for the IDL compiler
./src/appl/omniNames              : source files for the COS Naming service
./src/appl/utils                  : source files for utilities
./src/examples                    : source files for examples


Configuration
=============

omniORB's build is configured using the Autoconf configure script,
that tries to figure out the specifics of your machine.

Although you can run configure and make in the main omniORB source
directory, you are strongly advised to build in a different
directory. e.g.

  $ cd $OMNIORB_TOP
  $ mkdir build
  $ cd build
  $ ../configure [configure options]
  $ make
  $ make install

That keeps the build files separate from the source files, and allows
you to have several parallel builds.

configure options
-----------------

Run configure --help to get a list of configuration options. Most
options are standard Autoconf ones. The most commonly required is
--prefix, used to select the install location. The default is
/usr/local. To change it, use, for example

  ../configure --prefix=/home/fred/omni_inst

The configure script tries to figure out the location of the C and C++
compilers and Python. It will always choose gcc over the platform's
native compiler if it is available. To change the choices it makes,
use variables CC, CXX and PYTHON, e.g.:

  ../configure CXX=/usr/bin/platform_c++ PYTHON=/usr/local/bin/python2.7

There are various omniORB specific options:

  --disable-static   Disables the build of static libraries, which
                     shortens the build process.

  --enable-thread-tracing
                     Turns on thread and mutex tracing that can help
                     track down threading bugs in omniORB, but gives a
                     significant performance hit.

                     In some beta releases, thread tracing is turned
                     on by default, so you may wish to turn it off
                     with --disable-thread-tracing.
  
  --disable-ipv6     Disables support for IPv6.

  --disable-longdouble
                     Disables the CORBA::LongDouble type.

  --disable-atomic   Disables the use of atomic operations, using
                     mutexes instead.

  --with-openssl     Enable the SSL transport. If the configure script
                     does not find the OpenSSL libraries of its own
                     accord, you can specify the root directory of the
                     OpenSSL implementation: --with-openssl=/install/path

  --with-omniORB-config=
                     Location to look for the omniORB configuration
                     file. Default /etc/omniORB.cfg

  --with-omniNames-logdir=
                     Location for omniNames' log files. Default
                     /var/omninames.


Once omniORB is configured, build it with "make", then install it with
"make install". You must use GNU make.


Cross-compilation
=================

The configure script and make files support cross-compilation. The
normal build makes various tools that are used later in the build, in
particular omniidl. When cross-compiling, the tools must already be
available for your native platform. To cross-compile, use these steps:

 1. configure, build and install omniORB for your native platform. As
    recommended above, use a build subdirectory rather than building
    in the source tree.

    Make a note of the first few lines output by the configure script
    that tell you the build system type. For example
    "x86_64-unknown-linux-gnu".

 2. Add the ${prefix}/bin directory to your PATH so omniidl and other
    tools are available.

 3. In a new build directory (e.g. build-cross), run the configure
    script with arguments for cross-compiling. e.g. to compile on an
    x86-64 Linux machine, cross-compiling for ARM:

    cd build-cross
    ../configure CC=cross-cc CXX=cross-cxx \
                 --prefix=/home/example/cross-inst
                 --build=x86_64-unknown-linux-gnu \
                 --host=arm-unknown-linux-gnu

    The essential part to trigger a cross compile is to specify both
    --build and --host.

 4. Run make as usual.


Configuring the Naming service
==============================

You also have to configure the omniORB runtime and the naming service.
Consult the user guides in ./doc for details. For a quick start,
follow these steps:

    o Set the environment variable OMNINAMES_DATADIR to a directory where
      the naming service omniNames can store its data. For example:
          OMNINAMES_DATADIR=/wib/wob; export OMNINAMES_DATADIR

    o Start omniNames.
         $ omniNames -start &

    o Create a file omniORB.cfg, based on sample.cfg. It should
      contain a line of the form

        InitRef = NameService=corbaname::my.host.name

    o Set the environment variable OMNIORB_CONFIG to contain the full
      path name of the file omniORB.cfg. For example,
          OMNIORB_CONFIG=/wib/wob/omniORB.cfg; export OMNIORB_CONFIG


Building the examples
=====================

You are strongly encouraged to try out the examples provided in the
src/examples directory. To build them, go into the src/examples
directory (within the build tree if you are using an Autoconf separate
build tree) and do "make".

Study the documentation in ./doc before you run any of the example
programs.


Writing your own Makefile
=========================

The distribution makefiles may be a bit much to digest.
Here is a few tips of what to put into your makefiles to compile omniORB
programs:

1. Compiler flags:

To compile omniORB programs correctly, several C++ preprocessor defines
must be specified to identify the target platform. With an Autoconf
based build, the file include/omniconfig.h sets the defines for you,
so you do not need to explicitly set anything. With non-Autoconf
builds, you must set the following processor defines:

Sun Solaris 2.5     |__sparc__  __sunos__     __OSVERSION__=5            |
Digital Unix 3.2    |__alpha__  __osf1__      __OSVERSION__=3            |
HPUX 10.x           |__hppa__   __hpux__      __OSVERSION__=10           |
HPUX 11.x           |__hppa__   __hpux__      __OSVERSION__=11           |
IBM AIX 4.x &       |__aix__    __powerpc__   __OSVERSION__=4            |
Linux 2.0 (x86)     |__x86__    __linux__     __OSVERSION__=2            |
Linux 2.0 (alpha)   |__alpha__  __linux__     __OSVERSION__=2            |
Windows/NT 3.5      |__x86__    __NT__        __OSVERSION__=3  __WIN32__ |
Windows/NT 4.0      |__x86__    __NT__        __OSVERSION__=4  __WIN32__ |
Windows 2000        |__x86__    __NT__        __OSVERSION__=5  __WIN32__ |
Windows 95          |__x86__    __WIN32__                                |
OpenVMS 6.x (alpha) |__alpha__  __vms         __OSVERSION__=6            |
OpenVMS 6.x (vax)   |__vax__    __vms         __OSVERSION__=6            |
ATMos 4.0           |__arm__    __atmos__     __OSVERSION__=4            |
NextStep 3.x        |__m68k__   __nextstep__  __OSVERSION__=3            |
Unixware 7          |__x86__    __uw7__       __OSVERSION__=5            |

You should also specify the preprocessor defines (e.g. -D_REENTRANT) for
compiling multithreaded programs.


2. Libraries:

The runtime libraries that you have to link to your executables are
usually:

libomnithread.so    - omnithread shared library
libomniORB4.so      - omniORB runtime shared library
libomniDynamic4.so  - omniORB runtime shared library for dynamic features
libomniCodeSets4.so - extra code sets for string transformation
libomnisslTP4.so    - SSL transport (built if OpenSSL is available)
libCOS4.so          - stubs and skeletons for the COS service interfaces
libCOSDynamic4.so   - dynamic stubs for the COS service interfaces

The name of the libraries may be have different suffixes on different
platforms. You can figure it out.

3. IDL compiler:

IDL stubs can be compiled like this:

  omniidl -bcxx echo.idl

The product is the files: echo.hh and echoSK.cc


Documentation
=============

You should read the omniORB and the naming service user guides. Follow
the instructions in the guides to complete the configuration process.
