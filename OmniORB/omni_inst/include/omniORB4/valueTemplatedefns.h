// -*- Mode: C++; -*-
//                            Package   : omniORB
// valueTemplatedefns.h       Created on: 2005/01/06
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2005-2009 Apasphere Ltd.
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
//    ValueType templates
//

#ifndef __VALUETEMPLATEDEFNS_H__
#define __VALUETEMPLATEDEFNS_H__


template <class T, class ElemT, class T_Helper>
inline void
_CORBA_Sequence_Value<T,ElemT,T_Helper>::operator>>= (cdrStream& s) const
{
  ::operator>>=(_CORBA_ULong(pd_len), s);
  for (int i = 0; i < (int)pd_len; i++)
    T_Helper::marshal(pd_buf[i], s);
}

template <class T, class ElemT, class T_Helper>
inline void
_CORBA_Sequence_Value<T,ElemT,T_Helper>::operator<<= (cdrStream &s) {
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || (pd_bounded && l > pd_max)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }

  _CORBA_ULong i;

  if (pd_rel) {
    for (i=0; i < pd_len; i++) {
      T_Helper::remove_ref(pd_buf[i]);
      pd_buf[i] = 0;
    }
  }
  pd_len = 0;

  length(l);
  for (i = 0; i < l; i++)
    pd_buf[i] = T_Helper::unmarshal(s);
}


#endif // __VALUETEMPLATEDEFNS_H__
