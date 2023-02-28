omniORB on Win32 / Win64 platforms
==================================

This file contains information on installing, building, and using
omniORB on Win32 and Win64 platforms.

omniORB has been tested with the following software configuration:

- Operating System  : Any modern Windows version, back to NT 4.
- Architecture      : x86, x86-64 (and alpha in earlier versions)
- Compiler          : Visual C++ 6 and later


Roadmap
=======

When the omniORB distribution is unpacked, the following are created:

<Top-Level Directory>\                     : Directory where distribution was 
                                             unpacked

<Top-Level Directory>\doc\                 : omniORB Documentation

<Top-Level Directory>\include\             : Include files
<Top-Level Directory>\include\omniORB4\    : Include files for ORB run-time 
                                             library
<Top-Level Directory>\include\omnithread.h : Main omnithread include file
<Top-Level Directory>\include\omnithread\  : Include files for thread library

<Top-Level Directory>\src\                 : Source files
<Top-Level Directory>\src\lib\omniORB\     : Source files for ORB run-time 
                                             library
<Top-Level Directory>\src\lib\omnithread\  : Source files for thread library
<Top-Level Directory>\src\tool\omniidl\    : Source files for IDL Compiler
<Top-Level Directory>\src\appl\omniNames\  : Source files for COS Naming 
                                             Service
<Top-Level Directory>\src\appl\utils\      : Source files for utilities
<Top-Level Directory>\src\examples\        : Source for example programs


Installation
============

Note that although there are many references to "win32", everything
works on 64 bit Windows platforms. There is no separate "win64"
platform in the build environment.

The executables and DLLs are in <Top-Level Directory>\bin\x86_win32.
The libraries are in            <Top-Level Directory>\lib\x86_win32.

You should set up your PATH environment to include

   <Top-Level Directory>\bin\x86_win32

otherwise the DLLs will not be picked up when omniORB programs are run.

omniORB is primarily shipped in source form. To use it, you must first
compile it. There are a few pre-requisites, but as long as you follow
these instructions, compilation should be easy.


Building omniORB from the source files
======================================

