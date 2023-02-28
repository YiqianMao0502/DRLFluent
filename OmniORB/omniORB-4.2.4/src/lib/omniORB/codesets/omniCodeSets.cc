// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniCodeSets.cc            Created on: 26/07/2001
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library. If not, see http://www.gnu.org/licenses/
//
//
// Description:
//    Nastiness to force static initialisers to run

#include <omniORB4/linkHacks.h>

OMNI_FORCE_LINK(CS_8859_2);
OMNI_FORCE_LINK(CS_8859_3);
OMNI_FORCE_LINK(CS_8859_4);
OMNI_FORCE_LINK(CS_8859_5);
OMNI_FORCE_LINK(CS_8859_6);
OMNI_FORCE_LINK(CS_8859_7);
OMNI_FORCE_LINK(CS_8859_8);
OMNI_FORCE_LINK(CS_8859_9);
OMNI_FORCE_LINK(CS_8859_10);
OMNI_FORCE_LINK(CS_8859_11);
OMNI_FORCE_LINK(CS_8859_13);
OMNI_FORCE_LINK(CS_8859_14);
OMNI_FORCE_LINK(CS_8859_15);
OMNI_FORCE_LINK(CS_8859_16);
OMNI_FORCE_LINK(CS_CP1250);
OMNI_FORCE_LINK(CS_CP1251);
OMNI_FORCE_LINK(CS_CP1252);
OMNI_FORCE_LINK(CS_CP1253);
OMNI_FORCE_LINK(CS_CP1254);
OMNI_FORCE_LINK(CS_CP1255);
OMNI_FORCE_LINK(CS_CP1256);
OMNI_FORCE_LINK(CS_CP1257);
OMNI_FORCE_LINK(CS_CP1258);
OMNI_FORCE_LINK(CS_037);
OMNI_FORCE_LINK(CS_500);
OMNI_FORCE_LINK(CS_EDF_4);
OMNI_FORCE_LINK(CS_UCS_4);
OMNI_FORCE_LINK(CS_GBK);

OMNI_EXPORT_LINK_FORCE_SYMBOL(omniCodeSets);


#if defined(__DMC__) && defined(_WINDLL)
#  include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  return TRUE;
}
#endif
