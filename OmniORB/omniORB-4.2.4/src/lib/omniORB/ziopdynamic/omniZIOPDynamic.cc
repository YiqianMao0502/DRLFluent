// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniZIOPDynamic.cc         Created on: 2013/07/12
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2013 Apasphere Ltd.
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
// Description:
//    ZIOP support that requires Any

#include <omniORB4/omniZIOP.h>
#include <omniORB4/omniInterceptors.h>
#include <initialiser.h>


OMNI_NAMESPACE_BEGIN(omni)


static CORBA::Boolean
createPolicyInterceptor(omniInterceptors::createPolicy_T::info_T& iinfo)
{
  switch (iinfo.type) {
    case 64: // ZIOP::COMPRESSION_ENABLING_POLICY_ID:
      {
        CORBA::Boolean enabled;
        iinfo.value >>= CORBA::Any::to_boolean(enabled);
        iinfo.policy = omniZIOP::create_compression_enabling_policy(enabled);
        return 0;
      }

    case 65: // ZIOP::COMPRESSOR_ID_LEVEL_LIST_POLICY_ID:
      {
        Compression::CompressorIdLevelList* cids;
        iinfo.value >>= cids;
        iinfo.policy = omniZIOP::create_compression_id_level_list_policy(*cids);
        return 0;
      }

    case 66: // ZIOP::COMPRESSION_LOW_VALUE_POLICY_ID:
      {
        CORBA::ULong low_value;
        iinfo.value >>= low_value;
        iinfo.policy = omniZIOP::create_compression_low_value_policy(low_value);
        return 0;
      }

    case 67: // ZIOP::COMPRESSION_MIN_RATIO_POLICY_ID:
      {
        Compression::CompressionRatio ratio;
        iinfo.value >>= ratio;
        iinfo.policy = omniZIOP::create_compression_min_ratio_policy(ratio);
        return 0;
      }
  }
  return 1;
}

//
// Module initialiser

class omniZIOPDynamic_initialiser : public omniInitialiser {
public:
  inline omniZIOPDynamic_initialiser() {
    omniInitialiser::install(this);
  }
  void attach() {
    omniInterceptors* interceptors = omniORB::getInterceptors();

    interceptors->createPolicy.add(createPolicyInterceptor);

    omniORB::logs(2, "omniZIOPDynamic activated.");
  }

  void detach() {
    omniInterceptors* interceptors = omniORB::getInterceptors();

    interceptors->createPolicy.remove(createPolicyInterceptor);
    omniORB::logs(2, "omniZIOPDynamic deactivated.");
  }
};

static omniZIOPDynamic_initialiser the_omniZIOPDynamic_initialiser;

OMNI_NAMESPACE_END(omni)