omniORB should be compiled using Visual C++. It compiles fine with the
latest Visual Studio Community versions that are free for
non-commercial use. You can get it from https://www.visualstudio.com/


 A. Pre-requisites
    --------------

    The omniORB build system requires the Cygwin utilities and
    Python. Cygwin is only used at build time; Python is used by the
    IDL compiler, omniidl. Neither Cygwin or Python are used at
    runtime by compiled omniORB code.


    Cygwin
    ------

    The full Cygwin distribution is freely available at:

       http://www.cygwin.com/

    The installer allows you to choose from a vast number of packages.
    To build omniORB, you just need the standard tools from the base
    package, plus GNU make from the devel package.


    Python
    ------

    omniidl requires Python 2.7 or 3.x. You can download the full
    Python distribution from:

       http://www.python.org/download/download_windows.html

    Make sure you install a 64 bit or 32 bit Python version that
    matches the architecture you want to use for omniORB. The Cygwin
    version of Python does not work. You must use the version from
    python.org.



  B. Choose the right platform configuration file
     --------------------------------------------

     Edit <top>\config\config.mk to select the right platform file
     according to the compiler you are using:

       platform = x86_win32_vs_X

     Where X is the version of Visual Studio you are using. Microsoft
     confusingly gives both a year-version and a numeric version to
     their releases of Visual Studio. Even more confusingly, the
     cl.exe compiler has its own different version number!  Visual
     Studio 2017 is version 15, so for that you use x86_win32_vs_15.
     For a helpful table of version numbers, see

       https://en.wikipedia.org/wiki/Microsoft_Visual_Studio#History


     Instead of using Visual C++, you may have some success with MinGW
     instead, in which case you can use 

       platform = x86_win32_mingw

     The MinGW is not maintained and tested as part of the core
     omniORB development, so you may need to make some modifications
     to the platform file for it to work.


  C. Set the location of the Python interpreter
     ------------------------------------------

     Edit <top>\mk\platforms\<platform>.mk

     where <platform> is the platform you just chose in config.mk,
     e.g. <top>\mk\platforms\x86_win32_vs_15.mk.

     Set PYTHON to the location of your Python executable. Note that
     you must use a Unix-style Cygwin path,
     e.g. /cygdrive/c/Python36/python


  D. Building and installing
     --------------------------
     
     Visual C++ needs lots of environment variables to be set for it
     to find all the components it needs. By far the easiest way to
     get the right settings is to use the "Command Prompt" shortcuts
     created in the Start menu by the Visual Studio Installer. For
     example, to use the 64 bit compiler from Visual Studio 2017, run

     Start -> All Programs -> Visual Studio 2017 ->
        VS2015 x64 Native Tools Command Prompt

     In the terminal window that opens, you now need to add the Cygwin
     binaries and Python directory to the END of the path:

       set path=%path%;c:\cygwin64\bin;c:\Python36

     (Putting them at the end is essential -- cygwin includes a
      link.exe which clashes with Visual Studio's link.exe.)


     Now, to compile, go into the directory <top>\src and type 'make
     export'. If all goes well:

        1. The executables and DLLs will be installed into
               <top>\bin\x86_win32\

        2. The libraries will be installed into
               <top>\lib\x86_win32\


     If you are using mingw to build omniORB, before you build the
     main distribution, you must build the omkdepend tool first by
     going to the directory <top>\src\tool\omkdepend and typing 'make
     export'.



Libraries
=========

To link against the DLL versions of the omniORB libraries, you should
use the following libraries:

  omnithread_rt.lib     -- omnithread library
  omniORB4_rt.lib       -- omniORB runtime library
  omniDynamic4_rt.lib   -- omniORB runtime library for dynamic features
  omniCodeSets4_rt.lib  -- extra code sets for string transformation
  omnisslTP4_rt.lib     -- SSL transport (if OpenSSL is available)
  COS4_rt.lib           -- stubs and skeletons for the COS service interfaces
  COSDynamic4_rt.lib    -- dynamic stubs for the COS service interfaces

If you are building your application code in debug mode, you MUST use
the debug versions of the omniORB libraries, otherwise you will get
assertion errors from the Visual C++ runtime about invalid heap
pointers. The debug DLL versions can be used by linking with the
_rtd.lib libraries instead of the _rt.lib libraries.


To link with static libraries, pick the libraries without _rt in their
names: omnithread.lib, omniORB4.lib, omniDynamic4.lib,
omniCodeSets4.lib, omnisslTP.lib, COS4.lib and COSDynamic4.lib.
Again, if you are compiling your application code in debug mode, you
MUST use the debug versions of the omniORB libraries, omnithreadd.lib,
omniORB4d.lib, etc.



Configuring the naming service
==============================
 
To use the naming service, you must configure omniORB to tell it where
the naming service is running.

Consult the user guides in ./doc for full details. For a quick start,
follow these steps:

    o Make sure that <Top-Level Directory>\bin\x86_win32 is in your
      PATH environment.

    o Set the environment variable OMNINAMES_LOGDIR to a directory where
      the naming service omniNames can store its data. For example:
          set OMNINAMES_LOGDIR=C:\OMNINAMES

    o Start omniNames. The binary is in <Install Location>\bin\x86_win32. 
      For example:
         omniNames -start

    o Start the registry editor tool (regedit or regedt32)

    o Select the key HKEY_LOCAL_MACHINE\SOFTWARE\omniORB\InitRef (or
      create it if it doesn't exist). Add a string value (known as a
      REG_SZ data type when using REGEDT32) with name "1", value
      "NameService=corbaname::my.host.name" (putting the correct host
      name in, of course).

    o Due to a bug in some versions of Windows, omniORB may crash if
      you have no entries in the SOFTWARE\omniORB key other than the
      InitRef sub-key. If this happens, set the name InitRef in the
      SOFTWARE\omniORB key, rather than using a sub-key, or add at
      least one configuration parameter to the SOFTWARE\omniORB key.

    o The registry entries must be duplicated on all machines that
      will run omniORB programs. It is only necessary to run one
      instance of omniNames on your network.

    o To save manually editing registry entries, you can modify the
      sample.reg file to contain the configuration you require, then
      import the whole file into the registry.

    o Once you are satisfied the naming service is running properly.
      You can choose to setup omniNames to run as an NT service. See
      the description below.

omniNames can be run as a Windows service. See doc/omniNames.pdf for
details.



Compiling the examples with nmake
=================================

Once the installation is completed. You can try compiling and running
the examples in <Top-Level Directory>\src\examples. You can use GNU
make as with the core omniORB build, or you can use Microsoft's nmake.

Just do the following:

  cd <Top-Level Directory>\src\examples
  nmake /f dir.mak

Have a look at the dir.mak file in <Top-Level Directory>\src\examples,
it should give you some idea about the compiler flags and libraries to
compile and link omniORB programs.



Building Projects using omniORB and Visual C++ 7 and later
==========================================================

Before building anything (or running the examples), you should refer
to the omniORB documentation. In particular, you must configure the
naming service as described above.

To configure your project to us the omniORB DLLs, follow these steps:


a) Add the stub (SK.cpp) files generated by the IDL compiler to the project
   Project ->"Add existing Item")

