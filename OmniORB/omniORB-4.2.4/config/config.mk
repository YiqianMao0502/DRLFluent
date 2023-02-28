# This file selects the platform you are building for.

# If you have a Unix-like platform, you are probably better off using
# the configure script than using this configuration mechanism.

# Uncomment one of the following platform lines to build for the
# target platform
#
#  x86_win32_vs_16           x86 Windows, MS VC++ 16.x (2019)
#  x86_win32_vs_15           x86 Windows, MS VC++ 15.x (2017)
#  x86_win32_vs_14           x86 Windows, MS VC++ 14.x (2015)
#  x86_win32_vs_12           x86 Windows, MS VC++ 12.x (2013)
#  x86_win32_vs_11           x86 Windows, MS VC++ 11.x (2012)
#  x86_win32_vs_10           x86 Windows, MS VC++ 10.x (2010)
#  x86_win32_vs_9            x86 Windows, MS VC++ 9.x (2008)
#  x86_win32_vs_8            x86 Windows, MS VC++ 8.x (.NET)
#  x86_win32_vs_7            x86 Windows, MS VC++ 7.x (.NET)
#  x86_win32_vs_6            x86 Windows, MS VC++ 6.0 or 5.0
#  x86_win32_mingw           x86 Windows, mingw/g++ build
#  x86_win32_dmc             x86 Win32, Digital Mars C++ (>= 8.32.14)
#  x86_ets                   Phar Lap Realtime ETS-kernel
#  x86_LynxOS_4.0            x86, LynxOS 4.0, gcc 2.95.3
#  pc486_rtems_4.5.0         x86, RTEMS, gcc 2.95.2


# You should also look at <top>/mk/platforms/$(platform).mk and if necessary
# edit the make variables, such as CC and CXX, in the file.

#platform = x86_win32_vs_16
#platform = x86_win32_vs_15
#platform = x86_win32_vs_14
#platform = x86_win32_vs_12
#platform = x86_win32_vs_11
#platform = x86_win32_vs_10
#platform = x86_win32_vs_9
#platform = x86_win32_vs_8
#platform = x86_win32_vs_7
#platform = x86_win32_vs_6
#platform = x86_win32_mingw
#platform = x86_win32_dmc
#platform = powerpc_LynxOS_4.0
#platform = x86_ets
#platform = x86_LynxOS_4.0
#platform = pc486_rtems_4.5.0
#platform = arm_linux_mvl_3.1


# These platforms are mostly of historical interest. The configure
# script is usually the best starting point.
#
#  sun4_sosV_5.5             Solaris 2.5, Sunpro C++/gcc
#  sun4_sosV_5.6             Solaris 2.6, Sunpro C++/gcc
#  sun4_sosV_5.7             Solaris 7, Sunpro C++/gcc
#  i586_linux_2.0            x86 Redhat linux 4.2, gcc-2.7.2
#  i586_linux_2.0_glibc      x86 Redhat linux 5.1,egcs-980302 or later snapshot
#  i586_linux_2.0_glibc2.1   x86 Redhat linux 6, etc
#  alpha_nt_4.0              Alpha Windows NT 4.0, MS VC++ 5.0
#  alpha_osf1_3.2            Digital Unix 3.2, DEC C++ v5.5
#  alpha_osf1_4.0            Digital Unix 4.0, DEC C++ v6.0
#  alpha_osf1_5.0            Tru64 5.0, DEC C++ v6.2
#  alpha_linux_2.0           alpha linux (compiler problem,still doesn't work)
#  powerpc_aix_4.2           IBM AIX 4.2, IBM C Set++
#  powerpc_aix_4.2_xlc5      IBM AIX 4.2, XLC 5.0 (a.k.a. Visual Age 5.0)
#  powerpc_aix_4.3_xlc5      IBM AIX 4.3, XLC 5.0 (a.k.a. Visual Age 5.0)
#  powerpc_linux_2.2_glibc   PowerPC linux, Debian
#  powerpc_darwin_1.3        PowerPC Darwin/Mac OS X
#  hppa_hpux_10.20           HPUX 10.20, aC++ B3910B A.01.04
#  hppa_hpux_11.00           HPUX 11.00, aC++ B3910B A.01.04
#  m68k_nextstep_3.3         NextStep 3.3, gcc-2.7.2
#  x86_nextstep_3.3          NextStep 3.3, x86
#  mips_sinux_5.43           Reliant UNIX from Siemens-Nixdorf (SNI)
#  mips_irix_6.2_n32         SGI Irix 6.2 - 32bit mode
#  mips_irix_6.3_n32         SGI Irix 6.3 - 32bit mode
#  mips_irix_6.4_n32         SGI Irix 6.4 - 32bit mode
#  mips_irix_6.4_64          SGI Irix 6.4 - 64bit mode - not yet tested
#  mips_irix_6.5_n32         SGI Irix 6.5 - 32bit mode
#  mips_irix_6.5_64          SGI Irix 6.5 - 64bit mode
#  x86_osr5                  SCO OpenServer 5
#  x86_uw7                   SCO UnixWare 7 (7.1.0)
#  x86_freebsd_3.2           x86 FreeBSD 3.2, egcs 1.1.2
#  x86_freebsd_4.0           x86 FreeBSD 4.0, gcc 2.95

#platform = sun4_sosV_5.5
#platform = sun4_sosV_5.6
#platform = sun4_sosV_5.7
#platform = i586_linux_2.0
#platform = i586_linux_2.0_glibc
#platform = i586_linux_2.0_glibc2.1
#platform = alpha_nt_4.0
#platform = alpha_osf1_3.2
#platform = alpha_osf1_4.0
#platform = alpha_osf1_5.0
#platform = powerpc_aix_4.2
#platform = powerpc_aix_4.3
#platform = powerpc_aix_4.2_xlc5
#platform = powerpc_aix_4.3_xlc5
#platform = powerpc_linux_2.2_glibc
#platform = powerpc_darwin_1.3
#platform = hppa_hpux_10.20
#platform = hppa_hpux_11.00
#platform = m68k_nextstep_3.3
#platform = mips_sinux_5.43
#platform = mips_irix_6.2_n32
#platform = mips_irix_6.3_n32
#platform = mips_irix_6.4_n32
#platform = mips_irix_6.4_64
#platform = mips_irix_6.5_n32
#platform = mips_irix_6.5_64
#platform = x86_nextstep_3.3
#platform = x86_osr5
#platform = x86_uw7
#platform = x86_freebsd_3.2
#platform = x86_freebsd_4.0

# This setting is used on Windows platforms to build debug versions of
# the omniORB executables. It is not required if you want to debug
# your own code, or for building debug versions of the omniORB
# libraries. Setting this variable causes the build to break in subtle
# ways. DO NOT SET THIS UNLESS YOU KNOW WHAT YOU ARE DOING.
#
#BuildDebugBinary = 1
#

EXPORT_TREE =  $(TOP)

IMPORT_TREES = $(TOP)

override VPATH := .

THIS_IMPORT_TREE := $(TOP)
ifneq ($(wildcard $(THIS_IMPORT_TREE)/mk/beforedir.mk),)
include $(THIS_IMPORT_TREE)/mk/beforedir.mk
endif

include dir.mk

THIS_IMPORT_TREE := $(TOP)
ifneq ($(wildcard $(THIS_IMPORT_TREE)/mk/afterdir.mk),)
include $(THIS_IMPORT_TREE)/mk/afterdir.mk
endif