b) Set up the search paths for include and library files: 

   1. Open Project Properties Window
   2. Under C++ -> General(Additonal Include Directories), Enter The 
      directory where you installed the omniORB include files 
      (this is <Top-Level Directory>\include).

   3. Under Linker -> General (Additional Library Directories), Enter the
      directory where you installed the omniORB library files (this is 
      <Top-Level>\lib\win-32)  

c) Set up macros and libraries:

    1. Under Project Properties ->Settings, Select the "C/C++" tab.
    2. Select The Code Generaion Tab:
    3. Set "Enable exception Handling" To Yes
    4. Set Run-Time Library To "Mulithreaded DLL". 
    5. **This is an important step.**  
       Select The Code Generaion Tab: (Under C++ in project properties)
 
       In the Preprocessor Definitions Box Add The Macros:

       __WIN32__,__x86__,_WIN32_WINNT=0x0400

       If this is NT 4.0, Windows 2000 or XP (or something later), add
       the macros __NT__ and __OSVERSION__=4 as well.

    6. Expand The Linker Tab.
    7. Select The Input Tab
    8. add The following Libraries to "Additional Dependencies"

       ws2_32.lib, mswsock.lib, advapi32.lib, omniORB4_rt.lib,
       omniDynamic4_rt.lib, omnithread_rt.lib

       If you are building a debug executable, the debug version of
       the libraries: omniORB4_rtd.lib, omniDynamic4_rtd.lib and
       omnithread_rtd.lib should be used.

  d)   Your project is now set up, and you can build it.     



Known Problems
==============

When compiling the stub files generated by omniidl, you may come
across some bugs in Microsoft Visual C++. The bugs are to do with the
handling of nested classes. You may come across these bugs if you use
modules in your IDL files, and in certain other circumstances.

This release generates stub code that works around most of the MS VC++
bugs. However, it is not possible to avoid the bugs in all cases. In
particular, the following sample IDL will fail to compile:

// IDL

module A {
  struct B {
    long x;
  };
  enum C { C_1, C_2 };
  module D {
     struct B {
       float y;
     };
     // The stub for struct E would fail to compile
     struct E {
       A::B e1;
       B    e2;
     };
  };
};



Mailing List
============

There is a mailing list for discussing the use and development of
omniORB. See README.FIRST.txt for details on subscribing.
